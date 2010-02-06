/*!
************************************************************************
* \file
*    
* \brief
*    <<?? 
* \date
*    2008_09_09
* \author
*    - Qi LIU                         <>
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
************************************************************************
*/

#include "stdafx.h"
#include "CParabolaDetect.h"

#include "Macro.h"
#include "ParabolaStruct.h"

extern int g_debug;

ParamStruct g_ps;
ParamDistinguish g_pd;

CParabolaDetect::CParabolaDetect(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in, unsigned int const  nChannel)
{
  memset(TrackObject, 0, m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
  memset(ObjLabelInfo, 0, m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(PreLabelInfo, 0, m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(AreaInfo, 0, m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(&AreaInfoMax, 0, sizeof(ObjLabelInfoStruct));

  m_frame_width = nYWidth_in ;
  m_frame_height = nYHeight_in ;

  ParamSet.white_spot_num_min   = 2        ;
  ParamSet.track_max_frame_num = 20        ;
  ParamSet.track_x_continue_threshold = 0  ;
  ParamSet.track_x_offset_value = 1        ;
  ParamSet.b_little_region_flag = false    ;

  m_alarm_flg       = FALSE ;       //报警标志     
  m_fir_frm_num     = 0     ;       //记录最开始的几帧，用于背景叠加
  m_tracked_obj_flag = FALSE ;       //找到预测目标标志
  m_curr_frm_num    = 0     ;       //得到的当前帧数
  m_pre_frm_num     = 0     ;       //前一帧的目标数
  m_track_num        = 0     ;       //
  m_frm_num         = 0     ;       //
  m_frm_difference  = 0     ;
  m_area_frm_num    = 0     ;
  m_alarm_delay     = 0     ;
  m_smooth_rgb_number = 0   ;

  m_circle_frm_num  = 0     ;
  m_circle_frm_first= true  ;
  m_person_time_delay = 0 ;
  m_person_change_bk_time = 0;      //5分钟
  m_person_change_bk_flag = false;
  m_person_detect_flag = false ;
  m_person_frm_num = 0  ;

  uint16_t i ;
  for ( i = 0; i < 5; i++)
  {
    m_continue_image[i] = new uint8_t[nYWidth_in*nYHeight_in*3];  
    ASSERT( m_continue_image[i] );
  }
  for ( i = 0; i < 5; i++)
  {
    m_show_image[i] = new uint8_t[nYWidth_in*nYHeight_in]; 
    ASSERT( m_show_image[i] );
  }

  for ( i = 0; i < 2; i++)
  { 
    m_para_detect_image[i] = new uint8_t[nYWidth_in*nYHeight_in*3]; 
    ASSERT( m_para_detect_image[i] );
  }

   m_black_block_image = new uint8_t[nYWidth_in*nYHeight_in];
   ASSERT( m_black_block_image );
   memset(m_black_block_image,1,nYWidth_in*nYHeight_in);

   if (0 != m_frame_width )
   {
     InitParaRead(nChannel);
   }

//    // jiangqi //////////////
//    m_pFrameContainer = new CFrameContainer(m_frame_width, m_frame_height);
//    m_pOutFrameContainer = new CFrameContainer(m_frame_width, m_frame_height);
//    m_pFrame_matlabFunced = new CFrameContainer(m_frame_width, m_frame_height);
//    // jiangqi //////////////

}
CParabolaDetect::~CParabolaDetect()
{
  int i = 0 ;
  for ( i = 0; i < 5; i++)
    SAFEDELETE(m_continue_image[i]);
  for ( i = 0; i < 5; i++)
    SAFEDELETE(m_show_image[i]);
  for ( i = 0; i < 2; i++)
    SAFEDELETE(m_para_detect_image[i]);

  SAFEDELETE(m_black_block_image);

//   // jiangqi //////////////
//   delete m_pFrameContainer;
//   delete m_pOutFrameContainer;
//   delete m_pFrame_matlabFunced;
// 
//   m_pFrameContainer = NULL;
//   m_pOutFrameContainer = NULL;
//   m_pFrame_matlabFunced = NULL;
//   // jiangqi //////////////

}


//////////////////////////////////////////////////////////////////////////
/// function:erode the image   
/// input:pFrame_binaried--0,1 image,
///       pRect-- if erode the whole frame,pRect = NULL,if erode part of the frame,pRect point to the rectangle structure
///       elementstr--point to the width and height of the struct element 
/// output:OK if finished erode
/// 腐蚀函数，输入二值图像，如果整帧腐蚀，pRECT = null，否则给定矩形框结构，只对矩形框内区域腐蚀
/// elementstr 是腐蚀结构元素，给定腐蚀结构元素的长宽，例如3*3腐蚀，则elementst[0]= 3，elementstr[1] = 3;
//////////////////////////////////////////////////////////////////////////
ErrVal
CParabolaDetect::Imerode(CFrameContainer* const pFrame_binaried, uint16_t* pRect,uint16_t* elementstr) const
{
  uint16_t centerX,centerY,strleny,strlenx;
  if(elementstr == NULL)
  {
    centerX = 1;
    centerY = 1;
    strleny = 3;
    strlenx = 3;
  }
  else
  {
    centerX = (uint16_t)(((elementstr[0]+1)/2)-1);
    centerY = (uint16_t)(((elementstr[1]+1)/2)-1);
    strleny = elementstr[0];
    strlenx = elementstr[1];
  }
  uint16_t framewidth = pFrame_binaried->getWidth();
  uint16_t frameheight = pFrame_binaried->getHeight();

  if(pRect == NULL)
  {
    int m,n,i=1,j=0;
    bool flag = FALSE;
    uint8_t* pData = new uint8_t[frameheight*framewidth];
    ASSERT(pData);
    memcpy(pData,pFrame_binaried->m_YuvPlane[0],pFrame_binaried->getYSize());

    for(i = centerY;i < (frameheight - strleny + centerY);i++)
    {
      for(j = centerX;j < (framewidth - strlenx + centerX);j++)
      {
        if (pData[i*framewidth+j] == 1)
        {
          for(m = -centerY; m < (strleny-centerY);m++)
          {
            for(n = -centerX; n < (strlenx-centerX);n++)
            {
              if(pData[(i+m)*framewidth+(j+n)] == 0)
              {
                pFrame_binaried->m_YuvPlane[0][i*framewidth+j] = 0;
                flag = TRUE;
                break;
              }
            }
            if(flag)
            {
              flag = TRUE;
              break;
            }
          }
        }
      }
    }
    if(pData)
      delete [] pData;

    ROK();
  }
  else
  {
    int m,n,i=0,j=0;
    bool flag = FALSE;

    uint8_t* pData = new uint8_t[frameheight*framewidth];
    ASSERT(pData);
    memcpy(pData,pFrame_binaried->m_YuvPlane[0],pFrame_binaried->getYSize());

    for(i = pRect[1] + centerY;i < (pRect[1] + pRect[3] -1 - strleny + centerY);i++)
    {
      for(j = pRect[0] + centerX;j < (pRect[0] + pRect[2] - strlenx + centerX);j++)
      {
        if (pData[i*framewidth+j] == 1)
        {
          for(m = -centerY;m < (strleny-centerY);m++)
          {
            for(n = -centerX;n < (strlenx-centerX);n++)
            {
              if(pData[(i+m)*framewidth+(j+n)]==0)
              {
                pFrame_binaried->m_YuvPlane[0][i*framewidth+j] = 0;
                flag = TRUE;
                break;
              }
            }
            if(flag)
            {
              flag = TRUE;
              break;
            }
          }
        }
      }
    }
    if(pData)
      delete [] pData;
    ROK();
  }
}

///////////////////////////////////////////////////////////////////////
/// function: dilate the image 
/// input:0-1 image,dilate retangle area
/// 
/// output:    OK for success
/// 膨胀函数，pRectangle 给定膨胀区域矩形框，nDilateTimesPerPixel_in 为膨胀次数
///////////////////////////////////////////////////////////////////////
void CParabolaDetect::Imdilate( CFrameContainer* const pFrame_in,uint32_t nDilateTimesPerPixel_in,  uint16_t* pRectangle) 
{ 
  ASSERT( pFrame_in );

  int m = 0 ,
      n = 0 ,
      i = 0 ,
      j = 0 ;
  bool     v_flag   = false ;
  uint32_t k        = 0     ;
  uint16_t v_width  = m_frame_width ;
  uint16_t v_height = m_frame_height;

  uint8_t * p_data = new uint8_t[v_width*v_height];

  uint16_t* p_rect = NULL;
  if(pRectangle == NULL)
  {
    uint16_t Rect[4] = {0,0,v_width,v_height};
    p_rect = Rect;
  }
  else
    p_rect = pRectangle;

  for( k = 0; k < nDilateTimesPerPixel_in; ++k )
  {
    memcpy(p_data,pFrame_in->m_YuvPlane[0],v_width*v_height*sizeof(p_data[0]));
    for( i = p_rect[1]+1; i < p_rect[3]-1; ++i )
    {
      for( j = p_rect[0]+1; j < p_rect[2]-1; ++j )
      {
        if( BLACKSPOT == p_data[i*v_width + j])
        {
          for( m = -1; m < 2; ++m )
          {
            for( n = -1; n < 2; ++n )
            {
              if( WHITESPOT == p_data[(i+m)*v_width+j+n] )
              {
                pFrame_in->m_YuvPlane[0][i*v_width + j]=WHITESPOT;
                v_flag=TRUE;
                break;
              }
            }
            if(v_flag)
            {
              v_flag=FALSE;
              break;
            }
          }
        }
      }
    }  
  }
  if(p_data)
    delete [] p_data;

  return ;
}

ErrVal 
CParabolaDetect::GlobalLabelObject(CFrameContainer* const pFrame_binaried)
{
  ASSERT(pFrame_binaried);

  uint32_t i = 0 ,
           j = 0 ,
           m = 0 ,
           n = 0 ;

  uint16_t x_sign = 0;
  uint16_t m_temp = 0;
  uint16_t x_temp = 0;
  uint16_t y_temp = 0;
  uint32_t xTotal[m_nTrackObjectMaxNum]; //200
  uint32_t yTotal[m_nTrackObjectMaxNum];
  uint32_t objWhiteNum[m_nTrackObjectMaxNum];
  uint32_t framewidth = m_frame_width;
  uint32_t frameheight = m_frame_height;

  uint8_t * p_whitespot = pFrame_binaried->m_YuvPlane[0];
  uint8_t * p_data = new uint8_t[(frameheight+1)*(framewidth+2)];
  uint16_t* p_temp = new uint16_t[(frameheight+1)*(framewidth+2)];
  uint16_t* pLabelDataOut = new uint16_t[frameheight*framewidth];
  memset(objWhiteNum,0,m_nTrackObjectMaxNum*sizeof(uint32_t));
  memset(xTotal,0,m_nTrackObjectMaxNum*sizeof(uint32_t));
  memset(yTotal,0,m_nTrackObjectMaxNum*sizeof(uint32_t));
  memset(p_temp,0,(frameheight+1)*(framewidth+2)*sizeof(uint16_t));
  memset(p_data,0,(frameheight+1)*(framewidth+2)*sizeof(uint8_t));

  for(j = 1;j <= frameheight;j++)
  {
    memcpy(&p_data[j*(framewidth+2)+1],&pFrame_binaried->m_YuvPlane[0][(j-1)*framewidth],framewidth);
  }

  for( j = 1;j <= frameheight;j++)
    for( i = 1;i <= framewidth;i++)
    {
      if(p_data[j*(framewidth+2)+i] == 1)
      {
        if(p_data[(j-1)*(framewidth+2)+i+1] == 1)     //右上
        {
          p_temp[j*(framewidth+2)+i] = p_temp[(j-1)*(framewidth+2)+i+1];
          x_temp = p_temp[(j-1)*(framewidth+2)+i+1];
          objWhiteNum[x_temp] += 1;
          if(p_data[j*(framewidth+2)+i-1] == 1)  //左前
          {
            if(p_temp[j*(framewidth+2)+i-1] != x_temp) 
            {
              y_temp = p_temp[j*(framewidth+2)+i-1];
              if(x_temp < y_temp)
              {
                objWhiteNum[y_temp] = 0;
                for( m = 1;m <= frameheight;m++)
                  for( n = 1;n <= framewidth;n++)
                  {
                    if(p_temp[m*(framewidth+2)+n] == y_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = x_temp;
                      objWhiteNum[x_temp] += 1;
                    }
                    if(p_temp[m*(framewidth+2)+n] > y_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = p_temp[m*(framewidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = y_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
              else 
              {
                objWhiteNum[x_temp] = 0;
                for( m = 1;m <= frameheight;m++)
                  for( n = 1;n <= framewidth;n++)
                  {
                    if(p_temp[m*(framewidth+2)+n] == x_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = y_temp;
                      objWhiteNum[y_temp] += 1;
                    }
                    if(p_temp[m*(framewidth+2)+n] > x_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = p_temp[m*(framewidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = x_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
            }
          }
          else
          {
            if(p_data[(j-1)*(framewidth+2)+i-1] == 1 && p_temp[(j-1)*(framewidth+2)+i-1] != x_temp)   //左上
            {
              y_temp = p_temp[(j-1)*(framewidth+2)+i-1];
              if(x_temp < y_temp)
              {
                objWhiteNum[y_temp] = 0;
                for( m = 1;m <= frameheight;m++)
                  for( n = 1;n <= framewidth;n++)
                  {
                    if(p_temp[m*(framewidth+2)+n] == y_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = x_temp;
                      objWhiteNum[x_temp] += 1;
                    }
                    if(p_temp[m*(framewidth+2)+n] > y_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = p_temp[m*(framewidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = y_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
              else 
              {
                objWhiteNum[x_temp] = 0;
                for( m = 1;m <= frameheight;m++)
                  for( n = 1;n <= framewidth;n++)
                  {
                    if(p_temp[m*(framewidth+2)+n] == x_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = y_temp;
                      objWhiteNum[y_temp] += 1;
                    }
                    if(p_temp[m*(framewidth+2)+n] > x_temp)
                    {
                      p_temp[m*(framewidth+2)+n] = p_temp[m*(framewidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = x_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
            }
          }
        }
        else if(p_data[(j-1)*(framewidth+2)+i] == 1)    //正上
        {
          p_temp[j*(framewidth+2)+i] = p_temp[(j-1)*(framewidth+2)+i];
          x_temp = p_temp[(j-1)*(framewidth+2)+i];
          objWhiteNum[x_temp] += 1;
        }
        else if(p_data[(j-1)*(framewidth+2)+i-1] == 1)   //左上
        {
          p_temp[j*(framewidth+2)+i] = p_temp[(j-1)*(framewidth+2)+i-1];
          x_temp = p_temp[(j-1)*(framewidth+2)+i-1];
          objWhiteNum[x_temp] += 1;
        }
        else if(p_data[j*(framewidth+2)+i-1] == 1)     //左前
        {
          p_temp[j*(framewidth+2)+i] = p_temp[j*(framewidth+2)+i-1];
          x_temp = p_temp[j*(framewidth+2)+i-1];
          objWhiteNum[x_temp] += 1;
        }
        else
        {
          x_sign++;

          if(x_sign >= m_nTrackObjectMaxNum -1)
          {
            if(p_data)
              delete [] p_data;
            if(p_temp)
              delete [] p_temp;
            if(pLabelDataOut)
              delete [] pLabelDataOut;
            //          cout<<"Too many objects!"<<endl;
            RERR();
          }
          m_temp = x_sign;
          p_temp[j*(framewidth+2)+i] = m_temp;
          objWhiteNum[m_temp] = 1;
        }
      }
    }


    m_curr_frm_num = x_sign;

    for(j = 1;j <= frameheight;j++)
      memcpy(&pLabelDataOut[(j-1)*framewidth],&p_temp[j*(framewidth+2)+1],framewidth*sizeof(uint16_t));

    //赋初值
    for(i = 0;i < m_curr_frm_num;i++)
    {
      ObjLabelInfo[i].t_obj_label_rect[0] = framewidth-1;
      ObjLabelInfo[i].t_obj_label_rect[1] = 0;
      ObjLabelInfo[i].t_obj_label_rect[2] = frameheight-1;
      ObjLabelInfo[i].t_obj_label_rect[3] = 0;
    }

    //判断位置坐标，赋值
    for (m = 0; m < frameheight; m++)
      for (n = 0; n < framewidth; n++)
      {
        if( pLabelDataOut[m*framewidth+n] >0 && pLabelDataOut[m*framewidth+n]<m_nTrackObjectMaxNum ) 
        {
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[0] > n)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[0] = n;
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[1] < n)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[1] = n;
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[2] > m)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[2] = m;
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[3] < m)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].t_obj_label_rect[3] = m;
          xTotal[pLabelDataOut[m*framewidth+n]-1]+=n;
          yTotal[pLabelDataOut[m*framewidth+n]-1]+=m;
        }
      }

      uint32_t TempWhiteNum  = 0 ;
      uint16_t TempBeginX = 0 ;
      uint16_t TempBeginY = 0 ;
      uint16_t TempEndX = 0 ;
      uint16_t TempEndY = 0 ;

      for( i = 0;i < m_curr_frm_num; i++)
      {
        uint16_t bottom_x,bottom_y;
        bottom_x = ObjLabelInfo[i].t_obj_label_rect[1];
        bottom_y = ObjLabelInfo[i].t_obj_label_rect[3];
        ObjLabelInfo[i].t_obj_white_spot_num = objWhiteNum[i+1];
        ObjLabelInfo[i].t_obj_label_rect[1] = ObjLabelInfo[i].t_obj_label_rect[2];
        ObjLabelInfo[i].t_obj_label_rect[2] = bottom_x - ObjLabelInfo[i].t_obj_label_rect[0] + 1;
        ObjLabelInfo[i].t_obj_label_rect[3] = bottom_y - ObjLabelInfo[i].t_obj_label_rect[1] + 1;
        ObjLabelInfo[i].t_obj_label_center[0] = xTotal[i]/objWhiteNum[i+1];
        ObjLabelInfo[i].t_obj_label_center[1] = yTotal[i]/objWhiteNum[i+1];

        TempWhiteNum  = 0 ;
        if (ObjLabelInfo[i].t_obj_white_spot_num > (uint32_t)ParamSet.white_spot_num_min 
            && ObjLabelInfo[i].t_obj_white_spot_num < (uint32_t)ParamSet.white_spot_num_max )
        {
          ObjLabelInfo[i].t_flag = false ;
          if(ObjLabelInfo[i].t_obj_white_spot_num < (uint32_t)ParamSet.white_spot_num_max )
          {
            for (m = ObjLabelInfo[i].t_obj_label_rect[1]-3;
                (uint16_t)m < ObjLabelInfo[i].t_obj_label_rect[1] + ObjLabelInfo[i].t_obj_label_rect[3] +4 ;m++)
            {
              for (n = ObjLabelInfo[i].t_obj_label_rect[0]-3; 
                  (uint16_t)n < ObjLabelInfo[i].t_obj_label_rect[0] + ObjLabelInfo[i].t_obj_label_rect[2] +4 ;n++)
              {
                if (1 == p_whitespot[framewidth*m + n ])
                {
                  TempWhiteNum++;
                }
              }
            }

            if (TempWhiteNum < ObjLabelInfo[i].t_obj_white_spot_num + 1 )
            {
              ObjLabelInfo[i].t_flag = true ;
            }

			      if(ObjLabelInfo[i].t_obj_white_spot_num > 250 )
			      {
				      ObjLabelInfo[i].t_flag = true ;
			      }

          }
          else
          {
            ObjLabelInfo[i].t_flag = true ;
          }          
        }
        else
        {
          ObjLabelInfo[i].t_flag = false ;
        }

      }   

      if(p_data)
        delete [] p_data;
      if(p_temp)
        delete [] p_temp;
      if(pLabelDataOut)
        delete [] pLabelDataOut;
      ROK();
}

ErrVal 
CParabolaDetect::DetectedTrackedObject(const CFrameContainer* const pFrame_in, CFrameContainer* const pFrame_out, LabelObjStatus* pTrackObjInfo)
{
  ASSERT(pTrackObjInfo);

  uint16_t i = 0 ;
  int32_t temp_white_spot = 0 ;
  int32_t round_white_spot = 0 ;
  int16_t forecast_cent_x = pTrackObjInfo->t_forecast_frm_center[0] ;
  int16_t forecast_cent_y = pTrackObjInfo->t_forecast_frm_center[1] ;
  int32_t fore_white_spot_num = pTrackObjInfo->t_white_spot_num ;
  int16_t curr_track_x = pTrackObjInfo->t_cur_frame_center[0];

  int32_t curr_white_num    = 0 ;
  int16_t curr_x_coordinate = 0 ;                  // 
  int16_t curr_y_coordinate = 0 ;                  // 

  int16_t p_temp_fir_value  = 0 ;
  int16_t p_temp_sec_value  = 0 ;
  int16_t find_obj_num = 0 ;
  int16_t temp_diff = 0 ;
  int16_t threshold = 0 ;
  bool    last_dis_flg  = false ;
  bool    new_dis_flg   = false ;
  uint16_t rest_seek[4] = {0,0,0,0} ;
  uint16_t frame_width  = m_frame_width ;  
  uint16_t frame_height = m_frame_height ;

  uint16_t vi_temp_hight = 0 ;
  bool     find_flag = FALSE;

  int16_t detect_x = 10 + pTrackObjInfo->t_lost_frame_num*2 ;
  int16_t detect_y = 10 + pTrackObjInfo->t_lost_frame_num*2 ;

  if (1 == pTrackObjInfo->t_track_direction[0])
  {
    for (i = 0; i < m_curr_frm_num; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        curr_x_coordinate = ObjLabelInfo[i].t_obj_label_center[0];          
        curr_y_coordinate = ObjLabelInfo[i].t_obj_label_center[1]; 
        curr_white_num = ObjLabelInfo[i].t_obj_white_spot_num ;

        if ( fore_white_spot_num < 30 )
        {

          temp_white_spot = (int32_t)(fore_white_spot_num * 0.8 ) ;
          round_white_spot = (int32_t)(fore_white_spot_num  * 2 ) ;
        }
        else
        {
          temp_white_spot = (int32_t)(fore_white_spot_num * 0.8) ;
          round_white_spot = (int32_t)(fore_white_spot_num * 2 ) ;
        }
        if (fore_white_spot_num <= 15 )
        {
          temp_white_spot = 15 ;
          round_white_spot = 15 ;
        }

		if (pTrackObjInfo->t_migration_diff[1] <= 2)
		{
			vi_temp_hight = 6*pTrackObjInfo->t_migration_diff[1] ;
		}
		else
		{
			vi_temp_hight = 3*pTrackObjInfo->t_migration_diff[1] ;
		}

        if( ( abs(curr_x_coordinate - curr_track_x) >= ParamSet.track_x_offset_value ) 
			    && (abs(curr_y_coordinate - forecast_cent_y) <= vi_temp_hight )
          && ( ( abs(curr_x_coordinate - forecast_cent_x) <= detect_x &&  abs(curr_y_coordinate - forecast_cent_y) <= detect_y  && abs((int16_t)(fore_white_spot_num - curr_white_num)) < temp_white_spot )
          ||(abs(curr_x_coordinate - forecast_cent_x) <= 3 &&  abs(curr_y_coordinate - forecast_cent_y) <= 4  && abs((int16_t)(fore_white_spot_num - curr_white_num)) < round_white_spot )
          )
          )
        {
          if(!find_flag)
          {
            p_temp_fir_value = abs((int16_t)(fore_white_spot_num - curr_white_num)) ;
            find_obj_num = i;
          }
          else 
          {
            p_temp_sec_value = abs((int16_t)(fore_white_spot_num - curr_white_num)) ;
            if(p_temp_fir_value > p_temp_sec_value)
            {
              p_temp_fir_value = p_temp_sec_value;
              find_obj_num = i;
            }
          }
          find_flag = TRUE;   

        } 
      }
    }
  }
  else
  {
    for (i = 0; i < m_curr_frm_num; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        curr_x_coordinate = ObjLabelInfo[i].t_obj_label_center[0];          
        curr_y_coordinate = ObjLabelInfo[i].t_obj_label_center[1]; 
        curr_white_num = ObjLabelInfo[i].t_obj_white_spot_num ;

        if ( fore_white_spot_num < 30 )
        {
          temp_white_spot = (int32_t)(fore_white_spot_num * 0.8) ;
          round_white_spot = (int32_t)(fore_white_spot_num * 2 ) ;
        }
        else
        {
          temp_white_spot = (int32_t)(fore_white_spot_num * 0.8) ;
          round_white_spot = (int32_t)(fore_white_spot_num * 2 )  ;
        }
        if (fore_white_spot_num <= 15 )
        {
          temp_white_spot = 15 ;
          round_white_spot = 15 ;
        }

		    if (pTrackObjInfo->t_migration_diff[1] <= 2)
		    {
			    vi_temp_hight = 6*pTrackObjInfo->t_migration_diff[1] ;
		    }
		    else
		    {
			    vi_temp_hight = 3*pTrackObjInfo->t_migration_diff[1] ;
		    }

        if( ( abs(curr_track_x -curr_x_coordinate) >= ParamSet.track_x_offset_value ) 
		      && (abs(curr_y_coordinate - forecast_cent_y) <= vi_temp_hight )
          && (  ( abs(curr_x_coordinate - forecast_cent_x) <= detect_x &&  abs(curr_y_coordinate - forecast_cent_y) <= detect_y  && abs((int16_t)(fore_white_spot_num - curr_white_num)) < temp_white_spot )
          ||(abs(curr_x_coordinate - forecast_cent_x) <= 3 &&  abs(curr_y_coordinate - forecast_cent_y) <= 4  && abs((int16_t)(fore_white_spot_num - curr_white_num)) < round_white_spot )
          )
          )
        {
          if(!find_flag)
          {
            p_temp_fir_value = abs((int16_t)(fore_white_spot_num - curr_white_num)) ;
            find_obj_num = i;
          }
          else 
          {
            p_temp_sec_value = abs((int16_t)(fore_white_spot_num - curr_white_num)) ;
            if(p_temp_fir_value > p_temp_sec_value)
            {
              p_temp_fir_value = p_temp_sec_value;
              find_obj_num = i;
            }
          }

          find_flag = TRUE;   

        } 
      }
    }
  }

  ChangeObjectValue(pFrame_in, &ObjLabelInfo[find_obj_num], pTrackObjInfo, find_flag);

  return m_nTracked;
}

ErrVal CParabolaDetect::ChangeObjectValue(const CFrameContainer* const pFrame_in, ObjLabelInfoStruct* pLabelObjStatus, LabelObjStatus* pTrackObjInfo, bool FindObjectFlag )
{
  if (FindObjectFlag) //更新预测目标信息
  {
    if( pTrackObjInfo->t_track_frame_num > ParamSet.track_max_frame_num )      //--beyond max object number
    {
      pTrackObjInfo->t_continue_track_flag = false ;
      return m_nTracked;
    }

    pTrackObjInfo->t_find_obj_number++;
    pTrackObjInfo->t_track_frame_num++ ;
    pTrackObjInfo->t_lost_frame_num = 0 ;
    pTrackObjInfo->t_pre_frame_center[0] = pTrackObjInfo->t_cur_frame_center[0];  
    pTrackObjInfo->t_pre_frame_center[1] = pTrackObjInfo->t_cur_frame_center[1];
    pTrackObjInfo->t_cur_frame_center[0] = pLabelObjStatus->t_obj_label_center[0]; 
    pTrackObjInfo->t_cur_frame_center[1] = pLabelObjStatus->t_obj_label_center[1];
    pTrackObjInfo->t_white_spot_num = (uint32_t)((float)(pLabelObjStatus->t_obj_white_spot_num + pTrackObjInfo->t_track_frame_num * pTrackObjInfo->t_white_spot_num ) /  (float)(pTrackObjInfo->t_track_frame_num + 1) +0.5f);  

    pTrackObjInfo->t_offset_x = (float)abs((int16_t)(pTrackObjInfo->t_origin_frame_center[0] - pTrackObjInfo->t_cur_frame_center[0]))/(float)(pTrackObjInfo->t_track_frame_num+1);

    if ( abs((int16_t)pTrackObjInfo->t_migration_diff[0] - abs((int16_t)(pTrackObjInfo->t_pre_frame_center[0] - pTrackObjInfo->t_cur_frame_center[0]))  )<4  )
    {
      pTrackObjInfo->t_match_num[0]++;
    }

    if (  abs((int16_t)pTrackObjInfo->t_migration_diff[1] - abs((int16_t)(pTrackObjInfo->t_pre_frame_center[1] - pTrackObjInfo->t_cur_frame_center[1]))  )<4  )
    {
      if (pTrackObjInfo->t_migration_diff[1] != 0 || abs((int16_t)(pTrackObjInfo->t_pre_frame_center[1] - pTrackObjInfo->t_cur_frame_center[1])) != 0 )
      {
        pTrackObjInfo->t_match_num[1]++;
      }
    }

    if (pTrackObjInfo->t_track_frame_num < 6 )
    {
      pTrackObjInfo->t_migration_diff[0] = abs((int16_t)(pTrackObjInfo->t_pre_frame_center[0] - pTrackObjInfo->t_cur_frame_center[0]));
    }
    else
    {
      pTrackObjInfo->t_migration_diff[0] = (uint16_t)(pTrackObjInfo->t_offset_x + 0.3);
    }

    pTrackObjInfo->t_migration_diff[1] = abs((int16_t)(pLabelObjStatus->t_obj_label_center[1] - pTrackObjInfo->t_pre_frame_center[1]));


    pTrackObjInfo->t_x_frame_location[pTrackObjInfo->t_track_frame_num] = pTrackObjInfo->t_cur_frame_center[0];
    pTrackObjInfo->t_y_frame_location[pTrackObjInfo->t_track_frame_num] = pTrackObjInfo->t_cur_frame_center[1];

	  //-----判断最高点
	  if(pTrackObjInfo->t_cur_frame_center[1] > pTrackObjInfo->t_track_bottom_point[1] )
	  {
		  pTrackObjInfo->t_track_bottom_point[0] = pTrackObjInfo->t_cur_frame_center[0] ;
		  pTrackObjInfo->t_track_bottom_point[1] = pTrackObjInfo->t_cur_frame_center[1] ;
		  pTrackObjInfo->t_track_bottom_point[2] = pTrackObjInfo->t_track_frame_num + 1;
	  }
	  if(pTrackObjInfo->t_cur_frame_center[1] < pTrackObjInfo->t_track_top_point[1] )
	  {
		  pTrackObjInfo->t_track_top_point[0] = pTrackObjInfo->t_cur_frame_center[0] ;
		  pTrackObjInfo->t_track_top_point[1] = pTrackObjInfo->t_cur_frame_center[1] ;
		  pTrackObjInfo->t_track_top_point[2] = pTrackObjInfo->t_track_frame_num + 1;
	  }

    //-----判断小区域中是否有轨迹
    if (ParamSet.b_little_region_flag )
    {
      if(pTrackObjInfo->t_cur_frame_center[0] > ParamSet.little_region_range[0] && pTrackObjInfo->t_cur_frame_center[0] < ParamSet.little_region_range[2] 
        && pTrackObjInfo->t_cur_frame_center[1] > ParamSet.little_region_range[1] && pTrackObjInfo->t_cur_frame_center[1] < ParamSet.little_region_range[3])
      {
        pTrackObjInfo->t_region_little_number++;
      }
    }

    //---- 判断目标离屏幕的距离
    if (pTrackObjInfo->t_cur_frame_center[1] >= (m_frame_height/2) )
    {
      TrackObject[m_track_num].t_obj_distance_flg = true ;
    }

    if (1 == pTrackObjInfo->t_track_direction[0] )
    {
      if (pTrackObjInfo->t_cur_frame_center[0] > pTrackObjInfo->t_pre_frame_center[0] + ParamSet.track_x_continue_threshold)
      {
        pTrackObjInfo->t_x_continue_num[0]++;
      }
      else
      {
        if (pTrackObjInfo->t_x_continue_num[0] < 5 )
        {
          pTrackObjInfo->t_x_continue_num[0] = 0 ;
        }
      }
      pTrackObjInfo->t_forecast_frm_center[0] = pTrackObjInfo->t_cur_frame_center[0] + pTrackObjInfo->t_migration_diff[0] ;
      if (pTrackObjInfo->t_forecast_frm_center[0] > m_frame_width )
      {
        pTrackObjInfo->t_continue_track_flag = false ;
      }
    }
    else
    {
      if (pTrackObjInfo->t_cur_frame_center[0]+ParamSet.track_x_continue_threshold < pTrackObjInfo->t_pre_frame_center[0])
      {
        pTrackObjInfo->t_x_continue_num[0]++;
      }
      else
      {
        if (pTrackObjInfo->t_x_continue_num[0] < 5 )
        {
          pTrackObjInfo->t_x_continue_num[0] = 0 ;
        }
      }

      if (pTrackObjInfo->t_cur_frame_center[0] <= pTrackObjInfo->t_migration_diff[0])  
        pTrackObjInfo->t_continue_track_flag = false ;
      else
        pTrackObjInfo->t_forecast_frm_center[0] = pTrackObjInfo->t_cur_frame_center[0] - pTrackObjInfo->t_migration_diff[0] ;
    }
    //----判断方向改变次数
    if (1 == pTrackObjInfo->t_track_direction[1] && pTrackObjInfo->t_cur_frame_center[1] > pTrackObjInfo->t_pre_frame_center[1] + 1)
    {
      pTrackObjInfo->t_x_continue_num[1]++ ;
    }
    if (0 == pTrackObjInfo->t_track_direction[1] && pTrackObjInfo->t_cur_frame_center[1] + 1 < pTrackObjInfo->t_pre_frame_center[1])
    {
      pTrackObjInfo->t_x_continue_num[1]++ ;
    }
    if (pTrackObjInfo->t_x_continue_num[1] >= 4)
    {
      pTrackObjInfo->t_continue_track_flag = false;
    }

    //-----确定物体竖直运动方向，从下至上 1，从上至下 0
    if (pTrackObjInfo->t_cur_frame_center[1] < pTrackObjInfo->t_pre_frame_center[1])
    {
      pTrackObjInfo->t_track_direction[1] = 1 ;

      if (pTrackObjInfo->t_cur_frame_center[1]+ ParamSet.track_x_continue_threshold < pTrackObjInfo->t_pre_frame_center[1])
        pTrackObjInfo->t_rise_frame_num[0]++;

      if (pTrackObjInfo->t_cur_frame_center[1] <= pTrackObjInfo->t_migration_diff[1])  
        pTrackObjInfo->t_continue_track_flag = false ;
      else
        pTrackObjInfo->t_forecast_frm_center[1] = pTrackObjInfo->t_cur_frame_center[1] - (uint16_t)((float)pTrackObjInfo->t_migration_diff[1] * 0.8f - 0.5); 			
    }
    else
    {

      pTrackObjInfo->t_track_direction[1] = 0 ;

      if (pTrackObjInfo->t_cur_frame_center[1] -ParamSet.track_x_continue_threshold > pTrackObjInfo->t_pre_frame_center[1])
        pTrackObjInfo->t_rise_frame_num[1]++ ;

      pTrackObjInfo->t_forecast_frm_center[1] = pTrackObjInfo->t_cur_frame_center[1] + (uint16_t)((float)pTrackObjInfo->t_migration_diff[1] * 0.8f + 0.5); 
      if (pTrackObjInfo->t_forecast_frm_center[1] > pFrame_in->getHeight())
      {
        pTrackObjInfo->t_continue_track_flag = false ;
      }
    }

    //----判断是否在直线内
    if ( !pTrackObjInfo->t_line_range_flag[0] || !pTrackObjInfo->t_line_range_flag[1])
    {
      if(pTrackObjInfo->t_cur_frame_center[1] > 3 && pTrackObjInfo->t_cur_frame_center[1] < pFrame_in->getHeight())
      {
        int16_t LineTempA1 = (int16_t)( ( (float)pTrackObjInfo->t_cur_frame_center[1] - ParamSet.t_line_first_location[1] ) / ParamSet.t_line_first_location[0] );
        int16_t LineTempA2 = (int16_t)( ( (float)pTrackObjInfo->t_cur_frame_center[1] - ParamSet.t_line_first_location[3] ) / ParamSet.t_line_first_location[2] );
        if(LineTempA1 > 5  && LineTempA1 < pFrame_in->getWidth() )
        {
          if(pTrackObjInfo->t_cur_frame_center[0] > LineTempA1  )
            pTrackObjInfo->t_line_range_flag[0] = true ;
        }
        if(LineTempA2 > 5  && LineTempA2 < pFrame_in->getWidth() )
        {
          if(pTrackObjInfo->t_cur_frame_center[0] < LineTempA2  )
          {
            pTrackObjInfo->t_line_range_flag[1] = true ;
          }

        }

      }
    }

    //----5帧以上 如果连续数等于0，返回失败
    if (pTrackObjInfo->t_track_frame_num > 4 && pTrackObjInfo->t_x_continue_num[0] == 0 )
    {
      pTrackObjInfo->t_continue_track_flag = false ;
    }
  }

  else //未找到与预测目标相匹配的对象
  {
    pTrackObjInfo->t_all_lost_frame_num++;

    if ( ( pTrackObjInfo->t_find_obj_number > 4 && pTrackObjInfo->t_lost_frame_num > 5)||
         ( pTrackObjInfo->t_find_obj_number <= 4 && pTrackObjInfo->t_lost_frame_num > 4)||
         ( pTrackObjInfo->t_track_frame_num > ParamSet.track_max_frame_num )||
         ( pTrackObjInfo->t_track_frame_num > 5 && pTrackObjInfo->t_x_continue_num[0] == 0 )||
         ( pTrackObjInfo->t_find_obj_number < pTrackObjInfo->t_track_frame_num * 0.5 && pTrackObjInfo->t_all_lost_frame_num > 6 )||
         ( pTrackObjInfo->t_find_obj_number >= pTrackObjInfo->t_track_frame_num * 0.5 && pTrackObjInfo->t_all_lost_frame_num > 10 )
         )
    {
      pTrackObjInfo->t_continue_track_flag = false ;
      return m_nTracked;
    }

    pTrackObjInfo->t_lost_frame_num++;
    pTrackObjInfo->t_track_frame_num++;        
    pTrackObjInfo->t_pre_frame_center[0] = pTrackObjInfo->t_cur_frame_center[0];  
    pTrackObjInfo->t_pre_frame_center[1] = pTrackObjInfo->t_cur_frame_center[1];
    pTrackObjInfo->t_cur_frame_center[0] = pTrackObjInfo->t_forecast_frm_center[0]; 
    pTrackObjInfo->t_cur_frame_center[1] = pTrackObjInfo->t_forecast_frm_center[1];
    pTrackObjInfo->t_migration_diff[0] = (uint16_t)pTrackObjInfo->t_offset_x ;
    pTrackObjInfo->t_migration_diff[1] = abs((int16_t)(pTrackObjInfo->t_cur_frame_center[1] - pTrackObjInfo->t_pre_frame_center[1]));

    //----预测新坐标的位置
    if ( 1 == pTrackObjInfo->t_track_direction[0] )
    {
      pTrackObjInfo->t_forecast_frm_center[0] = pTrackObjInfo->t_cur_frame_center[0] + pTrackObjInfo->t_migration_diff[0] ;
      if (pTrackObjInfo->t_forecast_frm_center[0] >= m_frame_width )
      {
        pTrackObjInfo->t_continue_track_flag = false ;
      }
    }
    else
    {
      if (pTrackObjInfo->t_cur_frame_center[0] <= pTrackObjInfo->t_migration_diff[0] )
      {
        pTrackObjInfo->t_continue_track_flag = false ;
      }
      else
        pTrackObjInfo->t_forecast_frm_center[0] = pTrackObjInfo->t_cur_frame_center[0] - pTrackObjInfo->t_migration_diff[0] ; 
    }

    //----y方向上如果没有偏移，设置偏移为1 方向向下
    if ( 0 == pTrackObjInfo->t_migration_diff[1])   
    {
      pTrackObjInfo->t_track_direction[1] = 0 ;
      pTrackObjInfo->t_migration_diff[1] = 1 ;
    }

    //----预测新坐标的位置
    if ( 1 == pTrackObjInfo->t_track_direction[1])
    {
      if (pTrackObjInfo->t_cur_frame_center[1] <= pTrackObjInfo->t_migration_diff[1] )
      {
        pTrackObjInfo->t_continue_track_flag = false ;
      }
      else
        pTrackObjInfo->t_forecast_frm_center[1] = pTrackObjInfo->t_cur_frame_center[1] - (uint16_t)((float)pTrackObjInfo->t_migration_diff[1] * 0.8f + 0.5);  
    }
    else
    {
      pTrackObjInfo->t_forecast_frm_center[1] = pTrackObjInfo->t_cur_frame_center[1] + (uint16_t)((float)pTrackObjInfo->t_migration_diff[1] * 0.8f + 0.5); 
      if (pTrackObjInfo->t_pre_frame_center[1] >= m_frame_height )
      {
        pTrackObjInfo->t_continue_track_flag = false ;
      }
    }

  }
  return m_nTracked;
}
/***************************************
*
*
*
*
***************************************/
ErrVal 
CParabolaDetect::FindDetectedObject(CFrameContainer* const pFrame_in, ObjLabelInfoStruct* pCurrObjInfo)
{
  ASSERT(pCurrObjInfo);
  ASSERT(pFrame_in);

  if (m_pre_frm_num == 0) 
  {
    return m_nTracked;
  }

  uint16_t i = 0 ;
  uint16_t j = 0 ;
  int16_t x_curr_cen = 0 ;
  int16_t y_curr_cen = 0 ;
  int16_t p_temp_fir_value = 0;
  int16_t p_temp_sec_value = 0;
  int16_t v_obj_num = 0 ;

  BOOL cbv_find_flg = FALSE;
  BOOL cbv_add_flg = FALSE;
  BOOL no_track_flg = FALSE ;

  int32_t previous_WhiteSpotNum = 0 ;  // 上一帧物体白点个数
  int32_t curr_white_num  = 0 ;
  int16_t last_x = 0 ;                  // 上一帧物体重心X
  int16_t last_y = 0 ;                  // 上一帧物体重心Y

  x_curr_cen = pCurrObjInfo->t_obj_label_center[0];
  y_curr_cen = pCurrObjInfo->t_obj_label_center[1];
  curr_white_num = pCurrObjInfo->t_obj_white_spot_num;

  if (0 == m_track_num)
  {
    no_track_flg = TRUE ;
  }

  cbv_find_flg = FALSE ;
  cbv_add_flg  = FALSE ;

  //----找大小距离接近的点作为跟踪目标
  for (i = 0; i < m_pre_frm_num; i++)                         
  {
    if(PreLabelInfo[i].t_flag)
    {
      last_x = PreLabelInfo[i].t_obj_label_center[0];          
      last_y = PreLabelInfo[i].t_obj_label_center[1];  

      previous_WhiteSpotNum = PreLabelInfo[i].t_obj_white_spot_num;

      if ( ( abs(last_x - x_curr_cen)  <= 20) && ( abs(last_y - y_curr_cen) <= 20 )
        && (abs((int16_t)(last_x - x_curr_cen ))>= ParamSet.track_x_offset_value) 
        && (   (previous_WhiteSpotNum <  30 && abs( previous_WhiteSpotNum - curr_white_num ) < previous_WhiteSpotNum * 1.2 )
        ||(previous_WhiteSpotNum >= 30 && abs( previous_WhiteSpotNum - curr_white_num ) < previous_WhiteSpotNum * 0.9 )
        )
        )
      {
        if (cbv_find_flg)
        {
          p_temp_sec_value = abs((uint16_t)(previous_WhiteSpotNum - curr_white_num));
          if(p_temp_fir_value > p_temp_sec_value)
          {
            p_temp_fir_value = p_temp_sec_value;
            v_obj_num = i;
          }
        }
        else
        {
          p_temp_fir_value = abs((uint16_t)(previous_WhiteSpotNum - curr_white_num));
          v_obj_num = i;
        }

        cbv_find_flg = true ;

      } 
    }
  }

  //----判断该目标是否是跟踪预测中的目标
  if (cbv_find_flg)
  {
    cbv_add_flg = TRUE ;

    last_x = PreLabelInfo[v_obj_num].t_obj_label_center[0];          
    last_y = PreLabelInfo[v_obj_num].t_obj_label_center[1];

    if (!no_track_flg)
    {
      for (j = 0 ; j < m_track_num ; j++ )
      {
        if (  last_x == TrackObject[j].t_pre_frame_center[0] 
        && last_y == TrackObject[j].t_pre_frame_center[1]
        && x_curr_cen == TrackObject[j].t_cur_frame_center[0]
        && y_curr_cen ==  TrackObject[j].t_cur_frame_center[1])
        {
          cbv_add_flg = FALSE ;
          break ;
        }

      }
    }
  }

  //----建立跟踪信息
  LabelObjStatus * p_find_obj_info = &TrackObject[m_track_num] ;
  if (cbv_add_flg) 
  {
    p_find_obj_info->t_track_alarm_flag = false ;
    p_find_obj_info->t_find_obj_number = 2 ;
    p_find_obj_info->t_continue_track_flag = true ;
    p_find_obj_info->t_pre_frame_center[0] = last_x;  
    p_find_obj_info->t_pre_frame_center[1] = last_y;
    p_find_obj_info->t_cur_frame_center[0] = pCurrObjInfo->t_obj_label_center[0]; 
    p_find_obj_info->t_cur_frame_center[1] = pCurrObjInfo->t_obj_label_center[1];
    p_find_obj_info->t_white_spot_num    = pCurrObjInfo->t_obj_white_spot_num ;  

    p_find_obj_info->t_offset_x = (float)abs((int16_t)(x_curr_cen - last_x));

    p_find_obj_info->t_migration_diff[0] = abs(x_curr_cen - last_x);
    p_find_obj_info->t_migration_diff[1] = abs(y_curr_cen - last_y);
    p_find_obj_info->t_origin_frame_center[0] = last_x;
    p_find_obj_info->t_origin_frame_center[1] = last_y;

    p_find_obj_info->t_x_frame_location[0] = p_find_obj_info->t_cur_frame_center[0];/////
    p_find_obj_info->t_y_frame_location[0] = p_find_obj_info->t_cur_frame_center[1];/////

    p_find_obj_info->t_lost_frame_num = 0 ;     
    p_find_obj_info->t_rise_frame_num[1] = 0 ;
    p_find_obj_info->t_rise_frame_num[0] = 0 ;
    p_find_obj_info->t_track_frame_num = 0 ;
    p_find_obj_info->t_x_continue_num[1] = 0 ;
    p_find_obj_info->t_x_continue_num[0] = 0 ;
    p_find_obj_info->t_all_lost_frame_num = 0 ;
    p_find_obj_info->t_match_num[0] = 0 ;
    p_find_obj_info->t_match_num[1] = 0 ;

    //-----求最高点
    if(p_find_obj_info->t_pre_frame_center[1] > p_find_obj_info->t_cur_frame_center[1])
    {
      p_find_obj_info->t_track_bottom_point[0] = p_find_obj_info->t_pre_frame_center[0] ;
      p_find_obj_info->t_track_bottom_point[1] = p_find_obj_info->t_pre_frame_center[1] ;
      p_find_obj_info->t_track_bottom_point[2] = 1 ;
      p_find_obj_info->t_track_top_point[0] = p_find_obj_info->t_cur_frame_center[0] ;
      p_find_obj_info->t_track_top_point[1] = p_find_obj_info->t_cur_frame_center[1] ;
      p_find_obj_info->t_track_top_point[2] = 0 ;

    }
    else
    {
      p_find_obj_info->t_track_bottom_point[0] = p_find_obj_info->t_cur_frame_center[0] ;
      p_find_obj_info->t_track_bottom_point[1] = p_find_obj_info->t_cur_frame_center[1];
      p_find_obj_info->t_track_bottom_point[2] = 0 ;
      p_find_obj_info->t_track_top_point[0] = p_find_obj_info->t_pre_frame_center[0];
      p_find_obj_info->t_track_top_point[1] = p_find_obj_info->t_pre_frame_center[1];
      p_find_obj_info->t_track_top_point[2] = 1 ;
    }

    //-----判断轨迹是否在小范围中
    if (ParamSet.b_little_region_flag )
    {
      if(p_find_obj_info->t_pre_frame_center[0] > ParamSet.little_region_range[0] && p_find_obj_info->t_pre_frame_center[0] < ParamSet.little_region_range[2] 
      && p_find_obj_info->t_pre_frame_center[1] > ParamSet.little_region_range[1] && p_find_obj_info->t_pre_frame_center[1] < ParamSet.little_region_range[3])
      {
        p_find_obj_info->t_region_little_number++;
      }
      if(p_find_obj_info->t_cur_frame_center[0] > ParamSet.little_region_range[0] && p_find_obj_info->t_cur_frame_center[0] < ParamSet.little_region_range[2] 
      && p_find_obj_info->t_cur_frame_center[1] > ParamSet.little_region_range[1] && p_find_obj_info->t_cur_frame_center[1] < ParamSet.little_region_range[3])
      {
        p_find_obj_info->t_region_little_number++;
      }
    }


    //---- 判断目标离屏幕的距离
    if (p_find_obj_info->t_pre_frame_center[1] >= (pFrame_in->getHeight()/2) || p_find_obj_info->t_cur_frame_center[1] >= (pFrame_in->getHeight()/2) )
    {
      p_find_obj_info->t_obj_distance_flg = true ;
    }
    else
    {
      p_find_obj_info->t_obj_distance_flg = false ;
    }

    //----确定物体水平运动方向，从左至右 1，从右至左 0
    if(p_find_obj_info->t_cur_frame_center[0] > last_x)
    {
      p_find_obj_info->t_track_direction[0] = 1;
      p_find_obj_info->t_forecast_frm_center[0] = p_find_obj_info->t_cur_frame_center[0] + p_find_obj_info->t_migration_diff[0] ;
      if (p_find_obj_info->t_forecast_frm_center[0] > pFrame_in->getWidth())
      {
        p_find_obj_info->t_continue_track_flag = false ;
      }
    }
    else
    {
      p_find_obj_info->t_track_direction[0] = 0;
      if ( (p_find_obj_info->t_cur_frame_center[0] <= p_find_obj_info->t_migration_diff[0])  )
        p_find_obj_info->t_continue_track_flag = false ;
      else
        p_find_obj_info->t_forecast_frm_center[0] = p_find_obj_info->t_cur_frame_center[0] - p_find_obj_info->t_migration_diff[0] ;
    }


    //----确定物体竖直运动方向，从下至上 1，从上至下 0
    if(p_find_obj_info->t_cur_frame_center[1] <= last_y)
    {
      p_find_obj_info->t_track_direction[1] = 1 ;
      if ( (p_find_obj_info->t_cur_frame_center[1] <= p_find_obj_info->t_migration_diff[1])  )
        p_find_obj_info->t_continue_track_flag = false ;
      else
        p_find_obj_info->t_forecast_frm_center[1] = p_find_obj_info->t_cur_frame_center[1] - (uint16_t)((float)p_find_obj_info->t_migration_diff[1] * 0.8f + 0.5) ;
    }
    else
    {
      p_find_obj_info->t_track_direction[1] = 0 ;
      p_find_obj_info->t_forecast_frm_center[1] = p_find_obj_info->t_cur_frame_center[1] + (uint16_t)((float)p_find_obj_info->t_migration_diff[1] * 0.8f + 0.5) ;
      if (p_find_obj_info->t_forecast_frm_center[1] > pFrame_in->getHeight())
      {
        p_find_obj_info->t_continue_track_flag = false ;
      }
    }

    //----判断是否在直线两边
    p_find_obj_info->t_line_range_flag[0] = false ;
    p_find_obj_info->t_line_range_flag[1] = false ;

    if(p_find_obj_info->t_cur_frame_center[1] > 3 && p_find_obj_info->t_cur_frame_center[1] < pFrame_in->getHeight())
    {
      int16_t LineTempA1 = (int16_t)( ( (float)p_find_obj_info->t_cur_frame_center[1] - ParamSet.t_line_first_location[1] ) / ParamSet.t_line_first_location[0] );
      int16_t LineTempA2 = (int16_t)( ( (float)p_find_obj_info->t_cur_frame_center[1] - ParamSet.t_line_first_location[3] ) / ParamSet.t_line_first_location[2] );
      if(LineTempA1 > 5 && LineTempA1 > m_frame_width )
      {
        if(p_find_obj_info->t_cur_frame_center[0] > LineTempA1  )
          p_find_obj_info->t_line_range_flag[0] = true ;
      }
      if(LineTempA2 > 5 && LineTempA2 > m_frame_height )
      {
        if(p_find_obj_info->t_cur_frame_center[0] < LineTempA2  )
          p_find_obj_info->t_line_range_flag[1] = true ;
      }

    }


    m_track_num ++ ;
    if (m_track_num >= m_nTrackObjectMaxNum )
    {
      m_track_num = (uint16_t)(m_nTrackObjectMaxNum * 0.8) ;
      return m_nTracked;
    }
    cbv_find_flg = FALSE ;
  }

  return m_nTracked;
}

//////////////////////////////////////////////////////////////////////////
/// 清除标定缓存内容
//////////////////////////////////////////////////////////////////////////
ErrVal 
CParabolaDetect::ClearObjectBuffer()
{
  memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum );
  m_curr_frm_num = 0 ;
  ROK();
}

/************************************************
$   ForecastObjectDetect : 寻找是否有匹配的目标
$
$
*************************************************/
ErrVal
CParabolaDetect::ForecastObjectDetect(const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out)
{
  ASSERT(pFrame_in);
  ASSERT(pFrame_out);

  uint16_t i =0 ;
  uint16_t j =0 ;
  uint32_t const v_frame_width  = m_frame_width;
  uint32_t const v_frame_height = m_frame_height;

  m_alarm_flg = FALSE;  

  GlobalLabelObject(pFrame_out); //mark objects

  if ( m_alarm_delay > 0 )
  {
     m_alarm_delay--;
  }

  if(!m_tracked_obj_flag)
  {
    if (m_curr_frm_num == 0)
    {
      RERR();
    }

    //----跟踪的目标各项赋值
    memcpy(PreLabelInfo, ObjLabelInfo,sizeof(ObjLabelInfoStruct) * m_curr_frm_num) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;

    m_pre_frm_num = m_curr_frm_num ;
    m_tracked_obj_flag = TRUE ;
    m_curr_frm_num = 0 ;

    ROK();
  }
  else
  {

    //----目标数太多，直接清零
    if (m_curr_frm_num > 50 )
    {
      m_tracked_obj_flag = false ;
      memset(PreLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;
      memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
      m_track_num = 0 ;
      m_curr_frm_num = 0 ;
      ROK();
    }

    //----预测跟踪帧的目标
    for (i = 0 ;i < m_track_num; i++)
    {
      DetectedTrackedObject(pFrame_in, pFrame_out, &TrackObject[i]); 
    }

    //----预测当前帧的目标搜索位置
    for (i = 0 ;i < m_curr_frm_num; i++)
    {
      if (ObjLabelInfo[i].t_flag)
      {
        FindDetectedObject(pFrame_out, &ObjLabelInfo[i]); 
      }

    }

    memcpy(PreLabelInfo, ObjLabelInfo,sizeof(ObjLabelInfoStruct) * m_curr_frm_num) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;

    m_pre_frm_num = m_curr_frm_num ;
    m_curr_frm_num = 0 ;

    if(m_track_num == 0)
    {
      m_tracked_obj_flag = FALSE;
    }

    //----delete repeated object
    uint16_t m_temp_track_obj_num = 0;
    LabelObjStatus m_pTempObjTrackStatus[m_nTrackObjectMaxNum] ;
    memset(m_pTempObjTrackStatus, 0, m_nTrackObjectMaxNum * sizeof(LabelObjStatus) ) ;

	for (i = 0; i < m_track_num; i++)
	{

		if (TrackObject[i].t_continue_track_flag && m_alarm_delay == 0 )
		{
			if (TrackAlarmObject( i ))
			{ 
				if (CurveContrast(&TrackObject[i]) )
				{
					TrackObject[i].t_track_alarm_flag = true ;
					m_alarm_flg = TRUE ;
					m_alarm_delay = 20 ;
				}  
			}

			memcpy(&m_pTempObjTrackStatus[m_temp_track_obj_num],&TrackObject[i],sizeof(LabelObjStatus));
			m_temp_track_obj_num++;
		}

	}

    if (0 == m_temp_track_obj_num)
    {
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
      m_tracked_obj_flag = FALSE;
    }
    else if (m_track_num == m_temp_track_obj_num)
    {
      m_tracked_obj_flag = TRUE;
    }
    else if( m_temp_track_obj_num > m_nTrackObjectMaxNum * 0.8 )
    {
      m_tracked_obj_flag = TRUE;
      m_track_num = 0 ;
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));

    }
    else
    {
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
      memcpy(TrackObject,m_pTempObjTrackStatus,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));

      m_track_num = m_temp_track_obj_num;
      m_tracked_obj_flag = TRUE;
    }

    ROK();
  }
}

ErrVal
CParabolaDetect::Statistics_obj_curinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                                         const  CFrameContainer* const pFrame_curr_in,
                                         const CFrameContainer* const pFrame_bin_in)

{
  long v_curr_frm_num = objDList_inout->GetCount();
  if (0 == v_curr_frm_num)
  {
    memset( &ObjLabelInfo, 0, m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct) );
    ROK();
  }

  uint8_t b_mean = 0, g_mean = 0, r_mean = 0;

  for (uint16_t i = 1 ; i <= v_curr_frm_num; i++)
  {
    ObjLabelInfo[i - 1].t_obj_label_center[0]   =   objDList_inout->GetAt(i)->m_nOuterRect[0] + (objDList_inout->GetAt(i)->m_nOuterRect[2])/2;
    ObjLabelInfo[i - 1].t_obj_label_center[1]   =   objDList_inout->GetAt(i)->m_nOuterRect[1] + (objDList_inout->GetAt(i)->m_nOuterRect[3])/2;
    ObjLabelInfo[i - 1].t_obj_label_rect[0]     =   objDList_inout->GetAt(i)->m_nOuterRect[0];
    ObjLabelInfo[i - 1].t_obj_label_rect[1]     =   objDList_inout->GetAt(i)->m_nOuterRect[1];
    ObjLabelInfo[i - 1].t_obj_label_rect[2]     =   objDList_inout->GetAt(i)->m_nOuterRect[2];
    ObjLabelInfo[i - 1].t_obj_label_rect[3]     =   objDList_inout->GetAt(i)->m_nOuterRect[3];
    ObjLabelInfo[i - 1].t_obj_white_spot_num    =   objDList_inout->GetAt(i)->m_nXYDotPlus[2];

  }

  ROK();

}
ErrVal 
CParabolaDetect::AddBlackBlock(uint8_t* pFrame_in, uint16_t left, uint16_t bottom, uint16_t right, uint16_t top )
{
  ASSERT(pFrame_in);

  uint16_t j = 0 ; 
  uint16_t const TempWide = right - left ;
  uint8_t * p_data = pFrame_in ;

  if (m_frame_width < right )
    right = m_frame_width;
  if (m_frame_height < top )
    top = m_frame_height;

  for( j = bottom; j < top; j++ )
  {
    memset( p_data+j*m_frame_width + left, 0, TempWide );
  }
  ROK();
}

void
CParabolaDetect::AddStraightLine1(float x1, float y1, float x2, float y2 )
{
	ParamSet.t_line_first_location[0] = ( y2 - y1)/( x2 - x1);
	ParamSet.t_line_first_location[1] = ( y2*x1 - y1*x2)/(x1 - x2);
}
void
CParabolaDetect::AddStraightLine2(float x1, float y1, float x2, float y2 )
{
	ParamSet.t_line_first_location[2] = ( y2 - y1)/( x2 - x1);
	ParamSet.t_line_first_location[3] = ( y2*x1 - y1*x2)/(x1 - x2);
}
void
CParabolaDetect::AddStraightLineCurver(float x1, float y1, float x2, float y2 )
{
  ParamSet.t_line_second_location[0] = ( y2 - y1)/( x2 - x1);
  ParamSet.t_line_second_location[1] = ( y2*x1 - y1*x2)/(x1 - x2);
}
ErrVal 
CParabolaDetect::AddBlackLine(uint8_t* pFrame_in, float x1, float y1, float x2, float y2 )
{
  ASSERT(pFrame_in);

  uint16_t i = 0 ; 
  uint16_t j = 0 ;
  int16_t x = 0 ;
  int16_t y = 0 ;
  uint16_t framewidth  = m_frame_width  ;    //帧宽
  uint16_t frameheight = m_frame_height ;    //图像高

  uint8_t * p_data = pFrame_in ;

  float na = ( y2 - y1)/( x2 - x1);
  float nb = ( y2*x1 - y1*x2)/(x1 - x2);

  for (j = 5 ; j < frameheight-5 ; j++)
  {
      x = (int16_t)( ( (float)j - nb ) / na );
      if (x > 5 && x< framewidth-5)
      {
        memset(p_data + framewidth * j + x , 0 , framewidth -x );
      }
      
  }

  ROK();
}
BOOL CParabolaDetect::ThresholdChaseObject(CFrameContainer* const pFrame_in, CFrameContainer* const pFrame_out, LabelObjStatus* pTrackAreaObj )
{
  ASSERT(pTrackAreaObj);

  uint16_t i = 0 ;
  int32_t TempWhiteSpot = 0 ;
  int32_t RoundWhiteSpot = 0 ;
  int16_t ForecastCenX = pTrackAreaObj->t_forecast_frm_center[0] ;
  int16_t ForecastCenY = pTrackAreaObj->t_forecast_frm_center[1] ;
  int32_t ForeWhiteSpotNum = pTrackAreaObj->t_white_spot_num ;
  int16_t CurrTrackX = pTrackAreaObj->t_cur_frame_center[0];


  int32_t curr_white_num  = 0 ;
  int16_t CurrX  = 0 ;                  // 
  int16_t CurrY  = 0 ;                  // 
  int16_t ptemp0 = 0 ;
  int16_t ptemp1 = 0 ;
  int16_t objNum = 0 ;
  int16_t TempDiff = 0 ;
  bool    LastDisFlg  = false ;
  bool    NewDisFlg = false ;
  uint16_t framewidth  = pFrame_in->getWidth();    //帧宽
  uint16_t frameheight=pFrame_in->getHeight();//图像高


  int16_t DetectX = 10 + pTrackAreaObj->t_lost_frame_num*2 ;
  int16_t DetectY = 10 + pTrackAreaObj->t_lost_frame_num*2 ;

  bool FindFalg = FALSE;

  if (1 == pTrackAreaObj->t_track_direction[0])
  {

    for (i = 0; i < m_area_frm_num; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        CurrX = AreaInfo[i].t_obj_label_center[0];          
        CurrY = AreaInfo[i].t_obj_label_center[1]; 
        curr_white_num = AreaInfo[i].t_obj_white_spot_num ;


        if ( ForeWhiteSpotNum < 30 )
        {

          TempWhiteSpot = (int32_t)( ForeWhiteSpotNum * 0.8 );
          RoundWhiteSpot = (int32_t)( ForeWhiteSpotNum  * 2 );
        }
        else
        {
          TempWhiteSpot = (int32_t)( ForeWhiteSpotNum * 0.8 );
          RoundWhiteSpot = (int32_t)( ForeWhiteSpotNum * 2  ) ;
        }
        if (ForeWhiteSpotNum <= 15 )
        {
          TempWhiteSpot = 15 ;
          RoundWhiteSpot = 15 ;
        }

        if( ( CurrX - CurrTrackX >= 3 )   && (  ( abs(CurrX - ForecastCenX) <= DetectX &&  (CurrY - ForecastCenY) <= DetectY  && abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) < TempWhiteSpot )
          ||(abs(CurrX - ForecastCenX) <= 3 &&  (CurrY - ForecastCenY) <= 4  && abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) < RoundWhiteSpot )
          )
          )
        {
          if(!FindFalg)
          {
            ptemp0 = abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) ;
            objNum = i;
          }
          else 
          {
            ptemp1 = abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) ;
            if(ptemp0 > ptemp1)
            {
              ptemp0 = ptemp1;
              objNum = i;
            }
          }
          FindFalg = TRUE;   

        } 
      }
    }
  }
  else
  {
    for (i = 0; i < m_area_frm_num; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        CurrX = AreaInfo[i].t_obj_label_center[0];          
        CurrY = AreaInfo[i].t_obj_label_center[1]; 
        curr_white_num = AreaInfo[i].t_obj_white_spot_num ;

        if ( ForeWhiteSpotNum < 30 )
        {
          TempWhiteSpot = (int32_t)(ForeWhiteSpotNum * 0.8 );
          RoundWhiteSpot = (int32_t)(ForeWhiteSpotNum * 2 );
        }
        else
        {
          TempWhiteSpot = (int32_t)(ForeWhiteSpotNum * 0.8);
          RoundWhiteSpot = (int32_t)(ForeWhiteSpotNum * 2 ) ;
        }
        if (ForeWhiteSpotNum <= 15 )
        {
          TempWhiteSpot = 15 ;
          RoundWhiteSpot = 15 ;
        }

        if( ( CurrTrackX -CurrX >= 3 )   && (  ( abs(CurrX - ForecastCenX) <= DetectX &&  (CurrY - ForecastCenY) <= DetectY  && abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) < TempWhiteSpot )
          ||(abs(CurrX - ForecastCenX) <= 3 &&  (CurrY - ForecastCenY) <= 4  && abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) < RoundWhiteSpot )
          )
          )
        {
          if(!FindFalg)
          {
            ptemp0 = abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) ;
            objNum = i;
          }
          else 
          {
            ptemp1 = abs((int16_t)(ForeWhiteSpotNum - curr_white_num)) ;
            if(ptemp0 > ptemp1)
            {
              ptemp0 = ptemp1;
              objNum = i;
            }
          }

          FindFalg = TRUE;   

        } 
      }
    }
  }

  ChangeObjectValue(pFrame_in, &AreaInfo[objNum], pTrackAreaObj, FindFalg);

  return FindFalg ;

}
//////////////////////////////////////////////////////////////////////////
/// 均值平滑函数，nFilterSelect 指定平滑选用的滤波器类型，StartSmoothLine 指定开始平滑的行
/// nFilterSelect,StartSmoothLine默认为0
//////////////////////////////////////////////////////////////////////////
ErrVal 
CParabolaDetect::averageSmoothRgb(const CFrameContainer *const pFrame_decoded_inout,uint16_t nFilterSelect,uint16_t StartSmoothLine ) const
{
  ASSERT( pFrame_decoded_inout );

  ///////////////////
  CFrameContainer tmp(*pFrame_decoded_inout);
  cvSmooth(tmp.getImage(), pFrame_decoded_inout->m_pIplImage);
  ROK();
  ///////////////////

  const uint16_t nWidth=pFrame_decoded_inout->getWidth();//图像的宽
  const uint16_t nHeight=pFrame_decoded_inout->getHeight();//图像高

  uint8_t* pRGB = new uint8_t[pFrame_decoded_inout->getRgbSize()];
  memcpy(pRGB,pFrame_decoded_inout->m_BmpBuffer,pFrame_decoded_inout->getRgbSize());
  if (StartSmoothLine >= nHeight)
    return 0;//return ?

  int16_t i = 0, j = 0, m = 0, n = 0; 
  uint16_t sumB = 0, sumG = 0, sumR = 0;
  for( i = StartSmoothLine + 1; i < nHeight-1; ++i )
  {
    for( j = 1; j < nWidth-1; ++j )
    {
      sumB = 0, sumG = 0, sumR = 0;
      for (m = -1;m < 2;m++)
      {
        for (n = -1;n < 2;n++)
        {
          //滤波器类型0，1 1 1
          ///            1 1 1
          ///            1 1 1
          if(nFilterSelect == 0)
          {
            sumB += pRGB[3*(i+m)*nWidth+3*(j+n) + 0];
            sumG += pRGB[3*(i+m)*nWidth+3*(j+n) + 1];
            sumR += pRGB[3*(i+m)*nWidth+3*(j+n) + 2];
          }
          //滤波器类型1，1 1 1
          ///            1 2 1
          ///            1 1 1
          else if(nFilterSelect == 1)
          {
            if(m == 0 && n == 0)
            {
              sumB += 2*pRGB[3*(i+m)*nWidth+3*(j+n) + 0];
              sumG += 2*pRGB[3*(i+m)*nWidth+3*(j+n) + 1];
              sumR += 2*pRGB[3*(i+m)*nWidth+3*(j+n) + 2];
            }
            else
            {
              sumB += pRGB[3*(i+m)*nWidth+3*(j+n) + 0];
              sumG += pRGB[3*(i+m)*nWidth+3*(j+n) + 1];
              sumR += pRGB[3*(i+m)*nWidth+3*(j+n) + 2];
            }        
          }
          //滤波器类型2，1 2 1
          ///            2 4 2
          ///            1 2 1
          else
          {
            if(m == 0 || n == 0)
            {
              if(m == 0 && n == 0)
              {
                sumB += 4*pRGB[3*(i+m)*nWidth+3*(j+n) + 0];
                sumG += 4*pRGB[3*(i+m)*nWidth+3*(j+n) + 1];
                sumR += 4*pRGB[3*(i+m)*nWidth+3*(j+n) + 2];
              }
              else
              {
                sumB += 2*pRGB[3*(i+m)*nWidth+3*(j+n) + 0];
                sumG += 2*pRGB[3*(i+m)*nWidth+3*(j+n) + 1];
                sumR += 2*pRGB[3*(i+m)*nWidth+3*(j+n) + 2];
              }
            }
            else
            {
              sumB += pRGB[3*(i+m)*nWidth+3*(j+n) + 0];
              sumG += pRGB[3*(i+m)*nWidth+3*(j+n) + 1];
              sumR += pRGB[3*(i+m)*nWidth+3*(j+n) + 2];
            }        
          }       
        }
      }
      if(nFilterSelect == 0)
      {
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 0] = (uint8_t)(sumB/9);
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 1] = (uint8_t)(sumG/9);
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 2] = (uint8_t)(sumR/9);
      }
      else if(nFilterSelect == 1)
      {
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 0] = (uint8_t)(sumB/10);
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 1] = (uint8_t)(sumG/10);
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 2] = (uint8_t)(sumR/10);
      }
      else
      {
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 0] = (uint8_t)(sumB/16);
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 1] = (uint8_t)(sumG/16);
        pFrame_decoded_inout->m_BmpBuffer[3*i*nWidth+3*j + 2] = (uint8_t)(sumR/16);
      } 
    }
  }
  if(pRGB)
    delete [] pRGB;
  ROK();
}

//////////////////////////////////////////////////////////////////////////
/// 去掉图像边界
//////////////////////////////////////////////////////////////////////////
void CParabolaDetect::RemoveBorder(CFrameContainer* const pFrame_in)
{
  ASSERT(pFrame_in);

  uint16_t i = 0 ;
  uint16_t v_frame_width   = m_frame_width ;    //帧宽
  uint16_t v_frame_height  = m_frame_height;    //帧高
  uint16_t v_border_right  = 5 ;
  uint16_t v_border_top    = 5 ;
  uint16_t v_border_bottom = 5 ;
  uint16_t v_border_left   = 5 ;

  memset( pFrame_in->m_YuvPlane[0], 0, v_frame_width * v_border_top );
  memset( pFrame_in->m_YuvPlane[0]+(v_frame_height-v_border_bottom)*v_frame_width, 0, v_frame_width * v_border_bottom );

  for (i = 1; i < v_frame_height; i++)
  {
    memset( pFrame_in->m_YuvPlane[0]+v_frame_width*i + v_frame_width-v_border_right, 0, v_border_left+v_border_right );
  }

  return ;
}
//////////////////////////////////////////////////////////////////////////
/// |frame3 - frame2| & |frame2 - frame1|，第3帧与第2帧相减二值化后再与
/// 第2帧与第1帧相减二值化后的结果相与，得到最终二值化后的信息
//////////////////////////////////////////////////////////////////////////
ErrVal 
CParabolaDetect::BinarizeSub(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out, int16_t v_threshold, uint16_t* pRectangle )
{
  uint16_t v_i = 0 ;
  uint16_t v_j = 0 ;

  uint8_t* vp_rgb_0 = m_para_detect_image[0];
  uint8_t* vp_rgb_1 = m_para_detect_image[1];
  uint8_t* vp_rgb_2 = pFrame_in->m_BmpBuffer;

  uint8_t* vp_in_y = pFrame_in->m_YuvPlane[0];
  uint8_t* vp_out_y = pFrame_out->m_YuvPlane[0];

  uint16_t vi_frame_width  = m_frame_width;   //图像的宽
  uint16_t vi_frame_height = m_frame_height;  //图像的高

  int32_t vi_pos_start = 0   ;
  int32_t vi_pos_y     = 0   ;
  int16_t vi_pos_end   = 0   ;
  uint16_t* vp_rect  = NULL;

  int16_t v_temp_threshold = v_threshold ;

//   SYSTEMTIME Systemtime ;
//   GetLocalTime(&Systemtime);
// 
//   //-----夜间模式
//   if(Systemtime.wHour  >= 18 || Systemtime.wHour <= 6)
//   {
//     v_temp_threshold = 80;
//   }
// 
//   if(Systemtime.wHour == 5)
//   {
//     v_temp_threshold = 60;
//   }


  if(pRectangle == NULL)
  {
    uint16_t Rect[4] = {0, 0, vi_frame_width, vi_frame_height};
    vp_rect = Rect;
  }
  else
    vp_rect = pRectangle;

  for (v_j=vp_rect[1]; v_j<vp_rect[1]+vp_rect[3]; v_j++)
  {
    for (v_i=vp_rect[0]; v_i<vp_rect[0]+ vp_rect[2]; v_i++)
    {
      vi_pos_start = v_i* 3 + vi_frame_width* 3  * v_j  ;
      vi_pos_y = v_j * vi_frame_width +v_i;

      vp_in_y[vi_pos_y] = (abs( ((uint16_t)vp_rgb_1[vi_pos_start ]+(uint16_t)vp_rgb_1[vi_pos_start +1]+(uint16_t)vp_rgb_1[vi_pos_start +2])  - \
        ((uint16_t)vp_rgb_0[vi_pos_start ]+(uint16_t)vp_rgb_0[vi_pos_start +1]+(uint16_t)vp_rgb_0[vi_pos_start +2]) ) )
        >= v_temp_threshold \
        ? WHITESPOT : BLACKSPOT;
//         ? 255 : BLACKSPOT;

      vp_out_y[vi_pos_y] = (abs( ((uint16_t)vp_rgb_1[vi_pos_start ]+(uint16_t)vp_rgb_1[vi_pos_start +1]+(uint16_t)vp_rgb_1[vi_pos_start +2])  - \
        ((uint16_t)vp_rgb_2[vi_pos_start ]+(uint16_t)vp_rgb_2[vi_pos_start +1]+(uint16_t)vp_rgb_2[vi_pos_start +2]) ) )
        >= v_temp_threshold \
        ? WHITESPOT : BLACKSPOT;
//         ? 255 : BLACKSPOT;
    //  vp_out_y[vi_pos_y] = vp_out_y[vi_pos_y] & vp_in_y[vi_pos_y] & m_black_block_image[vi_pos_y];

    }   
  }

//  memcpy(pFrame_out->m_YuvPlane[0],p_out_y, m_FrameWidth*m_FrameHeight);
  ROK();
}
//////////////////////////////////////////////////////////////////////////
/// 滤波函数，滤掉单个点，pRect为滤波矩形框区域
//////////////////////////////////////////////////////////////////////////
void CParabolaDetect::ImfilterSingle(CFrameContainer* const pFrame_in,uint16_t* const pRectangle) 
{
  ASSERT(pFrame_in);

  uint16_t i = 0 ;
  uint16_t j = 0 ;
  uint16_t v_frame_width = m_frame_width;
  uint16_t v_frame_height = m_frame_height;
  uint8_t* p_temp_image = pFrame_in->m_YuvPlane[0];
  uint16_t* p_rect;
  if(pRectangle == NULL)
  {
    uint16_t Rect[4] = {0,0,v_frame_width,v_frame_height};
    p_rect = Rect;
  }
  else
    p_rect = pRectangle;

  for (i = 1; i < (p_rect[3]-1); i++)//待优化
    for (j = 1; j < (p_rect[2]-1); j++)
      if(p_temp_image[(p_rect[1]+i)*v_frame_width+p_rect[0]+j] == 1)
        if((p_temp_image[(p_rect[1]+i)*v_frame_width+p_rect[0]+j+1] + p_temp_image[(p_rect[1]+i)*v_frame_width+p_rect[0]+j-1] + \
          p_temp_image[(p_rect[1]+i+1)*v_frame_width+p_rect[0]+j] + p_temp_image[(p_rect[1]+i+1)*v_frame_width+p_rect[0]+j+1] + p_temp_image[(p_rect[1]+i+1)*v_frame_width+p_rect[0]+j-1] + \
          p_temp_image[(p_rect[1]+i-1)*v_frame_width+p_rect[0]+j] + p_temp_image[(p_rect[1]+i-1)*v_frame_width+p_rect[0]+j+1] + p_temp_image[(p_rect[1]+i-1)*v_frame_width+p_rect[0]+j-1]) <= ParamSet.imfilter_single_threshold)
          p_temp_image[(p_rect[1]+i)*v_frame_width+p_rect[0]+j] = 0;

  return ;
}

void convert(IplImage* pImage)
{
    for (int i = pImage->width*pImage->height; i >= 0; i--)
    {
        if (pImage->imageData[i] == 1)
        {
            pImage->imageData[i] = (char)255;
        }
    }
}

ErrVal 
CParabolaDetect::ParaDetectTwo( const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out)
{ 
  ASSERT(pFrame_in);
  ASSERT(pFrame_out);

//   if (m_pre_frm_num > 20 )
//   {
// 	  m_smooth_rgb_number = 100 ;
//   }
// 
//   if (m_smooth_rgb_number > 0 )
//   {
// 	  m_smooth_rgb_number-- ;
 	  averageSmoothRgb(pFrame_in, 2 );
//   }
  SHOW_IMAGE("smooth", pFrame_in->getImage());

  if ( 0)//m_circle_frm_first ) //建立背景
  {
    m_circle_frm_first = false ;    
    memcpy (m_continue_image[4], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );
  }

  if (++m_circle_frm_num == 10)
  {
    m_alarm_flg =false ;    
    m_circle_frm_num = 0 ;

    if (m_person_frm_num < 2)
    {
      memcpy (m_continue_image[m_person_frm_num++], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );
      ROK();
    }
    else
    {

      if ( PersonBinarizeSub(pFrame_in, pFrame_out, ParamSet.binarize_sub_threshold+110) )
      {
        if(++m_person_time_delay >= 3 )
        {
          m_person_time_delay = 3 ;
          m_person_detect_flag = true ;
        }
      }
      else
      {
        m_person_time_delay = 0 ;
        m_person_detect_flag = true ;
      }


      uint8_t* temp_add = m_continue_image[0] ;
      m_continue_image[0]= m_continue_image[1] ;
      m_continue_image[1] = temp_add;

      memcpy (m_continue_image[1], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );

    }
  }

  if (1)//m_person_detect_flag)
  {
    if (m_frm_num < 2)
    {
      memcpy (m_para_detect_image[m_frm_num++], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );
      ROK();
    }
    else
    {

      BinarizeSub(pFrame_in, pFrame_out, ParamSet.binarize_sub_threshold);


      ImfilterSingle(pFrame_out);
      RemoveBorder(pFrame_out);
      Imdilate(pFrame_out, ParamSet.imdilate_threshold);

      ForecastObjectDetect(pFrame_in, pFrame_out);  //检测目标

      SHOW_BIN_IMAGE("ForecastObjectDetect", pFrame_in->getWidth(), pFrame_in->getHeight(), 
          (char*)pFrame_out->m_YuvPlane[0]);

      uint8_t* temp_add = m_para_detect_image[0] ;
      m_para_detect_image[0]= m_para_detect_image[1] ;
      m_para_detect_image[1] = temp_add;

      memcpy (m_para_detect_image[1], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );

    }
  }
  else
  {
    m_frm_num = 0 ;
    m_tracked_obj_flag = TRUE;
    m_track_num = 0 ;
    memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));

  }
  ROK() ;
}

/**********************************************
$  ImgMoveObjectDetect :  检测移动目标
$          p_frame_in  :  输入图像指针
$          p_frame_out :  输出图像指针
***********************************************/
ErrVal CParabolaDetect::ImgMoveObjectDetect(const CFrameContainer* const p_frame_in,CFrameContainer* const p_frame_out)
{
	ASSERT (p_frame_in );
	ASSERT (p_frame_out);

	uint16_t i = 0 ;
	uint16_t j = 0 ;
	uint16_t k = 0 ;
	uint16_t rect[4] = {0, 0, 0, 0};
	uint16_t tmp_top = 0 ;
	uint16_t tmp_right = 0 ;

	uint8_t * p_data_in  = p_frame_in->m_BmpBuffer ;

	uint8_t * p_yuv_out  = p_frame_out->m_BmpBuffer ;

	uint16_t img_width  = p_frame_out->getWidth()* 3 ; 
	uint16_t img_height = p_frame_out->getHeight()   ; 
	uint16_t img_size   = p_frame_out->getRgbSize()  ; 

	uint8_t * p_data_out = new uint8_t[img_height*img_width ]; //p_frame_out->m_BmpBuffer ;

	memcpy(p_data_out,p_data_in, img_height*img_width );

	for (i = 0; i < m_track_num; i++ ) 
	{
		if (TrackObject[i].t_track_alarm_flag )
		{

			for(j = 0 ; j < 40 ; j ++ )
			{
				if ( TrackObject[i].t_x_frame_location[j] != 0 )
				{

					p_data_out[img_width * TrackObject[i].t_y_frame_location[j] + TrackObject[i].t_x_frame_location[j]*3   ] = 0 ;
					p_data_out[img_width * TrackObject[i].t_y_frame_location[j] + TrackObject[i].t_x_frame_location[j]*3+1 ] = 0 ;
					p_data_out[img_width * TrackObject[i].t_y_frame_location[j] + TrackObject[i].t_x_frame_location[j]*3+2 ] = 255 ;

					p_data_out[img_width * (TrackObject[i].t_y_frame_location[j]+1) + TrackObject[i].t_x_frame_location[j]*3   ] = 0 ;
					p_data_out[img_width * (TrackObject[i].t_y_frame_location[j]+1)+ TrackObject[i].t_x_frame_location[j]*3+1 ] = 0 ;
					p_data_out[img_width * (TrackObject[i].t_y_frame_location[j]+1) + TrackObject[i].t_x_frame_location[j]*3+2 ] = 255 ;

					p_data_out[img_width * (TrackObject[i].t_y_frame_location[j]+1) + TrackObject[i].t_x_frame_location[j]*3+3   ] = 0 ;
					p_data_out[img_width * (TrackObject[i].t_y_frame_location[j]+1) + TrackObject[i].t_x_frame_location[j]*3+4 ] = 0 ;
					p_data_out[img_width * (TrackObject[i].t_y_frame_location[j]+1) + TrackObject[i].t_x_frame_location[j]*3+5 ] = 255 ;

					p_data_out[img_width * TrackObject[i].t_y_frame_location[j] + TrackObject[i].t_x_frame_location[j]*3+3   ] = 0 ;
					p_data_out[img_width * TrackObject[i].t_y_frame_location[j] + TrackObject[i].t_x_frame_location[j]*3+4 ] = 0 ;
					p_data_out[img_width * TrackObject[i].t_y_frame_location[j] + TrackObject[i].t_x_frame_location[j]*3+5 ] = 255 ;
				}

			}

		}


	}


	for(i = 0 ; i < img_height ; i ++ )
	{
		memcpy(p_yuv_out + i * img_width , p_data_out + (img_height - i-1)*img_width ,img_width);
//		memcpy(p_yuv_out + i * img_width , p_data_out + i*img_width ,img_width);
	}
	delete[] p_data_out ;

	ROK(); 
}
DWORD CParabolaDetect::OnRegRead(const char*  address , const char*  str)
{
  HKEY hKey;
  long lRet ;

  lRet = RegOpenKey(HKEY_LOCAL_MACHINE, address, &hKey);

  if(lRet != ERROR_SUCCESS)
    return 0;
  DWORD dwType;
  DWORD dwValue;
  DWORD dwAge  = 0 ;
  RegQueryValueEx(hKey,str,0,&dwType,(LPBYTE)&dwAge,&dwValue);
  if(lRet != ERROR_SUCCESS)
    return 0;
  return dwAge;
}
void CParabolaDetect::InitParaRead( const int ChannelNum )
{
  const char address1[] = "Software\\parabola\\channel1" ;
  const char address2[] = "Software\\parabola\\channel2" ;
  const char address3[] = "Software\\parabola\\channel3" ;
  const char address4[] = "Software\\parabola\\channel4" ;

  switch (ChannelNum)
  {
  case 0 : 
    InitParaVal( address1 );

    ParamSet.t_line_second_location[2] = 120 ;
    ParamSet.little_region_value = 45 ; 

    ParamSet.little_region_range[0] = 101 ;
    ParamSet.little_region_range[1] = 29 ;
    ParamSet.little_region_range[2] = 154 ;
    ParamSet.little_region_range[3] = 73 ;

    ParamSet.person_range[0][0] = 53 ;
    ParamSet.person_range[0][1] = 0 ; 
    ParamSet.person_range[1][0] = 94 ; 
    ParamSet.person_range[1][1] = 59 ;
    ParamSet.person_range[2][0] = 4 ; 
    ParamSet.person_range[2][1] = 282 ;
    ParamSet.person_range[3][0] = 141 ; 
    ParamSet.person_range[3][1] = 58 ;
    ParamSet.person_range[4][0] = 242 ;
    ParamSet.person_range[4][1] = 271 ;

    PersonAddBlack( );

	  AddStraightLine1(132,60,195,199);
	  AddStraightLine2(148,36,317,125);
//	  AddStraightLine2(164,31,319,74);
    AddStraightLineCurver(144,46,315,159);

    AddBlackBlock(m_black_block_image,1,1,350,21);
    AddBlackBlock(m_black_block_image,1,1,70,287);
    AddBlackLine(m_black_block_image, 187,8,347,54);

    break;
  case 1 : 
    InitParaVal( address2 );

    ParamSet.t_line_second_location[2] = 117 ;
    ParamSet.little_region_value = 120 ;

    ParamSet.person_range[0][0] = 80 ;
    ParamSet.person_range[0][1] = 0 ; 
    ParamSet.person_range[1][0] = 73 ; 
    ParamSet.person_range[1][1] = 80 ;
    ParamSet.person_range[2][0] = 4 ; 
    ParamSet.person_range[2][1] = 282 ;
    ParamSet.person_range[3][0] = 130 ; 
    ParamSet.person_range[3][1] = 82 ;
    ParamSet.person_range[4][0] = 217 ;
    ParamSet.person_range[4][1] = 277 ;

    PersonAddBlack( );

	  AddStraightLine1(82,68,263,228);
	  AddStraightLine2(107,33,309,146);
    AddStraightLineCurver(140,51,331,154);

    AddBlackBlock(m_black_block_image,1,1,66,280);
    AddBlackLine( m_black_block_image, 132,13,337,92);  

    break;
  case 2 : 
    InitParaVal( address3 );

    ParamSet.t_line_second_location[2] = 172 ;
    ParamSet.little_region_value = 193 ;

    ParamSet.person_range[0][0] = 114 ;
    ParamSet.person_range[0][1] = 0 ; 
    ParamSet.person_range[1][0] = 128 ; 
    ParamSet.person_range[1][1] = 114 ;
    ParamSet.person_range[2][0] = 4 ; 
    ParamSet.person_range[2][1] = 282 ;
    ParamSet.person_range[3][0] = 191 ; 
    ParamSet.person_range[3][1] = 111 ;
    ParamSet.person_range[4][0] = 245 ;
    ParamSet.person_range[4][1] = 273 ;

    PersonAddBlack( );

	  AddStraightLine1(179,97,223,253);
  	AddStraightLine2(145,38,345,121);
    AddStraightLineCurver(193,60,323,195);

    AddBlackBlock(m_black_block_image,5,5,125,62);
    AddBlackBlock(m_black_block_image,5,5,155,50);
    AddBlackBlock(m_black_block_image,258,131,350,280);
    AddBlackLine(m_black_block_image, 190,22,241,76);  

    break;
  case 3 : 
    InitParaVal( address4 );

    ParamSet.t_line_second_location[2] = 147 ;
    ParamSet.little_region_value = 196 ;

    AddBlackBlock(m_black_block_image,5,5,70,67);
    AddBlackLine(m_black_block_image, 174,25,335,110);

    break;
  default :
    break;
  }
}
void CParabolaDetect::InitParaVal( const char* address )
{
  DWORD temp_settng = OnRegRead( address , "TwoValue" ) ;
  if( 0 ==  temp_settng )
    ParamSet.binarize_sub_threshold = 40 ;
  else
    ParamSet.binarize_sub_threshold = (int8_t)temp_settng ;

  temp_settng = OnRegRead(  address , "FliterValue" ) ;
  if( 0 ==  temp_settng )
    ParamSet.imfilter_single_threshold = 1 ;
  else
    ParamSet.imfilter_single_threshold = (UINT)temp_settng ;

  temp_settng = OnRegRead(  address , "ImdilateValue" ) ;
  if( 0 ==  temp_settng )
    ParamSet.imdilate_threshold = 2 ;
  else
    ParamSet.imdilate_threshold = (UINT)temp_settng ;

  temp_settng = OnRegRead(  address , "WhiteSpot" ) ;
  if( 0 ==  temp_settng )
    ParamSet.white_spot_num_max = 400 ;
  else
    ParamSet.white_spot_num_max = (UINT)temp_settng ;

  temp_settng = OnRegRead( address ,  "LittleValue" ) ;
  if( 0 ==  temp_settng )
    ParamSet.white_spot_num_min = 4 ;
  else
    ParamSet.white_spot_num_min = (UINT)temp_settng ;

}

/**********************************************
*函数名：InverseMatrix       
*函数介绍：求逆矩阵（高斯—约当法） 
*输入参数：矩阵首地址（二维数组）matrix，阶数row
*输出参数：matrix原矩阵的逆矩阵
*返回值：成功，1；失败，0
*调用函数：swap(double &a,double &b)
*作者：vcrs
*完成时间：2009-10-04
**********************************************/
int CParabolaDetect::InverseMatrix(double *matrix,const int &row)
{
  double *m=new double[row*row];
  double *ptemp,*pt=m;

  int i,j;

  ptemp=matrix;
  for (i=0;i<row;i++)
  {
    for (j=0;j<row;j++)
    {
      *pt=*ptemp;
      ptemp++;
      pt++;
    }
  }

  int k;

  int *is=new int[row],*js=new int[row];

  for (k=0;k<row;k++)
  {
    double max=0;
    //全选主元
    //寻找最大元素
    for (i=k;i<row;i++)
    {
      for (j=k;j<row;j++)
      {
        if (fabs(*(m+i*row+j))>max)
        {
          max=*(m+i*row+j);
          is[k]=i;
          js[k]=j;
        }
      }
    }

    if (0 == max)
    {
      return 0;
    }

    //行交换
    if (is[k]!=k)
    {
      for (i=0;i<row;i++)
      {
        swap(*(m+k*row+i),*(m+is[k]*row+i));
      }
    }

    //列交换
    if (js[k]!=k)
    {
      for (i=0;i<row;i++)
      {
        swap(*(m+i*row+k),*(m+i*row+js[k]));
      }
    }

    *(m+k*row+k)=1/(*(m+k*row+k));

    for (j=0;j<row;j++)
    {
      if (j!=k)
      {
        *(m+k*row+j)*=*((m+k*row+k));
      }
    }

    for (i=0;i<row;i++)
    {
      if (i!=k)
      {
        for (j=0;j<row;j++)
        {
          if(j!=k)
          {
            *(m+i*row+j)-=*(m+i*row+k)**(m+k*row+j);
          }
        }
      }
    }

    for (i=0;i<row;i++)
    {
      if(i!=k)
      {
        *(m+i*row+k)*=-(*(m+k*row+k));
      }
    }
  }

  int r;
  //恢复
  for (r=row-1;r>=0;r--)
  {
    if (js[r]!=r)
    {
      for (j=0;j<row;j++)
      {
        swap(*(m+r*row+j),*(m+js[r]*row+j));
      }
    }
    if (is[r]!=r)
    {
      for (i=0;i<row;i++)
      {
        swap(*(m+i*row+r),*(m+i*row+is[r]));
      }
    }
  }

  ptemp=matrix;
  pt=m;
  for (i=0;i<row;i++)
  {
    for (j=0;j<row;j++)
    {
      *ptemp=*pt;
      ptemp++;
      pt++;
    }
  }
  delete []is;
  delete []js;
  delete []m;

  return 1;
}
void CParabolaDetect::swap(double &a,double &b)
{
  double temp=a;
  a=b;
  b=temp;
}
bool CParabolaDetect::CurveFitting(double *matrix_x,double *matrix_y,double *matrix_A)
{
  double CurveA = 0 ;
  double CurveB = 0 ;
  double CurveC = 0 ;
  if( InverseMatrix(matrix_x, 3) )
  {
    CurveA = matrix_x[0]*matrix_y[0] + matrix_x[1]*matrix_y[1] + matrix_x[2]*matrix_y[2] ;
    CurveB = matrix_x[3]*matrix_y[0] + matrix_x[4]*matrix_y[1] + matrix_x[5]*matrix_y[2] ;
    CurveC = matrix_x[6]*matrix_y[0] + matrix_x[7]*matrix_y[1] + matrix_x[8]*matrix_y[2] ;

    matrix_A[0] = CurveA ;
    matrix_A[1] = CurveB ;
    matrix_A[2] = CurveC ;

    return true ;
  }
  return false ;
}

bool 
CParabolaDetect::PersonBinarizeSub(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out, int16_t Threshold, uint16_t* pRectangle )
{
	uint16_t i = 0 ;
	uint16_t j = 0 ;

	uint8_t* p_rgb_0 = m_continue_image[0] ;
	uint8_t* p_rgb_1 = m_continue_image[1] ;
	uint8_t* p_rgb_bk = m_continue_image[4] ; 
	uint8_t* p_rgb_chg_bk = pFrame_in->m_BmpBuffer;
	uint8_t* p_rgb_3 = m_continue_image[3];

	uint8_t* p_0_bk =  m_show_image[0];
	uint8_t* p_1_0  =  m_show_image[1];
	uint8_t* p_1_bk =  m_show_image[2];
	uint8_t* p_chg_bk = m_show_image[3];


	uint16_t frame_width  = pFrame_out->getWidth();   //图像的宽
	uint16_t frame_height = pFrame_out->getHeight();  //图像高

	int32_t pos_start = 0   ;
	int32_t pos_end   = 0   ;
	uint16_t* p_rect  = NULL;

	int16_t TempThreshold = 150 ;

	long v_person_0_bk = 0 ;
	long v_person_1_0  = 0 ;
	long v_person_1_bk = 0 ;

	bool vb_person_flag = false ;
	bool vb_change_bk_flag = false ;



	SYSTEMTIME Systemtime ;
	GetLocalTime(&Systemtime);

	//-----夜间模式
	if(Systemtime.wHour  >= 18 || Systemtime.wHour <= 6)
	{
		TempThreshold = 80;
	}

	if(Systemtime.wHour == 5)
	{
		TempThreshold = 60;
	}

	if(pRectangle == NULL)
	{
		uint16_t Rect[4] = {0, 0, frame_width, frame_height};
		p_rect = Rect;
	}
	else
		p_rect = pRectangle;

	for (j=p_rect[1]; j<p_rect[1]+p_rect[3]; j++)
	{
		for (i=p_rect[0]; i<p_rect[0]+ p_rect[2]; i++)
		{
			pos_start = i* 3 + frame_width* 3  * j  ;
			pos_end = j * frame_width +i ;

			p_0_bk[pos_end] = (abs( ((uint16_t)p_rgb_0[pos_start ]+(uint16_t)p_rgb_0[pos_start +1]+(uint16_t)p_rgb_0[pos_start +2])  - \
				((uint16_t)p_rgb_bk[pos_start ]+(uint16_t)p_rgb_bk[pos_start +1]+(uint16_t)p_rgb_bk[pos_start +2]) ) )
				>= TempThreshold \
				? WHITESPOT : BLACKSPOT;
			p_0_bk[pos_end] = p_0_bk[pos_end] &  m_show_image[4][pos_end];

		}   
	}
	PersonImfilterSingle(p_0_bk);
	PersonImdilate(p_0_bk, 2);

	//   memcpy(pFrame_in->m_YuvPlane[0],p_0_bk, frame_width*frame_height);
	//   memcpy(pFrame_out->m_YuvPlane[0],p_0_bk, frame_width*frame_height);

	uint16_t rect[4]={0,0,frame_width,frame_height};
	v_person_0_bk  = PersonDetectWhiteSpot( p_0_bk ) ;


	if ( v_person_0_bk < 20 )
	{
		memcpy( p_rgb_bk, p_rgb_0, frame_width*frame_height*3);
		vb_change_bk_flag = true ;
	}
	else
	{
		if ( v_person_0_bk < 8000 )
		{
			AreaLabelObj( p_0_bk, rect ) ;

			if (m_area_frm_num > 0 && m_area_frm_num < 100 )
			{
				AreaTargetCombinate( );

				if ( PersonShapeDetect( ) )
				{
					vb_person_flag =  true ;
				}

			}      
		}
		else
		{
			for (j=p_rect[1]; j<p_rect[1]+p_rect[3]; j++)
			{
				for (i=p_rect[0]; i<p_rect[0]+ p_rect[2]; i++)
				{
					pos_start = i* 3 + frame_width* 3  * j  ;
					pos_end = j * frame_width +i ;

					p_1_bk[pos_end] = (abs( ((uint16_t)p_rgb_1[pos_start ]+(uint16_t)p_rgb_1[pos_start +1]+(uint16_t)p_rgb_1[pos_start +2])  - \
						((uint16_t)p_rgb_bk[pos_start ]+(uint16_t)p_rgb_bk[pos_start +1]+(uint16_t)p_rgb_bk[pos_start +2]) ) )
						>= TempThreshold \
						? WHITESPOT : BLACKSPOT;
					p_1_bk[pos_end] = p_1_bk[pos_end] &  m_show_image[4][pos_end];

				}   
			}

			PersonImfilterSingle(p_1_bk);
			PersonImdilate(p_1_bk, 2);

			v_person_1_bk = PersonDetectWhiteSpot( p_1_bk ) ;

			if ( v_person_1_bk < 20 )
			{
				memcpy( p_rgb_bk, p_rgb_1, frame_width*frame_height*3);
				vb_change_bk_flag = true ;
			}
			else
			{
				if ( v_person_1_bk < 8000 )
				{
					AreaLabelObj( p_1_bk, rect ) ;

					if (m_area_frm_num > 0 && m_area_frm_num < 100 )
					{
						AreaTargetCombinate( ) ;

						if ( PersonShapeDetect(  ) )
						{
							vb_person_flag =  true ;
						}
					} 
				}
				else
				{
					for (j=p_rect[1]; j<p_rect[1]+p_rect[3]; j++)
					{
						for (i=p_rect[0]; i<p_rect[0]+ p_rect[2]; i++)
						{
							pos_start = i* 3 + frame_width* 3  * j  ;
							pos_end = j * frame_width +i ;

							p_1_0[pos_end] = (abs( ((uint16_t)p_rgb_1[pos_start ]+(uint16_t)p_rgb_1[pos_start +1]+(uint16_t)p_rgb_1[pos_start +2])  - \
								((uint16_t)p_rgb_0[pos_start ]+(uint16_t)p_rgb_0[pos_start +1]+(uint16_t)p_rgb_0[pos_start +2]) ) )
								>= TempThreshold \
								? WHITESPOT : BLACKSPOT;
							p_1_0[pos_end] = p_1_0[pos_end] &  m_show_image[4][pos_end];

						}   
					}

					PersonImfilterSingle(p_1_0);
					PersonImdilate(p_1_0, 2);

					v_person_1_0 = PersonDetectWhiteSpot( p_1_0 ) ;

					if ( v_person_1_0  < 30 )
					{
						memcpy( p_rgb_bk, p_rgb_1, frame_width*frame_height*3);
						vb_change_bk_flag = true ;
					}
				}
			}

		}
	}

	m_person_change_bk_time++ ;
	if (m_person_change_bk_time >= 450 )
	{
		for (j=p_rect[1]; j<p_rect[1]+p_rect[3]; j++)
		{
			for (i=p_rect[0]; i<p_rect[0]+ p_rect[2]; i++)
			{
				pos_start = i* 3 + frame_width* 3  * j  ;
				pos_end = j * frame_width +i ;

				p_chg_bk[pos_end] = (abs( ((uint16_t)p_rgb_3[pos_start ]+(uint16_t)p_rgb_3[pos_start +1]+(uint16_t)p_rgb_3[pos_start +2])  - \
					((uint16_t)p_rgb_chg_bk[pos_start ]+(uint16_t)p_rgb_chg_bk[pos_start +1]+(uint16_t)p_rgb_chg_bk[pos_start +2]) ) )
					>= TempThreshold \
					? WHITESPOT : BLACKSPOT;
				p_chg_bk[pos_end] = p_chg_bk[pos_end] &  m_show_image[4][pos_end];
			}
		}

		PersonImfilterSingle(p_chg_bk);

		if (PersonDetectWhiteSpot( p_chg_bk ) < 30 )
		{
			memcpy( p_rgb_bk, p_rgb_chg_bk, frame_width*frame_height*3);
		}

		memcpy( p_rgb_3, p_rgb_chg_bk, frame_width*frame_height*3);
		m_person_change_bk_time = 0 ;
	}

	//if (!vb_person_flag)
	//{ 
	// vb_person_flag = false ;
	//}
	return vb_person_flag ;
}
ErrVal 
CParabolaDetect::AreaLabelObj(uint8_t* pFrame_inout,uint16_t* const pRect)
{
  memset(AreaInfo,0,sizeof(ObjLabelInfoStruct)*m_nTrackObjectMaxNum);  // 清空标定数据结构
  m_area_frm_num = 0 ;

  uint16_t x_sign = 0;
  uint16_t m_temp = 0;
  uint16_t x_temp = 0;
  uint16_t y_temp = 0;
  uint32_t objWhiteNum[m_nTrackObjectMaxNum];
  uint32_t xTotal[m_nTrackObjectMaxNum];
  uint32_t yTotal[m_nTrackObjectMaxNum];
  uint16_t framewidth = m_frame_width ;
  uint16_t frameheight = m_frame_height;
  uint16_t labelAreaWidth = pRect[2];
  uint16_t labelAreaHeight = pRect[3];
  uint16_t i,j,m,n;
  uint8_t* p_data = new uint8_t[(labelAreaHeight+1)*(labelAreaWidth+2)];
  uint16_t* p_temp = new uint16_t[(labelAreaHeight+1)*(labelAreaWidth+2)];
  uint16_t*  pLabelDataOut = new uint16_t[labelAreaHeight*labelAreaWidth];
  memset(objWhiteNum,0,m_nTrackObjectMaxNum*sizeof(uint32_t));
  memset(xTotal,0,m_nTrackObjectMaxNum*sizeof(uint32_t));
  memset(yTotal,0,m_nTrackObjectMaxNum*sizeof(uint32_t));
  memset(p_temp,0,(labelAreaHeight+1)*(labelAreaWidth+2)*sizeof(uint16_t));
  memset(p_data,0,(labelAreaHeight+1)*(labelAreaWidth+2)*sizeof(uint8_t));
  for(j = 1;j <= labelAreaHeight;j++)
  {
    memcpy(&p_data[j*(labelAreaWidth+2)+1],&pFrame_inout[(pRect[1]+j-1)*framewidth+pRect[0]],labelAreaWidth);
  }

  for( j = 1;j <= labelAreaHeight;j++)
    for( i = 1;i <= labelAreaWidth;i++)
    {
      if(p_data[j*(labelAreaWidth+2)+i] == 1)
      {
        if(p_data[(j-1)*(labelAreaWidth+2)+i+1] == 1)     //右上
        {
          p_temp[j*(labelAreaWidth+2)+i]=p_temp[(j-1)*(labelAreaWidth+2)+i+1];
          x_temp = p_temp[(j-1)*(labelAreaWidth+2)+i+1];
          objWhiteNum[x_temp] += 1;
          if(p_data[j*(labelAreaWidth+2)+i-1] == 1)
          {
            if(p_temp[j*(labelAreaWidth+2)+i-1] != x_temp) //左前
            {
              y_temp = p_temp[j*(labelAreaWidth+2)+i-1];
              if(x_temp < y_temp)
              {
                objWhiteNum[y_temp] = 0;
                for( m = 1;m <= labelAreaHeight;m++)
                  for( n = 1;n <= labelAreaWidth;n++)
                  {
                    if(p_temp[m*(labelAreaWidth+2)+n] == y_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = x_temp;
                      objWhiteNum[x_temp] += 1;
                    }
                    if(p_temp[m*(labelAreaWidth+2)+n] > y_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = p_temp[m*(labelAreaWidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = y_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
              else 
              {
                objWhiteNum[x_temp] = 0;
                for( m = 1;m <= labelAreaHeight;m++)
                  for( n = 1;n <= labelAreaWidth;n++)
                  {
                    if(p_temp[m*(labelAreaWidth+2)+n] == x_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = y_temp;
                      objWhiteNum[y_temp] += 1;
                    }
                    if(p_temp[m*(labelAreaWidth+2)+n] > x_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = p_temp[m*(labelAreaWidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = x_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
            }
          }
          else
          {
            if(p_data[(j-1)*(labelAreaWidth+2)+i-1] == 1 && p_temp[(j-1)*(labelAreaWidth+2)+i-1] != x_temp) //左上
            {
              y_temp = p_temp[(j-1)*(labelAreaWidth+2)+i-1];
              if(x_temp < y_temp)
              {
                objWhiteNum[y_temp] = 0;
                for( m = 1;m <= labelAreaHeight;m++)
                  for( n = 1;n <= labelAreaWidth;n++)
                  {
                    if(p_temp[m*(labelAreaWidth+2)+n] == y_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = x_temp;
                      objWhiteNum[x_temp] += 1;
                    }
                    if(p_temp[m*(labelAreaWidth+2)+n] > y_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = p_temp[m*(labelAreaWidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = y_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
              else 
              {
                objWhiteNum[x_temp] = 0;
                for( m = 1;m <= labelAreaHeight;m++)
                  for( n = 1;n <= labelAreaWidth;n++)
                  {
                    if(p_temp[m*(labelAreaWidth+2)+n] == x_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = y_temp;
                      objWhiteNum[y_temp] += 1;
                    }
                    if(p_temp[m*(labelAreaWidth+2)+n] > x_temp)
                    {
                      p_temp[m*(labelAreaWidth+2)+n] = p_temp[m*(labelAreaWidth+2)+n] - 1;
                    }
                  }
                  uint16_t k = x_temp+1;
                  while(objWhiteNum[k] != 0)
                  {
                    objWhiteNum[k-1] = objWhiteNum[k];
                    k++;
                  }
                  objWhiteNum[k-1] = 0;
                  x_sign--;
              }
            }
          }        
        }
        else if(p_data[(j-1)*(labelAreaWidth+2)+i] == 1)    //正上
        {
          p_temp[j*(labelAreaWidth+2)+i] = p_temp[(j-1)*(labelAreaWidth+2)+i];
          x_temp = p_temp[(j-1)*(labelAreaWidth+2)+i];
          objWhiteNum[x_temp] += 1;
        }
        else if(p_data[(j-1)*(labelAreaWidth+2)+i-1] == 1)   //左上
        {
          p_temp[j*(labelAreaWidth+2)+i] = p_temp[(j-1)*(labelAreaWidth+2)+i-1];
          x_temp = p_temp[(j-1)*(labelAreaWidth+2)+i-1];
          objWhiteNum[x_temp] += 1;
        }
        else if(p_data[j*(labelAreaWidth+2)+i-1] == 1)     //左前
        {
          p_temp[j*(labelAreaWidth+2)+i] = p_temp[j*(labelAreaWidth+2)+i-1];
          x_temp = p_temp[j*(labelAreaWidth+2)+i-1];
          objWhiteNum[x_temp] += 1;
        }
        else
        {
          x_sign++;
          if(x_sign >= 50 )
          {
            m_area_frm_num = 0 ;
            if(p_data)
              delete [] p_data;
            if(p_temp)
              delete [] p_temp;
            if(pLabelDataOut)
              delete [] pLabelDataOut;
            //          cout<<"Too many objects!"<<endl;
            RERR();
          }
          m_temp = x_sign;
          p_temp[j*(labelAreaWidth+2)+i] = m_temp;
          objWhiteNum[m_temp] = 1;
        }
      }
    }

    m_area_frm_num = x_sign;

    for(j = 1;j <= labelAreaHeight;j++)
      memcpy(&pLabelDataOut[(j-1)*labelAreaWidth],&p_temp[j*(labelAreaWidth+2)+1],labelAreaWidth*sizeof(uint16_t));

    for(i = 0;i < m_area_frm_num;i++)
    {
      AreaInfo[i].t_obj_label_rect[0] = labelAreaWidth-1;
      AreaInfo[i].t_obj_label_rect[1] = 0;
      AreaInfo[i].t_obj_label_rect[2] = labelAreaHeight-1;
      AreaInfo[i].t_obj_label_rect[3] = 0;
    }

    for(m = 0;m < labelAreaHeight;m++)
      for(n = 0;n < labelAreaWidth;n++)
      {
        if(pLabelDataOut[m*labelAreaWidth+n]>0)
        {
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[0] > n)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[0] = n;
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[1] < n)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[1] = n;
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[2] > m)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[2] = m;
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[3] < m)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].t_obj_label_rect[3] = m;
          xTotal[pLabelDataOut[m*labelAreaWidth+n]-1]+=n;
          yTotal[pLabelDataOut[m*labelAreaWidth+n]-1]+=m;
        }
      }

      for( i = 0;i < m_area_frm_num; i++)
      {
        AreaInfo[i].t_flag = true ;
        uint16_t bottom_x,bottom_y;
        bottom_x = AreaInfo[i].t_obj_label_rect[1];
        bottom_y = AreaInfo[i].t_obj_label_rect[3];
        AreaInfo[i].t_obj_white_spot_num = objWhiteNum[i+1];
        AreaInfo[i].t_obj_label_rect[1] = AreaInfo[i].t_obj_label_rect[2];
        AreaInfo[i].t_obj_label_rect[2] = bottom_x - AreaInfo[i].t_obj_label_rect[0] + 1;
        AreaInfo[i].t_obj_label_rect[3] = bottom_y - AreaInfo[i].t_obj_label_rect[1] + 1;
        AreaInfo[i].t_obj_label_center[0] = xTotal[i]/objWhiteNum[i+1];
        AreaInfo[i].t_obj_label_center[1] = yTotal[i]/objWhiteNum[i+1];

        if (AreaInfo[i].t_obj_white_spot_num < 30 || AreaInfo[i].t_obj_white_spot_num > 4000 )
        {
          AreaInfo[i].t_flag = false ;
        }
      }  

      if(p_data)
        delete [] p_data;
      if(p_temp)
        delete [] p_temp;
      if(pLabelDataOut)
        delete [] pLabelDataOut;
      ROK();
}
void CParabolaDetect::PersonAddBlack()
{
  int i = 0 ;
  int j = 0 ;

  float vf_na = ( ParamSet.person_range[2][1] - ParamSet.person_range[1][1])/( ParamSet.person_range[2][0] - ParamSet.person_range[1][0]);
  float vf_nb = ( ParamSet.person_range[2][1]*ParamSet.person_range[1][0] - ParamSet.person_range[1][1]*ParamSet.person_range[2][0])/(ParamSet.person_range[1][0] - ParamSet.person_range[2][0]);

  float vf_nc = ( ParamSet.person_range[4][1] - ParamSet.person_range[3][1])/( ParamSet.person_range[4][0] - ParamSet.person_range[3][0]);
  float vf_nd = ( ParamSet.person_range[4][1]*ParamSet.person_range[3][0] - ParamSet.person_range[3][1]*ParamSet.person_range[4][0])/(ParamSet.person_range[3][0] - ParamSet.person_range[4][0]);

  int16_t XLeft = 0 ;
  int16_t XRight = 0;

  memset ( m_show_image[4], 0, m_frame_width*m_frame_height);

  for (j = (int)ParamSet.person_range[0][0] ; j < m_frame_height-2 ; j++)
  {
    XLeft = (int16_t)( ( (float)j - vf_nb ) / vf_na ) ;
    XRight = (int16_t)( ( (float)j - vf_nd ) / vf_nc );

    if (XLeft > 0 && XLeft < m_frame_width && XRight > 0 && XRight < m_frame_width && XLeft < XRight )
    {
      for(i = XLeft; i < XRight ; i++)
        m_show_image[4][i + j* m_frame_width] = 1 ;

    }
  }

  return ;

}
void CParabolaDetect::AreaTargetCombinate( )
{
  int i = 0 ;
  int j = 0 ;
  int vi_distance = 0 ;
  int vi_left = 0 ;
  int vi_right = 0 ;
  int vi_bottom = 0 ;
  int vi_top   = 0 ;

  for ( i = 0; i < m_area_frm_num-1; i++ )
  {
    for ( j = i; j< m_area_frm_num; j++)
    {
      if ( AreaInfo[i].t_flag && AreaInfo[j].t_flag)
      {
        //判断哪个边小
        vi_left   = ( AreaInfo[i].t_obj_label_rect[0]< AreaInfo[j].t_obj_label_rect[0])? (AreaInfo[i].t_obj_label_rect[0]): (AreaInfo[j].t_obj_label_rect[0]);
        vi_right  = ( AreaInfo[i].t_obj_label_rect[0]+ AreaInfo[i].t_obj_label_rect[2])> (AreaInfo[j].t_obj_label_rect[0]+ AreaInfo[j].t_obj_label_rect[2] )? (AreaInfo[i].t_obj_label_rect[0]+ AreaInfo[i].t_obj_label_rect[2]): (AreaInfo[j].t_obj_label_rect[0]+AreaInfo[j].t_obj_label_rect[2]);
        vi_bottom = ( AreaInfo[i].t_obj_label_rect[1]< AreaInfo[j].t_obj_label_rect[1])? (AreaInfo[i].t_obj_label_rect[1]): (AreaInfo[j].t_obj_label_rect[1]);
        vi_top    = ( AreaInfo[i].t_obj_label_rect[1]+ AreaInfo[i].t_obj_label_rect[3])> (AreaInfo[j].t_obj_label_rect[1]+ AreaInfo[j].t_obj_label_rect[3] )? (AreaInfo[i].t_obj_label_rect[1]+ AreaInfo[i].t_obj_label_rect[3]): (AreaInfo[j].t_obj_label_rect[1]+AreaInfo[j].t_obj_label_rect[3]);
        //判断是否有重合区域
        if ( (vi_right - vi_left) < (AreaInfo[i].t_obj_label_rect[2] + AreaInfo[i].t_obj_label_rect[2]) )
        {
          if ( AreaInfo[i].t_obj_label_center[1] < AreaInfo[j].t_obj_label_center[1] )
          {
            if ( abs( (AreaInfo[i].t_obj_label_center[1]+AreaInfo[i].t_obj_label_center[3])- AreaInfo[j].t_obj_label_center[1]) <=10  )
            {
              AreaInfo[i].t_obj_label_rect[0] = vi_left ;
              AreaInfo[i].t_obj_label_rect[1] = vi_bottom ;
              AreaInfo[i].t_obj_label_rect[2] = vi_right - vi_left ;
              AreaInfo[i].t_obj_label_rect[3] = vi_top - vi_bottom ;
              AreaInfo[i].t_obj_label_center[0] = AreaInfo[i].t_obj_label_rect[0] + AreaInfo[i].t_obj_label_rect[2]/2 ;
              AreaInfo[i].t_obj_label_center[1] = AreaInfo[i].t_obj_label_rect[1] + AreaInfo[i].t_obj_label_rect[3]/2 ;
              AreaInfo[i].t_obj_white_spot_num += AreaInfo[j].t_obj_white_spot_num ;
              AreaInfo[j].t_flag = false ;
            }            
          }
          else
          {
            if ( abs( (AreaInfo[j].t_obj_label_center[1]+AreaInfo[j].t_obj_label_center[3])- AreaInfo[i].t_obj_label_center[1]) <=10  )
            {
              AreaInfo[i].t_obj_label_rect[0] = vi_left ;
              AreaInfo[i].t_obj_label_rect[1] = vi_bottom ;
              AreaInfo[i].t_obj_label_rect[2] = vi_right - vi_left ;
              AreaInfo[i].t_obj_label_rect[3] = vi_top - vi_bottom ;
              AreaInfo[i].t_obj_label_center[0] = AreaInfo[i].t_obj_label_rect[0] + AreaInfo[i].t_obj_label_rect[2]/2 ;
              AreaInfo[i].t_obj_label_center[1] = AreaInfo[i].t_obj_label_rect[1] + AreaInfo[i].t_obj_label_rect[3]/2 ;
              AreaInfo[i].t_obj_white_spot_num += AreaInfo[j].t_obj_white_spot_num ;
              AreaInfo[j].t_flag = false ;

            }  
          }
        }
      }
    }

  }


}
bool CParabolaDetect::PersonShapeDetect( )
{
	int i = 0 ;
	int j = 0 ;
	bool vb_return_value = false ;
	int  vi_temp_while_number = 0 ;
	int  vi_temp_areaifo_number = 0 ;
	bool vb_temp_change_flag = false ;
	int vi_object_bottom = 0 ;
	int vi_object_wide = 0   ;
	int vi_object_height = 0 ;
	//if (p_AreaInfo->WhiteSpotNum > 100 )
	//{
	//  vb_return_value = true ;
	//}
	//int vi_object_bottom = p_AreaInfo->ObjLabelRect[1] + p_AreaInfo->ObjLabelRect[3] ;
	//int vi_object_wide   = p_AreaInfo->ObjLabelRect[2] ;
	//int vi_object_height = p_AreaInfo->ObjLabelRect[3] ;
	//if (vi_object_bottom < m_FrameHeight-10  && vi_object_height*3 > vi_object_wide && vi_object_height < 8*vi_object_wide )
	//{
	//  vb_return_value =  true ;
	//}
	//else
	//{
	//  j = 4 ;
	//}
	//if (vi_object_bottom >= 275 && p_AreaInfo->WhiteSpotNum > 500 )
	//{
	//  vb_return_value =  true ;
	//}
	for (i = 0; i < m_area_frm_num; i++)
	{
		if ( AreaInfo[i].t_flag  )
		{
			if ( AreaInfo[i].t_obj_white_spot_num >  (uint32_t)vi_temp_while_number )
			{
				vi_temp_areaifo_number =  i ;
				vi_temp_while_number = AreaInfo[i].t_obj_white_spot_num ;
				vb_temp_change_flag = true  ;
			}
		}          
	}
	if (vb_temp_change_flag)
	{
		memcpy(&AreaInfoMax,&AreaInfo[vi_temp_areaifo_number],sizeof(ObjLabelInfoStruct) );
		vi_object_bottom = AreaInfoMax.t_obj_label_rect[1] + AreaInfoMax.t_obj_label_rect[3] ;
		vi_object_wide   = AreaInfoMax.t_obj_label_rect[2] ;
		vi_object_height = AreaInfoMax.t_obj_label_rect[3] ;
		//if (vi_object_bottom < m_FrameHeight-10  && vi_object_height*3 > vi_object_wide && vi_object_height < 8*vi_object_wide && AreaInfoMax.WhiteSpotNum > 50 )
		//{
		//  vb_return_value =  true ;
		//}
		//if (vi_object_bottom >= 275 && AreaInfoMax.WhiteSpotNum > 500 )
		//{
		//  vb_return_value =  true ;
		//}
		if (AreaInfoMax.t_obj_white_spot_num > 80 )
		{
			vb_return_value =  true ;
		}
	}

	return vb_return_value ;
}
long CParabolaDetect::PersonDetectWhiteSpot( uint8_t *p_image_src )
{

  uint16_t i = 0 ; 
  uint16_t j = 0 ;
  int16_t  XLeft = 0 ;
  int16_t  XRight = 0 ;
  long     WhiteNum = 0 ;
  uint16_t framewidth  = m_frame_width  ;    //帧宽
  uint16_t frameheight = m_frame_height ;    //图像高

  float vf_na = ( ParamSet.person_range[2][1] - ParamSet.person_range[1][1])/( ParamSet.person_range[2][0] - ParamSet.person_range[1][0]);
  float vf_nb = ( ParamSet.person_range[2][1]*ParamSet.person_range[1][0] - ParamSet.person_range[1][1]*ParamSet.person_range[2][0])/(ParamSet.person_range[1][0] - ParamSet.person_range[2][0]);

  float vf_nc = ( ParamSet.person_range[4][1] - ParamSet.person_range[3][1])/( ParamSet.person_range[4][0] - ParamSet.person_range[3][0]);
  float vf_nd = ( ParamSet.person_range[4][1]*ParamSet.person_range[3][0] - ParamSet.person_range[3][1]*ParamSet.person_range[4][0])/(ParamSet.person_range[3][0] - ParamSet.person_range[4][0]);

  for (j = (int)ParamSet.person_range[0][0] ; j < m_frame_height-2 ; j++)
  {
    XLeft = (int16_t)( ( (float)j - vf_nb ) / vf_na );
    XRight = (int16_t)( ( (float)j - vf_nd ) / vf_nc ) ;

    if (XLeft > 0 && XLeft < m_frame_width && XRight > 0 && XRight < m_frame_width && XLeft < XRight )
    {
      for(i = XLeft; i < XRight ; i++)
        if (p_image_src[i + j* m_frame_width] == 1 )
        {
          WhiteNum ++ ;
        }
    }

  }

  return WhiteNum ;
}
void CParabolaDetect::PersonImfilterSingle(uint8_t* p_frame, uint16_t* const pRectangle ) 
{

	uint16_t i = 0 ;
	uint16_t j = 0 ;
	uint16_t framewidth = m_frame_width;
	uint16_t frameheight = m_frame_height;
	uint8_t* BW = p_frame;
	uint16_t* pRect;
	uint8_t *temp_frame = new uint8_t[frameheight*framewidth];
	memcpy(temp_frame, BW ,framewidth*frameheight);

	if(pRectangle == NULL)
	{
		uint16_t Rect[4] = {0,0,framewidth,frameheight};
		pRect = Rect;
	}
	else
		pRect = pRectangle;

	for (i = 1; i < (pRect[3]-2); i++)//待优化
		for (j = 1; j < (pRect[2]-2); j++)
			if(BW[(pRect[1]+i)*framewidth+pRect[0]+j] == 1)
				if((BW[(pRect[1]+i)*framewidth+pRect[0]+j+1] + BW[(pRect[1]+i)*framewidth+pRect[0]+j-1] + \
					BW[(pRect[1]+i+1)*framewidth+pRect[0]+j] + BW[(pRect[1]+i+1)*framewidth+pRect[0]+j+1] + BW[(pRect[1]+i+1)*framewidth+pRect[0]+j-1] + \
					BW[(pRect[1]+i-1)*framewidth+pRect[0]+j] + BW[(pRect[1]+i-1)*framewidth+pRect[0]+j+1] + BW[(pRect[1]+i-1)*framewidth+pRect[0]+j-1]) <= 3)
					temp_frame[(pRect[1]+i)*framewidth+pRect[0]+j] = 0;

	memcpy(BW,temp_frame,framewidth*frameheight);

	delete[] temp_frame ;

}
void CParabolaDetect::PersonImdilate( uint8_t *p_frame,uint32_t nDilateTimesPerPixel_in,  uint16_t* pRectangle) 
{ 
	ASSERT( p_frame );

	int m,n,i=0,j=0;
	uint32_t k = 0;
	uint16_t framewidth  = m_frame_width;  //图像的宽
	uint16_t frameheight = m_frame_height; //图像高
	bool flag = FALSE;
	uint8_t* pData = new uint8_t[framewidth*frameheight];
	uint16_t* pRect = NULL;
	if(pRectangle == NULL)
	{
		uint16_t Rect[4] = {0,0,framewidth,frameheight};
		pRect = Rect;
	}
	else
		pRect = pRectangle;

	for( k = 0; k < nDilateTimesPerPixel_in; ++k )
	{
		memcpy(pData,p_frame,framewidth*frameheight*sizeof(pData[0]));
		for( i = pRect[1]+1; i < pRect[3]-1; ++i )
		{
			for( j = pRect[0]+1; j < pRect[2]-1; ++j )
			{
				if( BLACKSPOT == pData[i*framewidth + j])
				{
					for( m = -1; m < 2; ++m )
					{
						for( n = -1; n < 2; ++n )
						{
							if( WHITESPOT == pData[(i+m)*framewidth+j+n] )
							{
								p_frame[i*framewidth + j]=WHITESPOT;
								flag=TRUE;
								break;
							}
						}
						if(flag)
						{
							flag=FALSE;
							break;
						}
					}
				}
			}
		}  
	}

	if(pData)
		delete [] pData;
	return ;
}


/********************************************************
*CurveContrast:曲线拟合函数
*功能:拟合曲线判读是否是需要的曲线形状
*pTrackCurveInfo:跟踪轨迹结构体指针
*
*
*********************************************************/
bool CParabolaNatural::CurveContrast( LabelObjStatus* pTrackCurveInfo)
{
  int    i = 0 ; 
  int    j = 0 ;

  bool   v_temp_flag = false ;
  double v_middle_coordinate_x = 0 ;  
  double v_middle_coordinate_y = 0 ;

  long v_inflexion_x = 0 ; 
  long v_inflexion_y = 0 ;
  long v_line_y = 0 ;


  for (j = pTrackCurveInfo->t_track_frame_num/2 ; j < pTrackCurveInfo->t_track_frame_num  ;j++)
  {
    if (pTrackCurveInfo->t_x_frame_location[j] > 0 )
    {
      v_middle_coordinate_x = pTrackCurveInfo->t_x_frame_location[j];
      v_middle_coordinate_y = pTrackCurveInfo->t_y_frame_location[j];
      v_temp_flag = true ;
      break ;
    }
  }

  if ( ! v_temp_flag)
  {
    return false ;
  }

  //-----3*3矩阵 Y=AX 
  double v_matrix_a[3] = { 0, 0, 0 };
  double v_matrix_y[3] = { pTrackCurveInfo->t_cur_frame_center[1], pTrackCurveInfo->t_origin_frame_center[1], v_middle_coordinate_y };
  double v_matrix_x[9] = { pTrackCurveInfo->t_cur_frame_center[0]*pTrackCurveInfo->t_cur_frame_center[0],       pTrackCurveInfo->t_cur_frame_center[0],    1,
    pTrackCurveInfo->t_origin_frame_center[0]*pTrackCurveInfo->t_origin_frame_center[0], pTrackCurveInfo->t_origin_frame_center[0], 1,
    v_middle_coordinate_x*v_middle_coordinate_x,   v_middle_coordinate_x , 1  };

  if ( CurveFitting(v_matrix_x,v_matrix_y,v_matrix_a) )
  {
    if (v_matrix_a[0] > 0.004)
    {
      v_inflexion_x = (long)( (-v_matrix_a[1])/ (2.0*v_matrix_a[0]) );

      if (v_inflexion_x > ParamSet.t_line_second_location[2]  &&  v_inflexion_x < 450 )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        v_line_y = (long)( ParamSet.t_line_second_location[0]*v_inflexion_x + ParamSet.t_line_second_location[1] );
      }

      if (v_inflexion_y < v_line_y)
      {
        return true ;
      }

      if ( ParamSet.b_little_region_flag  )
      {
        if (pTrackCurveInfo->t_region_little_number >= (pTrackCurveInfo->t_find_obj_number-2))
        {
          return true ;
        }
      }

    }


  }

  return false ;

}
bool CParabolaNatural::TrackAlarmObject(uint16_t i)
{
  bool Temp_alarm = false ;
  int16_t y_height_value  = 0 ;

  if (TrackObject[i].t_obj_distance_flg)
  {
    y_height_value = 25 ;
  }
  else
  {
    y_height_value = 10 ;
  }

  if ( ParamSet.b_little_region_flag  )
  {
    if ( TrackObject[i].t_find_obj_number >= 5  && TrackObject[i].t_find_obj_number <= 8 && TrackObject[i].t_track_frame_num <= 8
      && !TrackObject[i].t_track_alarm_flag
      && TrackObject[i].t_x_continue_num[1] <= 2
      && TrackObject[i].t_lost_frame_num == 0 
      && TrackObject[i].t_migration_diff[0] >= 0	 
      && TrackObject[i].t_region_little_number >= (TrackObject[i].t_find_obj_number-1)
      && (TrackObject[i].t_rise_frame_num[0] >=2 || TrackObject[i].t_rise_frame_num[1] >= 2) 
      && TrackObject[i].t_migration_diff[1] >= 1 	
      && TrackObject[i].t_white_spot_num < 80  
      && (TrackObject[i].t_match_num[0] >= TrackObject[i].t_track_frame_num * 0.6 && TrackObject[i].t_match_num[0] >= 3 )
      && (TrackObject[i].t_match_num[1] >= TrackObject[i].t_track_frame_num * 0.6 && TrackObject[i].t_match_num[1] >= 3 )
      && TrackObject[i].t_track_top_point[1] < ParamSet.little_region_value
      )
    {
      Temp_alarm = TRUE;
    }
  }


  ////////////////////////////临时加的

  if ( TrackObject[i].t_track_frame_num >= 3  
    && TrackObject[i].t_line_range_flag[0]
    && TrackObject[i].t_line_range_flag[1]
    && !TrackObject[i].t_track_alarm_flag
    && TrackObject[i].t_lost_frame_num == 0 
    && TrackObject[i].t_track_frame_num <= 10
    && (TrackObject[i].t_migration_diff[0] >= 1 && abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >= y_height_value-3 ) 
    && (abs(TrackObject[i].t_cur_frame_center[0]-TrackObject[i].t_origin_frame_center[0]) > 5 )
    && (TrackObject[i].t_x_continue_num[0] >= TrackObject[i].t_track_frame_num * 0.5 ||TrackObject[i].t_x_continue_num[0] >= 3 )
    && (TrackObject[i].t_match_num[0] >= TrackObject[i].t_track_frame_num * 0.4 && TrackObject[i].t_match_num[0] >= 2 )
    && (TrackObject[i].t_match_num[1] >= TrackObject[i].t_track_frame_num * 0.3 && TrackObject[i].t_match_num[1] >= 2 )
    && (TrackObject[i].t_find_obj_number >= 5  && TrackObject[i].t_find_obj_number >= TrackObject[i].t_track_frame_num * 0.6)
    && (TrackObject[i].t_rise_frame_num[0] >=3 || TrackObject[i].t_rise_frame_num[1] >= 3)
    )
  {
    Temp_alarm = TRUE;
  }               
  ////////////////////////////临时加的

  if ( TrackObject[i].t_track_frame_num >= 4  
    && TrackObject[i].t_line_range_flag[0]
    && TrackObject[i].t_line_range_flag[1]
    && !TrackObject[i].t_track_alarm_flag
    && TrackObject[i].t_lost_frame_num == 0 
    && TrackObject[i].t_track_frame_num <= 12
    && ((TrackObject[i].t_migration_diff[0] >= 1 && abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >= y_height_value ) || (abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >= y_height_value + 20 )  )
    && (abs(TrackObject[i].t_cur_frame_center[0]-TrackObject[i].t_origin_frame_center[0]) > 5 )
    && (TrackObject[i].t_x_continue_num[0] >= TrackObject[i].t_track_frame_num * 0.5 ||TrackObject[i].t_x_continue_num[0] >= 4 )
    && (TrackObject[i].t_match_num[0] >= TrackObject[i].t_track_frame_num * 0.4 && TrackObject[i].t_match_num[0] >= 3 )
    && (TrackObject[i].t_match_num[1] >= TrackObject[i].t_track_frame_num * 0.3 && TrackObject[i].t_match_num[1] >= 2 )
    && (TrackObject[i].t_find_obj_number >= 6  && TrackObject[i].t_find_obj_number >= TrackObject[i].t_track_frame_num * 0.6)
    && (TrackObject[i].t_rise_frame_num[0] >=3 || TrackObject[i].t_rise_frame_num[1] >= 3)
    )
  {
    Temp_alarm = TRUE;
  }
  if ( TrackObject[i].t_track_frame_num >= 10  && TrackObject[i].t_track_frame_num <= 18 
    && TrackObject[i].t_line_range_flag[0]
    && TrackObject[i].t_line_range_flag[1]
    && !TrackObject[i].t_track_alarm_flag
    && TrackObject[i].t_lost_frame_num == 0 
    && ((TrackObject[i].t_migration_diff[0] >= 1 && abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >= y_height_value ) || (abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].t_cur_frame_center[0]-TrackObject[i].t_origin_frame_center[0]) > 20 )
    && (TrackObject[i].t_x_continue_num[0] >= TrackObject[i].t_track_frame_num * 0.5 ||TrackObject[i].t_x_continue_num[0] >= 6 )
    && (TrackObject[i].t_match_num[0] >= TrackObject[i].t_track_frame_num * 0.4 && TrackObject[i].t_match_num[0] >= 5 )
    && (TrackObject[i].t_match_num[1] >= TrackObject[i].t_track_frame_num * 0.3 && TrackObject[i].t_match_num[1] >= 5 )
    && (TrackObject[i].t_find_obj_number >= 10  && TrackObject[i].t_find_obj_number >= TrackObject[i].t_track_frame_num * 0.7)
    && (TrackObject[i].t_rise_frame_num[0] >=4 || TrackObject[i].t_rise_frame_num[1] >= 4) 
    )
  {
    Temp_alarm = TRUE;
  }

  if ( TrackObject[i].t_track_frame_num >= 14   
    && TrackObject[i].t_line_range_flag[0]
    && TrackObject[i].t_line_range_flag[1]
    && !TrackObject[i].t_track_alarm_flag
    && ((TrackObject[i].t_migration_diff[0] >= 1 && abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >= y_height_value ) || (abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].t_cur_frame_center[0]-TrackObject[i].t_origin_frame_center[0]) > 30 )
    && (TrackObject[i].t_x_continue_num[0] >= TrackObject[i].t_track_frame_num * 0.5 ||TrackObject[i].t_x_continue_num[0] >= 8 )
    && (TrackObject[i].t_match_num[0] >= TrackObject[i].t_track_frame_num * 0.3 )
    && (TrackObject[i].t_match_num[1] >= TrackObject[i].t_track_frame_num * 0.2 )
    && (TrackObject[i].t_find_obj_number >= 12  && TrackObject[i].t_find_obj_number >= TrackObject[i].t_track_frame_num * 0.7)
    && (TrackObject[i].t_rise_frame_num[0] >=4 || TrackObject[i].t_rise_frame_num[1] >= 4) 
    )
  {
    Temp_alarm = TRUE;    
  }

  return Temp_alarm;

}

/********************************************************
*CurveContrast:曲线拟合函数
*功能:拟合曲线判读是否是需要的曲线形状
*pTrackCurveInfo:跟踪轨迹结构体指针
*
*
*********************************************************/
bool CParabolaCurve::CurveContrast( LabelObjStatus* pTrackCurveInfo)
{
  int    i = 0 ; 
  int    j = 0 ;

  bool   v_temp_flag = false ;
  double v_middle_coordinate_x = 0 ;  
  double v_middle_coordinate_y = 0 ;

  long v_inflexion_x = 0 ; 
  long v_inflexion_y = 0 ;
  long v_line_y = 0 ;

  for (j = pTrackCurveInfo->t_track_frame_num/2 ; j < pTrackCurveInfo->t_track_frame_num  ;j++)
  {
    if (pTrackCurveInfo->t_x_frame_location[j] > 0 )
    {
      v_middle_coordinate_x = pTrackCurveInfo->t_x_frame_location[j];
      v_middle_coordinate_y = pTrackCurveInfo->t_y_frame_location[j];
      v_temp_flag = true ;
      break ;
    }
  }

  if ( ! v_temp_flag)
  {
    return false ;
  }

  //-----3*3矩阵 Y=AX 
  double v_matrix_a[3] = { 0, 0, 0 };
  double v_matrix_y[3] = { pTrackCurveInfo->t_cur_frame_center[1], pTrackCurveInfo->t_origin_frame_center[1], v_middle_coordinate_y };
  double v_matrix_x[9] = { pTrackCurveInfo->t_cur_frame_center[0]*pTrackCurveInfo->t_cur_frame_center[0],       pTrackCurveInfo->t_cur_frame_center[0],    1,
    pTrackCurveInfo->t_origin_frame_center[0]*pTrackCurveInfo->t_origin_frame_center[0], pTrackCurveInfo->t_origin_frame_center[0], 1,
    v_middle_coordinate_x*v_middle_coordinate_x,   v_middle_coordinate_x , 1  };

  if ( CurveFitting(v_matrix_x,v_matrix_y,v_matrix_a) )
  {
    if (v_matrix_a[0] > 0.004)
    {
      v_inflexion_x = (long)( (-v_matrix_a[1])/ (2.0*v_matrix_a[0]) );

      if (v_inflexion_x > ParamSet.t_line_second_location[2]  &&  v_inflexion_x < 450 )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        v_line_y = (long)( ParamSet.t_line_second_location[0]*v_inflexion_x + ParamSet.t_line_second_location[1] );

        if (v_inflexion_y < v_line_y)
        {
          return true ;
        }
      }
      if (v_inflexion_x > 0 && v_inflexion_x <= ParamSet.t_line_second_location[2] )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        if (v_inflexion_y < 60 )
        {
          return true ;
        }
      }


      //if (0 == m_channel_num)
      //{
      //  if (v_inflexion_x > ParamSet.t_line_second_location[2]  &&  v_inflexion_x < 450 )
      //  {
      //    v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
      //    v_line_y = (long)( ParamSet.t_line_second_location[0]*v_inflexion_x + ParamSet.t_line_second_location[1] );

      //    if (v_inflexion_y < v_line_y)
      //    {
      //      return true ;
      //    }
      //  }
      //  if (v_inflexion_x > 0 && v_inflexion_x <= ParamSet.CurverA )
      //  {
      //    v_inflexion_y = v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2];
      //    if (v_inflexion_y < 60 )
      //    {
      //      return true ;
      //    }
      //  }
      //}

      //if (1 == m_channel_num)
      //{
      //  if (v_inflexion_x > ParamSet.t_line_second_location[2]  &&  v_inflexion_x < 450 )
      //  {
      //    v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
      //    v_line_y = (long)( ParamSet.t_line_second_location[0]*v_inflexion_x + ParamSet.t_line_second_location[1] );

      //    if (v_inflexion_y < v_line_y)
      //    {
      //      return true ;
      //    }
      //  }
      //  if (v_inflexion_x > 0 && v_inflexion_x <= ParamSet.CurverA )
      //  {
      //    v_inflexion_y = v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2];
      //    if (v_inflexion_y < 70 )
      //    {
      //      return true ;
      //    }
      //  }
      //}

    }


  }

  return false ;

}
bool CParabolaCurve::TrackAlarmObject(uint16_t i)
{
    bool Temp_alarm = false ;
    int16_t y_height_value  = 0 ;

    if (TrackObject[i].t_white_spot_num < 200)
    {
      y_height_value = 15 ;
    }
    else
    {
      y_height_value = 20 ;
    }
    if (TrackObject[i].t_white_spot_num > 300)
    {
      y_height_value = 30 ;
    }
    if (TrackObject[i].t_white_spot_num > 400 )
    {
      y_height_value = 35 ;
    }

    if (TrackObject[i].t_obj_distance_flg)
    {
      y_height_value = 40 ;
    }

    if (TrackObject[i].t_track_top_point[0] > 240 )
    {
      y_height_value = 35 ;
    }

    if ( TrackObject[i].t_find_obj_number >= 6  && TrackObject[i].t_find_obj_number <= 12 && TrackObject[i].t_track_frame_num <= 14
      && !TrackObject[i].t_track_alarm_flag
      && TrackObject[i].t_x_continue_num[1] < 2
      && TrackObject[i].t_lost_frame_num == 0 

     // && (abs((int)TrackObject[i]. - (int)TrackObject[i].WhiteSpotNum)< TrackObject[i].PurWhiteSpotNum * 0.6 )
      )
    {

      if ( TrackObject[i].t_white_spot_num < 200 
        && TrackObject[i].t_track_top_point[0] < 160  
        && ( abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) > 15 && TrackObject[i].t_track_top_point[1] <= 61 && TrackObject[i].t_track_bottom_point[1] > 75)
//        && TrackObject[i].y_change_number >= ( TrackObject[i].t_find_obj_number - 2 )
        && (TrackObject[i].t_match_num[0] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.8  ) )
        && (TrackObject[i].t_match_num[1] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.7  ) )   
        && abs(TrackObject[i].t_origin_frame_center[0] - TrackObject[i].t_cur_frame_center[0]) > 10
        && TrackObject[i].t_migration_diff[0] <= 3	)
      {
        Temp_alarm = TRUE;
      }

      if ( TrackObject[i].t_white_spot_num >= 200 
        && TrackObject[i].t_white_spot_num <= 600
        && abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) > 30 
        &&  TrackObject[i].t_track_top_point[0] > 200 && TrackObject[i].t_track_top_point[1] < 126
        && abs(TrackObject[i].t_origin_frame_center[0] - TrackObject[i].t_cur_frame_center[0]) > 25
        )
      {
        Temp_alarm = TRUE;
      }
      if ( TrackObject[i].t_white_spot_num > 600 
        && abs(TrackObject[i].t_track_bottom_point[1] - TrackObject[i].t_track_top_point[1]) > 50 
        && (TrackObject[i].t_track_bottom_point[0] > 260 || TrackObject[i].t_track_top_point[0] > 260 )//???????
        && abs(TrackObject[i].t_origin_frame_center[0] - TrackObject[i].t_cur_frame_center[0]) > 30
        )
      {
        Temp_alarm = TRUE;
      }

    }

    if ( TrackObject[i].t_track_frame_num >= 6  && TrackObject[i].t_track_frame_num <= 14 
      && TrackObject[i].t_line_range_flag[0]
      && TrackObject[i].t_line_range_flag[1]
      && !TrackObject[i].t_track_alarm_flag
      && TrackObject[i].t_x_continue_num[1] < 2
      && TrackObject[i].t_lost_frame_num == 0 
      && abs(TrackObject[i].t_origin_frame_center[0] - TrackObject[i].t_cur_frame_center[0]) > 12
      && (  (TrackObject[i].t_migration_diff[0] >= 0 && (TrackObject[i].t_track_bottom_point[1]-TrackObject[i].t_track_top_point[1]) >= y_height_value && TrackObject[i].t_track_top_point[1] < 177 ) ||
         (  (TrackObject[i].t_track_bottom_point[1]-TrackObject[i].t_track_top_point[1]) >= 10 && (TrackObject[i].t_rise_frame_num[0] >=4 && TrackObject[i].t_rise_frame_num[1] >= 4 ) && TrackObject[i].t_track_top_point[1] < 83 ) )
      && (TrackObject[i].t_x_continue_num[0] >= TrackObject[i].t_track_frame_num * 0.5 ||TrackObject[i].t_x_continue_num[0] >= 6 )
      && (TrackObject[i].t_match_num[0] >= (int16_t)(TrackObject[i].t_track_frame_num * 0.4) && TrackObject[i].t_match_num[0] >= 5 )
      && (TrackObject[i].t_match_num[1] >= (int16_t)(TrackObject[i].t_track_frame_num * 0.3) && TrackObject[i].t_match_num[1] >= 5 )
      && (TrackObject[i].t_find_obj_number >= 8  && TrackObject[i].t_find_obj_number >= TrackObject[i].t_track_frame_num * 0.7)
      && (TrackObject[i].t_rise_frame_num[0] >=4 || TrackObject[i].t_rise_frame_num[1] >= 4 ) 
      )
    {

      Temp_alarm = TRUE;
    }
    if( TrackObject[i].t_track_frame_num >= 8  && TrackObject[i].t_track_frame_num <= 18 
      && TrackObject[i].t_line_range_flag[0]
      && TrackObject[i].t_line_range_flag[1]
      && !TrackObject[i].t_track_alarm_flag
      && TrackObject[i].t_x_continue_num[1] < 2
      && TrackObject[i].t_lost_frame_num == 0 
      && abs(TrackObject[i].t_origin_frame_center[0] - TrackObject[i].t_cur_frame_center[0]) > 20
      && (  (TrackObject[i].t_migration_diff[0] >= 0 &&(TrackObject[i].t_track_bottom_point[1]-TrackObject[i].t_track_top_point[1]) >= y_height_value  && TrackObject[i].t_track_top_point[1] < 177) ||
         (  (TrackObject[i].t_track_bottom_point[1]-TrackObject[i].t_track_top_point[1]) >= 15 && (TrackObject[i].t_rise_frame_num[0] >=4 && TrackObject[i].t_rise_frame_num[1] >= 4 ) && TrackObject[i].t_track_top_point[1] < 83 ) )
      && (TrackObject[i].t_x_continue_num[0] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.6)  )
      && (TrackObject[i].t_match_num[0] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.7)  )
      && (TrackObject[i].t_match_num[1] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.7)  )
      && (TrackObject[i].t_find_obj_number >= 10  )
      && (TrackObject[i].t_rise_frame_num[0] >=2 || TrackObject[i].t_rise_frame_num[1] >= 2) 
      )
    {
      Temp_alarm = TRUE;
    }
    if( TrackObject[i].t_track_frame_num >= 11  && TrackObject[i].t_track_frame_num <= 18 
      && TrackObject[i].t_line_range_flag[0]
      && TrackObject[i].t_line_range_flag[1]
      && !TrackObject[i].t_track_alarm_flag
      && TrackObject[i].t_x_continue_num[1] < 2
      && TrackObject[i].t_lost_frame_num == 0 
      && abs(TrackObject[i].t_origin_frame_center[0] - TrackObject[i].t_cur_frame_center[0]) > 30
      && (  (TrackObject[i].t_migration_diff[0] >= 0 &&(TrackObject[i].t_track_bottom_point[1]-TrackObject[i].t_track_top_point[1]) >= y_height_value && TrackObject[i].t_track_top_point[1] < 177 ) ||
      ((TrackObject[i].t_track_bottom_point[1]-TrackObject[i].t_track_top_point[1]) >= 20 && (TrackObject[i].t_rise_frame_num[0] >=4 && TrackObject[i].t_rise_frame_num[1] >= 4 ) && TrackObject[i].t_track_top_point[1] < 83 ) )
      && (TrackObject[i].t_x_continue_num[0] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.6)  )
      && (   TrackObject[i].t_match_num[0] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.7)  
          || TrackObject[i].t_match_num[1] >= (int16_t)(TrackObject[i].t_find_obj_number * 0.7)  )
      && (TrackObject[i].t_find_obj_number >= 13  )
      && (TrackObject[i].t_rise_frame_num[0] >=4 || TrackObject[i].t_rise_frame_num[1] >= 4) 
      )
    {
      Temp_alarm = TRUE;
    }

    return Temp_alarm;
}
//bool CParabolaCurve::TrackAlarmObject(uint16_t i)
//{
//  bool Temp_alarm = false ;
//  int16_t y_height_value  = 0 ;
//
//  if ( 0 == m_channel_num )
//  {
//    if (TrackObject[i].t_obj_distance_flg)
//    {
//      y_height_value = 40 ;
//    }
//    else
//    {
//      y_height_value = 25 ;
//    }
//    if (TrackObject[i].t_white_spot_num > 300 )
//      y_height_value = 35 ;
//
//    if ( TrackObject[i].m_nTrackNum >= 4  
//      &&TrackObject[i].Slope >= 2
//      && TrackObject[i].LineFlagLeft
//      && TrackObject[i].LineFlagRight
//      && !TrackObject[i].TrackAlart
//      && TrackObject[i].ChangeNum < 2
//      && TrackObject[i].Xdictance == 1
//      && TrackObject[i].m_nLostFrameNum == 0 
//      && TrackObject[i].m_nTrackNum <= 12
//      && ((TrackObject[i].xDiff >= 1 &&abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= y_height_value + 20 )  )
//      && (abs(TrackObject[i].m_nCurCenter[0]-TrackObject[i].m_nOriginCenter[0]) > 15 )
//      && (TrackObject[i].UpturnNum >= TrackObject[i].m_nTrackNum * 0.5 ||TrackObject[i].UpturnNum >= 4 )
//      && (TrackObject[i].slopeX >= TrackObject[i].m_nTrackNum * 0.4 && TrackObject[i].slopeX >= 3 )
//      && (TrackObject[i].slopeY >= TrackObject[i].m_nTrackNum * 0.3 && TrackObject[i].slopeY >= 2 )
//      && (TrackObject[i].FindNum >= 6  && TrackObject[i].FindNum >= TrackObject[i].m_nTrackNum * 0.6)
//      && (TrackObject[i].m_nUpFrameNum >=4 || TrackObject[i].m_nDownFrameNum >= 4)
//      )
//    {
//      Temp_alarm = TRUE;
//    }
//    if ( TrackObject[i].m_nTrackNum >= 8  && TrackObject[i].m_nTrackNum <= 16 
//      && TrackObject[i].Slope >= 5
//      && TrackObject[i].LineFlagLeft
//      && TrackObject[i].LineFlagRight
//      && !TrackObject[i].TrackAlart
//      && TrackObject[i].ChangeNum < 3
//      && TrackObject[i].Xdictance == 1
//      && TrackObject[i].m_nLostFrameNum == 0 
//      && ((TrackObject[i].xDiff >= 1 &&abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >=  y_height_value + 15))
//      && (abs(TrackObject[i].m_nCurCenter[0]-TrackObject[i].m_nOriginCenter[0]) > 20 )
//      && (TrackObject[i].UpturnNum >= TrackObject[i].m_nTrackNum * 0.5 ||TrackObject[i].UpturnNum >= 6 )
//      && (TrackObject[i].slopeX >= TrackObject[i].m_nTrackNum * 0.4 && TrackObject[i].slopeX >= 5 )
//      && (TrackObject[i].slopeY >= TrackObject[i].m_nTrackNum * 0.3 && TrackObject[i].slopeY >= 5 )
//      && (TrackObject[i].FindNum >= 10  && TrackObject[i].FindNum >= TrackObject[i].m_nTrackNum * 0.7)
//      && (TrackObject[i].m_nUpFrameNum >=5 || TrackObject[i].m_nDownFrameNum >= 5) 
//      )
//    {
//      Temp_alarm = TRUE;
//    }
//
//  }
//
//  if(1 == m_nChannel )
//  {
//    if (TrackObject[i].distance_flg)
//    {
//      y_height_value = 40 ;
//    }
//    else
//    {
//      y_height_value = 25 ;
//    }
//
//    if (TrackObject[i].WhiteSpotNum > 500 )
//      y_height_value = 40 ;
//
//    if ( TrackObject[i].m_nTrackNum >= 6  
//      && TrackObject[i].Slope >= 2
//      && TrackObject[i].LineFlagLeft
//      && TrackObject[i].LineFlagRight
//      && !TrackObject[i].TrackAlart
//      && TrackObject[i].ChangeNum < 2
//      && TrackObject[i].Xdictance == 1
//      && TrackObject[i].m_nLostFrameNum == 0 
//      && TrackObject[i].m_nTrackNum <= 12
//      && ((TrackObject[i].xDiff >= 1 &&abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= y_height_value + 20 )  )
//      && (abs(TrackObject[i].m_nCurCenter[0]-TrackObject[i].m_nOriginCenter[0]) > 10 )
//      && (TrackObject[i].UpturnNum >= TrackObject[i].m_nTrackNum * 0.5 ||TrackObject[i].UpturnNum >= 4 )
//      && (TrackObject[i].slopeX >= TrackObject[i].m_nTrackNum * 0.4 && TrackObject[i].slopeX >= 3 )
//      && (TrackObject[i].slopeY >= TrackObject[i].m_nTrackNum * 0.3 && TrackObject[i].slopeY >= 2 )
//      && (TrackObject[i].FindNum >= 8  && TrackObject[i].FindNum >= TrackObject[i].m_nTrackNum * 0.6)
//      && (TrackObject[i].m_nUpFrameNum >=4 || TrackObject[i].m_nDownFrameNum >= 4)
//      )
//    {
//      Temp_alarm = TRUE;
//    }
//    if ( TrackObject[i].m_nTrackNum >= 10  && TrackObject[i].m_nTrackNum <= 18 
//      && TrackObject[i].Slope >= 5
//      && TrackObject[i].LineFlagLeft
//      && TrackObject[i].LineFlagRight
//      && !TrackObject[i].TrackAlart
//      && TrackObject[i].ChangeNum < 3
//      && TrackObject[i].Xdictance == 1
//      && TrackObject[i].m_nLostFrameNum == 0 
//      && ((TrackObject[i].xDiff >= 1 &&abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= (y_height_value - 10) ) || (abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >=  y_height_value + 15))
//      && (abs(TrackObject[i].m_nCurCenter[0]-TrackObject[i].m_nOriginCenter[0]) > 12 )
//      && (TrackObject[i].UpturnNum >= TrackObject[i].m_nTrackNum * 0.5 ||TrackObject[i].UpturnNum >= 6 )
//      && (TrackObject[i].slopeX >= TrackObject[i].m_nTrackNum * 0.4 && TrackObject[i].slopeX >= 5 )
//      && (TrackObject[i].slopeY >= TrackObject[i].m_nTrackNum * 0.3 && TrackObject[i].slopeY >= 5 )
//      && (TrackObject[i].FindNum >= 10  && TrackObject[i].FindNum >= TrackObject[i].m_nTrackNum * 0.7)
//      && (TrackObject[i].m_nUpFrameNum >=5 || TrackObject[i].m_nDownFrameNum >= 5) 
//      )
//    {
//      Temp_alarm = TRUE;
//    }
//
//    if ( TrackObject[i].m_nTrackNum >= 16   
//      &&TrackObject[i].Slope >= 7
//      && TrackObject[i].LineFlagLeft
//      && TrackObject[i].LineFlagRight
//      && !TrackObject[i].TrackAlart
//      && TrackObject[i].ChangeNum < 3
//      && TrackObject[i].Xdictance == 1
//      && TrackObject[i].m_nLostFrameNum == 0 
//      && ((TrackObject[i].xDiff >= 1 &&abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >= (y_height_value-15) ) || (abs(TrackObject[i].BottomPoint[1]-TrackObject[i].TopPoint[1]) >=  y_height_value + 15))
//      && (abs(TrackObject[i].m_nCurCenter[0]-TrackObject[i].m_nOriginCenter[0]) > 20 )
//      && (TrackObject[i].UpturnNum >= TrackObject[i].m_nTrackNum * 0.5 ||TrackObject[i].UpturnNum >= 8 )
//      && (TrackObject[i].slopeX >= TrackObject[i].m_nTrackNum * 0.3 )
//      && (TrackObject[i].slopeY >= TrackObject[i].m_nTrackNum * 0.2 )
//      && (TrackObject[i].FindNum >= 12  && TrackObject[i].FindNum >= TrackObject[i].m_nTrackNum * 0.7)
//      && (TrackObject[i].m_nUpFrameNum >=5 || TrackObject[i].m_nDownFrameNum >= 5) 
//      )
//    {
//      Temp_alarm = TRUE;
//    }
//  }
//
//  return Temp_alarm;
//}

// void CParabolaDetect::updateFrame(const IplImage *pFrame) 
// {
//     if (pFrame->width != m_frame_width || pFrame->height != m_frame_height)
//     {
//         m_frame_width = pFrame->width;
//         m_frame_height = pFrame->height;
// 
//         delete m_pFrameContainer;
//         delete m_pOutFrameContainer;
//         delete m_pFrame_matlabFunced;
// 
//         m_pFrameContainer = new CFrameContainer(m_frame_width, m_frame_height);
//         m_pOutFrameContainer = new CFrameContainer(m_frame_width, m_frame_height);
//         m_pFrame_matlabFunced = new CFrameContainer(m_frame_width, m_frame_height);
//     }
// 
//     memcpy(m_pFrameContainer->m_BmpBuffer, pFrame->imageData, pFrame->imageSize);
// 
//     this->ParaDetectTwo(m_pFrameContainer, m_pOutFrameContainer);
//     if( m_alarm_flg )
//     {
//         ImgMoveObjectDetect(m_pFrameContainer, m_pFrame_matlabFunced );
// 
//         //报警
//         FOR_EACH(IAlerter*, alert, m_pFrame_matlabFunced->getImage());
//     }
// }


//#endif
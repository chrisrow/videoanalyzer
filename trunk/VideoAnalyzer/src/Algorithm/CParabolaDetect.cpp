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

ParamStruct ParamSet;
ParamDistinguish ParamDsting;

CParabolaDetect::CParabolaDetect(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in )
{
  memset(TrackObject, 0, m_iTrackObjectMaxNum*sizeof(LabelObjStatus));
  memset(ObjLabelInfo, 0, m_iTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(PreLabelInfo, 0, m_iTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(AreaInfo, 0, m_iTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(&AreaInfoMax, 0, sizeof(ObjLabelInfoStruct));

  m_iFrameWidth = nYWidth_in ;
  m_iFrameHeight = nYHeight_in ;

  ParamSet.iXTrackContinueThreshold = 0  ;
  ParamSet.iXTrackOffsetValue = 1        ;
  //ParamSet.bTransLensImage = true       ;

  m_AlarmFlg       = FALSE ;       //报警标志     
  m_bTrackedObjFlag = FALSE ;       //找到预测目标标志
  m_iCurrFrmNum    = 0     ;       //得到的当前帧数
  m_iPreFrmNum     = 0     ;       //前一帧的目标数
  m_iTrackNum        = 0     ;       //
  m_iFirFrmNum         = 0     ;       //
  m_iFrmDifference  = 0     ;
  m_iAreaFrmNum    = 0     ;
  m_iAlarmDelay     = 0     ;
  m_ImfilterSingleThreshold = 0 ;
  m_BinarizeSubThreshold = 0     ;
  m_NightNumber = 0              ;
  m_NightFlag  = 0               ;

  m_iCircleFrmNum  = 0     ;
  m_bCircleFrmFirst= true  ;
  m_iPersonTimeDelay = 0 ;
  m_iPersonChangeBkTime = 0;      //5分钟
  m_bPersonDetectFlag = false ;
  m_iPersonFrmNum = 0  ;

  uint16_t i ;
  for ( i = 0; i < 5; i++)
  {
    m_pContinueImage[i] = new uint8_t[nYWidth_in*nYHeight_in*3];  
    ASSERT( m_pContinueImage[i] );
  }
  for ( i = 0; i < 5; i++)
  {
    m_pShowImage[i] = new uint8_t[nYWidth_in*nYHeight_in]; 
    ASSERT( m_pShowImage[i] );
  }

  for ( i = 0; i < 2; i++)
  { 
    m_pParaDetectImage[i] = new uint8_t[nYWidth_in*nYHeight_in*3]; 
    ASSERT( m_pParaDetectImage[i] );
  }

  m_pBlackBlockImage = new uint8_t[nYWidth_in*nYHeight_in];
  ASSERT( m_pBlackBlockImage );
  memset(m_pBlackBlockImage,1,nYWidth_in*nYHeight_in);

  if (0 != m_iFrameWidth )
  {
    InitParaVal();
  }

}
CParabolaDetect::~CParabolaDetect()
{
  int i = 0 ;
  for ( i = 0; i < 5; i++)
    SAFEDELETE(m_pContinueImage[i]);
  for ( i = 0; i < 5; i++)
    SAFEDELETE(m_pShowImage[i]);
  for ( i = 0; i < 2; i++)
    SAFEDELETE(m_pParaDetectImage[i]);

  SAFEDELETE(m_pBlackBlockImage);

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
  uint16_t v_width  = m_iFrameWidth ;
  uint16_t v_height = m_iFrameHeight;

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
  uint32_t xTotal[m_iTrackObjectMaxNum]; //200
  uint32_t yTotal[m_iTrackObjectMaxNum];
  uint32_t objWhiteNum[m_iTrackObjectMaxNum];
  uint32_t framewidth = m_iFrameWidth;
  uint32_t frameheight = m_iFrameHeight;

  uint8_t * p_whitespot = pFrame_binaried->m_YuvPlane[0];
  uint8_t * p_data = new uint8_t[(frameheight+1)*(framewidth+2)];
  uint16_t* p_temp = new uint16_t[(frameheight+1)*(framewidth+2)];
  uint16_t* pLabelDataOut = new uint16_t[frameheight*framewidth];
  memset(objWhiteNum,0,m_iTrackObjectMaxNum*sizeof(uint32_t));
  memset(xTotal,0,m_iTrackObjectMaxNum*sizeof(uint32_t));
  memset(yTotal,0,m_iTrackObjectMaxNum*sizeof(uint32_t));
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

          if(x_sign >= m_iTrackObjectMaxNum -1)
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


    m_iCurrFrmNum = x_sign;

    for(j = 1;j <= frameheight;j++)
      memcpy(&pLabelDataOut[(j-1)*framewidth],&p_temp[j*(framewidth+2)+1],framewidth*sizeof(uint16_t));

    //赋初值
    for(i = 0;i < m_iCurrFrmNum;i++)
    {
      ObjLabelInfo[i].iObjLabelRect[0] = framewidth-1;
      ObjLabelInfo[i].iObjLabelRect[1] = 0;
      ObjLabelInfo[i].iObjLabelRect[2] = frameheight-1;
      ObjLabelInfo[i].iObjLabelRect[3] = 0;
    }

    //判断位置坐标，赋值
    for (m = 0; m < frameheight; m++)
      for (n = 0; n < framewidth; n++)
      {
        if( pLabelDataOut[m*framewidth+n] >0 && pLabelDataOut[m*framewidth+n]<m_iTrackObjectMaxNum ) 
        {
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[0] > n)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[0] = n;
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[1] < n)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[1] = n;
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[2] > m)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[2] = m;
          if(ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[3] < m)
            ObjLabelInfo[pLabelDataOut[m*framewidth+n]-1].iObjLabelRect[3] = m;
          xTotal[pLabelDataOut[m*framewidth+n]-1]+=n;
          yTotal[pLabelDataOut[m*framewidth+n]-1]+=m;
        }
      }

      uint32_t TempWhiteNum  = 0 ;
      uint16_t TempBeginX = 0 ;
      uint16_t TempBeginY = 0 ;
      uint16_t TempEndX = 0 ;
      uint16_t TempEndY = 0 ;

      for( i = 0;i < m_iCurrFrmNum; i++)
      {
        uint16_t bottom_x,bottom_y;
        bottom_x = ObjLabelInfo[i].iObjLabelRect[1];
        bottom_y = ObjLabelInfo[i].iObjLabelRect[3];
        ObjLabelInfo[i].iObjWhiteSpotNum = objWhiteNum[i+1];
        ObjLabelInfo[i].iObjLabelRect[1] = ObjLabelInfo[i].iObjLabelRect[2];
        ObjLabelInfo[i].iObjLabelRect[2] = bottom_x - ObjLabelInfo[i].iObjLabelRect[0] + 1;
        ObjLabelInfo[i].iObjLabelRect[3] = bottom_y - ObjLabelInfo[i].iObjLabelRect[1] + 1;
        ObjLabelInfo[i].iObjLabelCenter[0] = xTotal[i]/objWhiteNum[i+1];
        ObjLabelInfo[i].iObjLabelCenter[1] = yTotal[i]/objWhiteNum[i+1];

        TempWhiteNum  = 0 ;
        if (ObjLabelInfo[i].iObjWhiteSpotNum > ParamSet.iWhiteSpotNumMin && ObjLabelInfo[i].iObjWhiteSpotNum < ParamSet.iWhiteSpotNumMax )
        {
          ObjLabelInfo[i].t_flag = false ;
          if(ObjLabelInfo[i].iObjWhiteSpotNum < ParamSet.iWhiteSpotNumMax )
          {
            for (m = ObjLabelInfo[i].iObjLabelRect[1]-3;m < ObjLabelInfo[i].iObjLabelRect[1] + ObjLabelInfo[i].iObjLabelRect[3] +4 ;m++)
            {
              for (n = ObjLabelInfo[i].iObjLabelRect[0]-3; n < ObjLabelInfo[i].iObjLabelRect[0] + ObjLabelInfo[i].iObjLabelRect[2] +4 ;n++)
              {
                if (1 == p_whitespot[framewidth*m + n ])
                {
                  TempWhiteNum++;
                }
              }
            }

            if (TempWhiteNum < ObjLabelInfo[i].iObjWhiteSpotNum + 1 )
            {
              ObjLabelInfo[i].t_flag = true ;
            }

            if(ObjLabelInfo[i].iObjWhiteSpotNum > 250 )
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
  int16_t forecast_cent_x = pTrackObjInfo->iForecastFrmCenter[0] ;
  int16_t forecast_cent_y = pTrackObjInfo->iForecastFrmCenter[1] ;
  int32_t fore_white_spot_num = pTrackObjInfo->iWhiteSpotNum ;
  int16_t curr_track_x = pTrackObjInfo->iCurFrameCenter[0];

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
  uint16_t frame_width  = m_iFrameWidth ;  
  uint16_t frame_height = m_iFrameHeight ;

  uint16_t vi_temp_hight = 0 ;
  bool     find_flag = FALSE;

  int16_t detect_x = 10 + pTrackObjInfo->iLostFrameNum*2 ;
  int16_t detect_y = 10 + pTrackObjInfo->iLostFrameNum*2 ;

  if (1 == pTrackObjInfo->iTrackDirection[0])
  {
    for (i = 0; i < m_iCurrFrmNum; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        curr_x_coordinate = ObjLabelInfo[i].iObjLabelCenter[0];          
        curr_y_coordinate = ObjLabelInfo[i].iObjLabelCenter[1]; 
        curr_white_num = ObjLabelInfo[i].iObjWhiteSpotNum ;

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

        if (pTrackObjInfo->iMigrationDiff[1] <= 2)
        {
          vi_temp_hight = 6*pTrackObjInfo->iMigrationDiff[1] ;
        }
        else
        {
          vi_temp_hight = 3*pTrackObjInfo->iMigrationDiff[1] ;
        }

        if( ( abs(curr_x_coordinate - curr_track_x) >= ParamSet.iXTrackOffsetValue ) 
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
    for (i = 0; i < m_iCurrFrmNum; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        curr_x_coordinate = ObjLabelInfo[i].iObjLabelCenter[0];          
        curr_y_coordinate = ObjLabelInfo[i].iObjLabelCenter[1]; 
        curr_white_num = ObjLabelInfo[i].iObjWhiteSpotNum ;

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

        if (pTrackObjInfo->iMigrationDiff[1] <= 2)
        {
          vi_temp_hight = 6*pTrackObjInfo->iMigrationDiff[1] ;
        }
        else
        {
          vi_temp_hight = 3*pTrackObjInfo->iMigrationDiff[1] ;
        }

        if( ( abs(curr_track_x -curr_x_coordinate) >= ParamSet.iXTrackOffsetValue ) 
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
    if( pTrackObjInfo->iTrackFrameNum > ParamSet.iTrackMaxFrameNum )      //--beyond max object number
    {
      pTrackObjInfo->bContinueTrackFlag = false ;
      return m_nTracked;
    }

    pTrackObjInfo->iFindObjNumber++;
    pTrackObjInfo->iTrackFrameNum++ ;
    pTrackObjInfo->iLostFrameNum = 0 ;
    pTrackObjInfo->iPreFrameCenter[0] = pTrackObjInfo->iCurFrameCenter[0];  
    pTrackObjInfo->iPreFrameCenter[1] = pTrackObjInfo->iCurFrameCenter[1];
    pTrackObjInfo->iCurFrameCenter[0] = pLabelObjStatus->iObjLabelCenter[0]; 
    pTrackObjInfo->iCurFrameCenter[1] = pLabelObjStatus->iObjLabelCenter[1];
    pTrackObjInfo->iWhiteSpotNum = (uint32_t)((float)(pLabelObjStatus->iObjWhiteSpotNum + pTrackObjInfo->iTrackFrameNum * pTrackObjInfo->iWhiteSpotNum ) /  (float)(pTrackObjInfo->iTrackFrameNum + 1) +0.5f);  

    pTrackObjInfo->fXOffset = (float)abs((int16_t)(pTrackObjInfo->iOriginFrameCenter[0] - pTrackObjInfo->iCurFrameCenter[0]))/(float)(pTrackObjInfo->iTrackFrameNum+1);

    if ( abs((int16_t)pTrackObjInfo->iMigrationDiff[0] - abs((int16_t)(pTrackObjInfo->iPreFrameCenter[0] - pTrackObjInfo->iCurFrameCenter[0]))  )<4  )
    {
      pTrackObjInfo->iMatchNum[0]++;
    }

    if (  abs((int16_t)pTrackObjInfo->iMigrationDiff[1] - abs((int16_t)(pTrackObjInfo->iPreFrameCenter[1] - pTrackObjInfo->iCurFrameCenter[1]))  )<4  )
    {
      if (pTrackObjInfo->iMigrationDiff[1] != 0 || abs((int16_t)(pTrackObjInfo->iPreFrameCenter[1] - pTrackObjInfo->iCurFrameCenter[1])) != 0 )
      {
        pTrackObjInfo->iMatchNum[1]++;
      }
    }

    if (pTrackObjInfo->iTrackFrameNum < 6 )
    {
      pTrackObjInfo->iMigrationDiff[0] = abs((int16_t)(pTrackObjInfo->iPreFrameCenter[0] - pTrackObjInfo->iCurFrameCenter[0]));
    }
    else
    {
      pTrackObjInfo->iMigrationDiff[0] = (uint16_t)(pTrackObjInfo->fXOffset + 0.3);
    }

    pTrackObjInfo->iMigrationDiff[1] = abs((int16_t)(pLabelObjStatus->iObjLabelCenter[1] - pTrackObjInfo->iPreFrameCenter[1]));


    pTrackObjInfo->iXFrameLocation[pTrackObjInfo->iTrackFrameNum] = pTrackObjInfo->iCurFrameCenter[0];
    pTrackObjInfo->iYFrameLocation[pTrackObjInfo->iTrackFrameNum] = pTrackObjInfo->iCurFrameCenter[1];

    //-----判断最高点
    if(pTrackObjInfo->iCurFrameCenter[1] > pTrackObjInfo->iTrackBottomPoint[1] )
    {
      pTrackObjInfo->iTrackBottomPoint[0] = pTrackObjInfo->iCurFrameCenter[0] ;
      pTrackObjInfo->iTrackBottomPoint[1] = pTrackObjInfo->iCurFrameCenter[1] ;
      pTrackObjInfo->iTrackBottomPoint[2] = pTrackObjInfo->iTrackFrameNum + 1;
    }
    if(pTrackObjInfo->iCurFrameCenter[1] < pTrackObjInfo->iTrackTopPoint[1] )
    {
      pTrackObjInfo->iTrackTopPoint[0] = pTrackObjInfo->iCurFrameCenter[0] ;
      pTrackObjInfo->iTrackTopPoint[1] = pTrackObjInfo->iCurFrameCenter[1] ;
      pTrackObjInfo->iTrackTopPoint[2] = pTrackObjInfo->iTrackFrameNum + 1;
    }

    //-----判断小区域中是否有轨迹
    if ( ParamSet.tRectLittleRegion.bFlag )
    {
      if(pTrackObjInfo->iCurFrameCenter[0] > ParamSet.tRectLittleRegion.BeginPointX && pTrackObjInfo->iCurFrameCenter[0] < ParamSet.tRectLittleRegion.EndPointX
        && pTrackObjInfo->iCurFrameCenter[1] > ParamSet.tRectLittleRegion.BeginPointY && pTrackObjInfo->iCurFrameCenter[1] < ParamSet.tRectLittleRegion.EndPointY)
      {
        pTrackObjInfo->iLittleRegionNum++;
      }
    }

    //-----判断轨迹是否在树的小范围中
    if (ParamSet.tRectTreeLittleRegion.bFlag )
    {
      if(pTrackObjInfo->iCurFrameCenter[0] > ParamSet.tRectTreeLittleRegion.BeginPointX && pTrackObjInfo->iCurFrameCenter[0] < ParamSet.tRectTreeLittleRegion.EndPointX
        && pTrackObjInfo->iCurFrameCenter[1] > ParamSet.tRectTreeLittleRegion.BeginPointY && pTrackObjInfo->iCurFrameCenter[1] < ParamSet.tRectTreeLittleRegion.EndPointY)
      {
        pTrackObjInfo->iTreeLittleRegionNum++;
      }
    }

    //---- 判断目标离屏幕的距离
    if (pTrackObjInfo->iCurFrameCenter[1] >= (m_iFrameHeight/2) )
    {
      TrackObject[m_iTrackNum].bObjDistanceFlg = true ;
    }

    if (1 == pTrackObjInfo->iTrackDirection[0] )
    {
      if (pTrackObjInfo->iCurFrameCenter[0] > pTrackObjInfo->iPreFrameCenter[0] + ParamSet.iXTrackContinueThreshold)
      {
        pTrackObjInfo->iXContinueNum[0]++;
      }
      else
      {
        if (pTrackObjInfo->iXContinueNum[0] < 5 )
        {
          pTrackObjInfo->iXContinueNum[0] = 0 ;
        }
      }
      pTrackObjInfo->iForecastFrmCenter[0] = pTrackObjInfo->iCurFrameCenter[0] + pTrackObjInfo->iMigrationDiff[0] ;
      if (pTrackObjInfo->iForecastFrmCenter[0] > m_iFrameWidth )
      {
        pTrackObjInfo->bContinueTrackFlag = false ;
      }
    }
    else
    {
      if (pTrackObjInfo->iCurFrameCenter[0]+ParamSet.iXTrackContinueThreshold < pTrackObjInfo->iPreFrameCenter[0])
      {
        pTrackObjInfo->iXContinueNum[0]++;
      }
      else
      {
        if (pTrackObjInfo->iXContinueNum[0] < 5 )
        {
          pTrackObjInfo->iXContinueNum[0] = 0 ;
        }
      }

      if (pTrackObjInfo->iCurFrameCenter[0] <= pTrackObjInfo->iMigrationDiff[0])  
        pTrackObjInfo->bContinueTrackFlag = false ;
      else
        pTrackObjInfo->iForecastFrmCenter[0] = pTrackObjInfo->iCurFrameCenter[0] - pTrackObjInfo->iMigrationDiff[0] ;
    }
    //----判断方向改变次数
    if (1 == pTrackObjInfo->iTrackDirection[1] && pTrackObjInfo->iCurFrameCenter[1] > pTrackObjInfo->iPreFrameCenter[1] + 1)
    {
      pTrackObjInfo->iXContinueNum[1]++ ;
    }
    if (0 == pTrackObjInfo->iTrackDirection[1] && pTrackObjInfo->iCurFrameCenter[1] + 1 < pTrackObjInfo->iPreFrameCenter[1])
    {
      pTrackObjInfo->iXContinueNum[1]++ ;
    }
    if (pTrackObjInfo->iXContinueNum[1] >= 4)
    {
      pTrackObjInfo->bContinueTrackFlag = false;
    }

    //-----确定物体竖直运动方向，从下至上 1，从上至下 0
    if (pTrackObjInfo->iCurFrameCenter[1] < pTrackObjInfo->iPreFrameCenter[1])
    {
      pTrackObjInfo->iTrackDirection[1] = 1 ;

      if (pTrackObjInfo->iCurFrameCenter[1]+ ParamSet.iXTrackContinueThreshold < pTrackObjInfo->iPreFrameCenter[1])
        pTrackObjInfo->iRiseFrameNum[0]++;

      if (pTrackObjInfo->iCurFrameCenter[1] <= pTrackObjInfo->iMigrationDiff[1])  
        pTrackObjInfo->bContinueTrackFlag = false ;
      else
        pTrackObjInfo->iForecastFrmCenter[1] = pTrackObjInfo->iCurFrameCenter[1] - (uint16_t)((float)pTrackObjInfo->iMigrationDiff[1] * 0.8f - 0.5); 			
    }
    else
    {

      pTrackObjInfo->iTrackDirection[1] = 0 ;

      if (pTrackObjInfo->iCurFrameCenter[1] -ParamSet.iXTrackContinueThreshold > pTrackObjInfo->iPreFrameCenter[1])
        pTrackObjInfo->iRiseFrameNum[1]++ ;

      pTrackObjInfo->iForecastFrmCenter[1] = pTrackObjInfo->iCurFrameCenter[1] + (uint16_t)((float)pTrackObjInfo->iMigrationDiff[1] * 0.8f + 0.5); 
      if (pTrackObjInfo->iForecastFrmCenter[1] > pFrame_in->getHeight())
      {
        pTrackObjInfo->bContinueTrackFlag = false ;
      }
    }

    //----判断是否在直线内
    if ( !pTrackObjInfo->bLineRangeFlag[0] || !pTrackObjInfo->bLineRangeFlag[1])
    {
      if(pTrackObjInfo->iCurFrameCenter[1] > 3 && pTrackObjInfo->iCurFrameCenter[1] < pFrame_in->getHeight())
      {
        int16_t LineTempA1 = (int16_t)( ( (float)pTrackObjInfo->iCurFrameCenter[1] - m_fLineFirstLocation[1] ) / m_fLineFirstLocation[0] );
        int16_t LineTempA2 = (int16_t)( ( (float)pTrackObjInfo->iCurFrameCenter[1] - m_fLineFirstLocation[3] ) / m_fLineFirstLocation[2] );
        if(LineTempA1 > 5  && LineTempA1 < pFrame_in->getWidth() )
        {
          if(pTrackObjInfo->iCurFrameCenter[0] > LineTempA1  )
            pTrackObjInfo->bLineRangeFlag[0] = true ;
        }
        if(LineTempA2 > 5  && LineTempA2 < pFrame_in->getWidth() )
        {
          if(pTrackObjInfo->iCurFrameCenter[0] < LineTempA2  )
          {
            pTrackObjInfo->bLineRangeFlag[1] = true ;
          }

        }

      }
    }

    //----5帧以上 如果连续数等于0，返回失败
    if (pTrackObjInfo->iTrackFrameNum > 4 && pTrackObjInfo->iXContinueNum[0] == 0 )
    {
      pTrackObjInfo->bContinueTrackFlag = false ;
    }
  }

  else //未找到与预测目标相匹配的对象
  {
    pTrackObjInfo->iAllLostFrameNum++;

    if ( ( pTrackObjInfo->iFindObjNumber > 4 && pTrackObjInfo->iLostFrameNum > 5)||
      ( pTrackObjInfo->iFindObjNumber <= 4 && pTrackObjInfo->iLostFrameNum > 4)||
      ( pTrackObjInfo->iTrackFrameNum > ParamSet.iTrackMaxFrameNum )||
      ( pTrackObjInfo->iTrackFrameNum > 5 && pTrackObjInfo->iXContinueNum[0] == 0 )||
      ( pTrackObjInfo->iFindObjNumber < pTrackObjInfo->iTrackFrameNum * 0.5 && pTrackObjInfo->iAllLostFrameNum > 6 )||
      ( pTrackObjInfo->iFindObjNumber >= pTrackObjInfo->iTrackFrameNum * 0.5 && pTrackObjInfo->iAllLostFrameNum > 10 )
      )
    {
      pTrackObjInfo->bContinueTrackFlag = false ;
      return m_nTracked;
    }

    pTrackObjInfo->iLostFrameNum++;
    pTrackObjInfo->iTrackFrameNum++;        
    pTrackObjInfo->iPreFrameCenter[0] = pTrackObjInfo->iCurFrameCenter[0];  
    pTrackObjInfo->iPreFrameCenter[1] = pTrackObjInfo->iCurFrameCenter[1];
    pTrackObjInfo->iCurFrameCenter[0] = pTrackObjInfo->iForecastFrmCenter[0]; 
    pTrackObjInfo->iCurFrameCenter[1] = pTrackObjInfo->iForecastFrmCenter[1];
    pTrackObjInfo->iMigrationDiff[0] = (uint16_t)pTrackObjInfo->fXOffset ;
    pTrackObjInfo->iMigrationDiff[1] = abs((int16_t)(pTrackObjInfo->iCurFrameCenter[1] - pTrackObjInfo->iPreFrameCenter[1]));

    //----预测新坐标的位置
    if ( 1 == pTrackObjInfo->iTrackDirection[0] )
    {
      pTrackObjInfo->iForecastFrmCenter[0] = pTrackObjInfo->iCurFrameCenter[0] + pTrackObjInfo->iMigrationDiff[0] ;
      if (pTrackObjInfo->iForecastFrmCenter[0] >= m_iFrameWidth )
      {
        pTrackObjInfo->bContinueTrackFlag = false ;
      }
    }
    else
    {
      if (pTrackObjInfo->iCurFrameCenter[0] <= pTrackObjInfo->iMigrationDiff[0] )
      {
        pTrackObjInfo->bContinueTrackFlag = false ;
      }
      else
        pTrackObjInfo->iForecastFrmCenter[0] = pTrackObjInfo->iCurFrameCenter[0] - pTrackObjInfo->iMigrationDiff[0] ; 
    }

    //----y方向上如果没有偏移，设置偏移为1 方向向下
    if ( 0 == pTrackObjInfo->iMigrationDiff[1])   
    {
      pTrackObjInfo->iTrackDirection[1] = 0 ;
      pTrackObjInfo->iMigrationDiff[1] = 1 ;
    }

    //----预测新坐标的位置
    if ( 1 == pTrackObjInfo->iTrackDirection[1])
    {
      if (pTrackObjInfo->iCurFrameCenter[1] <= pTrackObjInfo->iMigrationDiff[1] )
      {
        pTrackObjInfo->bContinueTrackFlag = false ;
      }
      else
        pTrackObjInfo->iForecastFrmCenter[1] = pTrackObjInfo->iCurFrameCenter[1] - (uint16_t)((float)pTrackObjInfo->iMigrationDiff[1] * 0.8f + 0.5);  
    }
    else
    {
      pTrackObjInfo->iForecastFrmCenter[1] = pTrackObjInfo->iCurFrameCenter[1] + (uint16_t)((float)pTrackObjInfo->iMigrationDiff[1] * 0.8f + 0.5); 
      if (pTrackObjInfo->iPreFrameCenter[1] >= m_iFrameHeight )
      {
        pTrackObjInfo->bContinueTrackFlag = false ;
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

  if (m_iPreFrmNum == 0) 
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

  x_curr_cen = pCurrObjInfo->iObjLabelCenter[0];
  y_curr_cen = pCurrObjInfo->iObjLabelCenter[1];
  curr_white_num = pCurrObjInfo->iObjWhiteSpotNum;

  if (0 == m_iTrackNum)
  {
    no_track_flg = TRUE ;
  }

  cbv_find_flg = FALSE ;
  cbv_add_flg  = FALSE ;

  //----找大小距离接近的点作为跟踪目标
  for (i = 0; i < m_iPreFrmNum; i++)                         
  {
    if(PreLabelInfo[i].t_flag)
    {
      last_x = PreLabelInfo[i].iObjLabelCenter[0];          
      last_y = PreLabelInfo[i].iObjLabelCenter[1];  

      previous_WhiteSpotNum = PreLabelInfo[i].iObjWhiteSpotNum;

      if ( ( abs(last_x - x_curr_cen)  <= 20) && ( abs(last_y - y_curr_cen) <= 20 )
        && (abs((int16_t)(last_x - x_curr_cen ))>= ParamSet.iXTrackOffsetValue) 
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

    last_x = PreLabelInfo[v_obj_num].iObjLabelCenter[0];          
    last_y = PreLabelInfo[v_obj_num].iObjLabelCenter[1];

    if (!no_track_flg)
    {
      for (j = 0 ; j < m_iTrackNum ; j++ )
      {
        if (  last_x == TrackObject[j].iPreFrameCenter[0] 
        && last_y == TrackObject[j].iPreFrameCenter[1]
        && x_curr_cen == TrackObject[j].iCurFrameCenter[0]
        && y_curr_cen ==  TrackObject[j].iCurFrameCenter[1])
        {
          cbv_add_flg = FALSE ;
          break ;
        }

      }
    }
  }

  //----建立跟踪信息
  LabelObjStatus * p_find_obj_info = &TrackObject[m_iTrackNum] ;
  if (cbv_add_flg) 
  {
    p_find_obj_info->bTrackAlarmFlag = false ;
    p_find_obj_info->iFindObjNumber = 2 ;
    p_find_obj_info->bContinueTrackFlag = true ;
    p_find_obj_info->iPreFrameCenter[0] = last_x;  
    p_find_obj_info->iPreFrameCenter[1] = last_y;
    p_find_obj_info->iCurFrameCenter[0] = pCurrObjInfo->iObjLabelCenter[0]; 
    p_find_obj_info->iCurFrameCenter[1] = pCurrObjInfo->iObjLabelCenter[1];
    p_find_obj_info->iWhiteSpotNum    = pCurrObjInfo->iObjWhiteSpotNum ;  

    p_find_obj_info->fXOffset = (float)abs((int16_t)(x_curr_cen - last_x));

    p_find_obj_info->iMigrationDiff[0] = abs(x_curr_cen - last_x);
    p_find_obj_info->iMigrationDiff[1] = abs(y_curr_cen - last_y);
    p_find_obj_info->iOriginFrameCenter[0] = last_x;
    p_find_obj_info->iOriginFrameCenter[1] = last_y;

    p_find_obj_info->iXFrameLocation[0] = p_find_obj_info->iCurFrameCenter[0];/////
    p_find_obj_info->iYFrameLocation[0] = p_find_obj_info->iCurFrameCenter[1];/////

    p_find_obj_info->iLostFrameNum = 0 ;     
    p_find_obj_info->iRiseFrameNum[1] = 0 ;
    p_find_obj_info->iRiseFrameNum[0] = 0 ;
    p_find_obj_info->iTrackFrameNum = 0 ;
    p_find_obj_info->iXContinueNum[1] = 0 ;
    p_find_obj_info->iXContinueNum[0] = 0 ;
    p_find_obj_info->iAllLostFrameNum = 0 ;
    p_find_obj_info->iMatchNum[0] = 0 ;
    p_find_obj_info->iMatchNum[1] = 0 ;

    //-----求最高点
    if(p_find_obj_info->iPreFrameCenter[1] > p_find_obj_info->iCurFrameCenter[1])
    {
      p_find_obj_info->iTrackBottomPoint[0] = p_find_obj_info->iPreFrameCenter[0] ;
      p_find_obj_info->iTrackBottomPoint[1] = p_find_obj_info->iPreFrameCenter[1] ;
      p_find_obj_info->iTrackBottomPoint[2] = 1 ;
      p_find_obj_info->iTrackTopPoint[0] = p_find_obj_info->iCurFrameCenter[0] ;
      p_find_obj_info->iTrackTopPoint[1] = p_find_obj_info->iCurFrameCenter[1] ;
      p_find_obj_info->iTrackTopPoint[2] = 0 ;

    }
    else
    {
      p_find_obj_info->iTrackBottomPoint[0] = p_find_obj_info->iCurFrameCenter[0] ;
      p_find_obj_info->iTrackBottomPoint[1] = p_find_obj_info->iCurFrameCenter[1];
      p_find_obj_info->iTrackBottomPoint[2] = 0 ;
      p_find_obj_info->iTrackTopPoint[0] = p_find_obj_info->iPreFrameCenter[0];
      p_find_obj_info->iTrackTopPoint[1] = p_find_obj_info->iPreFrameCenter[1];
      p_find_obj_info->iTrackTopPoint[2] = 1 ;
    }

    //-----判断轨迹是否在小范围中
    if (ParamSet.tRectLittleRegion.bFlag )
    {
      if(p_find_obj_info->iPreFrameCenter[0] > ParamSet.tRectLittleRegion.BeginPointX && p_find_obj_info->iPreFrameCenter[0] < ParamSet.tRectLittleRegion.EndPointX 
        && p_find_obj_info->iPreFrameCenter[1] > ParamSet.tRectLittleRegion.BeginPointY && p_find_obj_info->iPreFrameCenter[1] < ParamSet.tRectLittleRegion.EndPointY)
      {
        p_find_obj_info->iLittleRegionNum++;
      }
      if(p_find_obj_info->iCurFrameCenter[0] > ParamSet.tRectLittleRegion.BeginPointX && p_find_obj_info->iCurFrameCenter[0] < ParamSet.tRectLittleRegion.EndPointX
        && p_find_obj_info->iCurFrameCenter[1] > ParamSet.tRectLittleRegion.BeginPointY && p_find_obj_info->iCurFrameCenter[1] < ParamSet.tRectLittleRegion.EndPointY)
      {
        p_find_obj_info->iLittleRegionNum++;
      }
    }
    //-----判断轨迹是否在树的小范围中
    if (ParamSet.tRectTreeLittleRegion.bFlag )
    {
      if(p_find_obj_info->iPreFrameCenter[0] > ParamSet.tRectTreeLittleRegion.BeginPointX && p_find_obj_info->iPreFrameCenter[0] < ParamSet.tRectTreeLittleRegion.EndPointX
        && p_find_obj_info->iPreFrameCenter[1] > ParamSet.tRectTreeLittleRegion.BeginPointY && p_find_obj_info->iPreFrameCenter[1] < ParamSet.tRectTreeLittleRegion.EndPointY )
      {
        p_find_obj_info->iTreeLittleRegionNum++;
      }
      if(p_find_obj_info->iCurFrameCenter[0] > ParamSet.tRectTreeLittleRegion.BeginPointX && p_find_obj_info->iCurFrameCenter[0] < ParamSet.tRectTreeLittleRegion.EndPointX
        && p_find_obj_info->iCurFrameCenter[1] > ParamSet.tRectTreeLittleRegion.BeginPointY && p_find_obj_info->iCurFrameCenter[1] < ParamSet.tRectTreeLittleRegion.EndPointY)
      {
        p_find_obj_info->iTreeLittleRegionNum++;
      }
    }

    //---- 判断目标离屏幕的距离
    if (p_find_obj_info->iPreFrameCenter[1] >= (pFrame_in->getHeight()/2) || p_find_obj_info->iCurFrameCenter[1] >= (pFrame_in->getHeight()/2) )
    {
      p_find_obj_info->bObjDistanceFlg = true ;
    }
    else
    {
      p_find_obj_info->bObjDistanceFlg = false ;
    }

    //----确定物体水平运动方向，从左至右 1，从右至左 0
    if(p_find_obj_info->iCurFrameCenter[0] > last_x)
    {
      p_find_obj_info->iTrackDirection[0] = 1;
      p_find_obj_info->iForecastFrmCenter[0] = p_find_obj_info->iCurFrameCenter[0] + p_find_obj_info->iMigrationDiff[0] ;
      if (p_find_obj_info->iForecastFrmCenter[0] > pFrame_in->getWidth())
      {
        p_find_obj_info->bContinueTrackFlag = false ;
      }
    }
    else
    {
      p_find_obj_info->iTrackDirection[0] = 0;
      if ( (p_find_obj_info->iCurFrameCenter[0] <= p_find_obj_info->iMigrationDiff[0])  )
        p_find_obj_info->bContinueTrackFlag = false ;
      else
        p_find_obj_info->iForecastFrmCenter[0] = p_find_obj_info->iCurFrameCenter[0] - p_find_obj_info->iMigrationDiff[0] ;
    }


    //----确定物体竖直运动方向，从下至上 1，从上至下 0
    if(p_find_obj_info->iCurFrameCenter[1] <= last_y)
    {
      p_find_obj_info->iTrackDirection[1] = 1 ;
      if ( (p_find_obj_info->iCurFrameCenter[1] <= p_find_obj_info->iMigrationDiff[1])  )
        p_find_obj_info->bContinueTrackFlag = false ;
      else
        p_find_obj_info->iForecastFrmCenter[1] = p_find_obj_info->iCurFrameCenter[1] - (uint16_t)((float)p_find_obj_info->iMigrationDiff[1] * 0.8f + 0.5) ;
    }
    else
    {
      p_find_obj_info->iTrackDirection[1] = 0 ;
      p_find_obj_info->iForecastFrmCenter[1] = p_find_obj_info->iCurFrameCenter[1] + (uint16_t)((float)p_find_obj_info->iMigrationDiff[1] * 0.8f + 0.5) ;
      if (p_find_obj_info->iForecastFrmCenter[1] > pFrame_in->getHeight())
      {
        p_find_obj_info->bContinueTrackFlag = false ;
      }
    }

    //----判断是否在直线两边
    p_find_obj_info->bLineRangeFlag[0] = false ;
    p_find_obj_info->bLineRangeFlag[1] = false ;

    if(p_find_obj_info->iCurFrameCenter[1] > 3 && p_find_obj_info->iCurFrameCenter[1] < pFrame_in->getHeight())
    {
      int16_t LineTempA1 = (int16_t)( ( (float)p_find_obj_info->iCurFrameCenter[1] - m_fLineFirstLocation[1] ) / m_fLineFirstLocation[0] );
      int16_t LineTempA2 = (int16_t)( ( (float)p_find_obj_info->iCurFrameCenter[1] - m_fLineFirstLocation[3] ) / m_fLineFirstLocation[2] );
      if(LineTempA1 > 5 && LineTempA1 > m_iFrameWidth )
      {
        if(p_find_obj_info->iCurFrameCenter[0] > LineTempA1  )
          p_find_obj_info->bLineRangeFlag[0] = true ;
      }
      if(LineTempA2 > 5 && LineTempA2 > m_iFrameHeight )
      {
        if(p_find_obj_info->iCurFrameCenter[0] < LineTempA2  )
          p_find_obj_info->bLineRangeFlag[1] = true ;
      }

    }


    m_iTrackNum ++ ;
    if (m_iTrackNum >= m_iTrackObjectMaxNum )
    {
      m_iTrackNum = (uint16_t)(m_iTrackObjectMaxNum * 0.8) ;
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
  memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_iTrackObjectMaxNum );
  m_iCurrFrmNum = 0 ;
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
  uint32_t const v_frame_width  = m_iFrameWidth;
  uint32_t const v_frame_height = m_iFrameHeight;

  m_AlarmFlg = FALSE;  

  GlobalLabelObject(pFrame_out); //mark objects

  if ( m_iAlarmDelay > 0 )
  {
    m_iAlarmDelay--;
  }

  if(!m_bTrackedObjFlag)
  {
    if (m_iCurrFrmNum == 0)
    {
      RERR();
    }

    //----跟踪的目标各项赋值
    memcpy(PreLabelInfo, ObjLabelInfo,sizeof(ObjLabelInfoStruct) * m_iCurrFrmNum) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_iTrackObjectMaxNum) ;

    m_iPreFrmNum = m_iCurrFrmNum ;
    m_bTrackedObjFlag = TRUE ;
    m_iCurrFrmNum = 0 ;

    ROK();
  }
  else
  {

    //----目标数太多，直接清零
    if (m_iCurrFrmNum > 50 )
    {
      m_bTrackedObjFlag = false ;
      memset(PreLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_iTrackObjectMaxNum) ;
      memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_iTrackObjectMaxNum) ;
      memset(TrackObject,0,m_iTrackObjectMaxNum*sizeof(LabelObjStatus));
      m_iTrackNum = 0 ;
      m_iCurrFrmNum = 0 ;
      ROK();
    }

    //----预测跟踪帧的目标
    for (i = 0 ;i < m_iTrackNum; i++)
    {
      DetectedTrackedObject(pFrame_in, pFrame_out, &TrackObject[i]); 
    }

    //----预测当前帧的目标搜索位置
    for (i = 0 ;i < m_iCurrFrmNum; i++)
    {
      if (ObjLabelInfo[i].t_flag)
      {
        FindDetectedObject(pFrame_out, &ObjLabelInfo[i]); 
      }

    }

    memcpy(PreLabelInfo, ObjLabelInfo,sizeof(ObjLabelInfoStruct) * m_iCurrFrmNum) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_iTrackObjectMaxNum) ;

    m_iPreFrmNum = m_iCurrFrmNum ;
    m_iCurrFrmNum = 0 ;

    if(m_iTrackNum == 0)
    {
      m_bTrackedObjFlag = FALSE;
    }

    //----delete repeated object
    uint16_t m_temp_track_obj_num = 0;
    LabelObjStatus m_pTempObjTrackStatus[m_iTrackObjectMaxNum] ;
    memset(m_pTempObjTrackStatus, 0, m_iTrackObjectMaxNum * sizeof(LabelObjStatus) ) ;

    for (i = 0; i < m_iTrackNum; i++)
    {

      if (TrackObject[i].bContinueTrackFlag && m_iAlarmDelay == 0 )
      {
        if (TrackAlarmObject( i ))
        { 
          if (CurveContrast(&TrackObject[i]) )
          {
            TrackObject[i].bTrackAlarmFlag = true ;
            m_AlarmFlg = TRUE ;
            m_iAlarmDelay = 20 ;
          }  
        }

        memcpy(&m_pTempObjTrackStatus[m_temp_track_obj_num],&TrackObject[i],sizeof(LabelObjStatus));
        m_temp_track_obj_num++;
      }

    }

    if (0 == m_temp_track_obj_num)
    {
      memset(TrackObject,0,m_iTrackObjectMaxNum*sizeof(LabelObjStatus));
      m_bTrackedObjFlag = FALSE;
    }
    else if (m_iTrackNum == m_temp_track_obj_num)
    {
      m_bTrackedObjFlag = TRUE;
    }
    else if( m_temp_track_obj_num > m_iTrackObjectMaxNum * 0.8 )
    {
      m_bTrackedObjFlag = TRUE;
      m_iTrackNum = 0 ;
      memset(TrackObject,0,m_iTrackObjectMaxNum*sizeof(LabelObjStatus));

    }
    else
    {
      memset(TrackObject,0,m_iTrackObjectMaxNum*sizeof(LabelObjStatus));
      memcpy(TrackObject,m_pTempObjTrackStatus,m_iTrackObjectMaxNum*sizeof(LabelObjStatus));

      m_iTrackNum = m_temp_track_obj_num;
      m_bTrackedObjFlag = TRUE;
    }

    ROK();
  }
}

ErrVal
CParabolaDetect::StatisticsObjCurinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                                       const  CFrameContainer* const pFrame_curr_in,
                                       const CFrameContainer* const pFrame_bin_in)

{
  long v_curr_frm_num = objDList_inout->GetCount();
  if (0 == v_curr_frm_num)
  {
    memset( &ObjLabelInfo, 0, m_iTrackObjectMaxNum*sizeof(ObjLabelInfoStruct) );
    ROK();
  }

  uint8_t b_mean = 0, g_mean = 0, r_mean = 0;

  for (uint16_t i = 1 ; i <= v_curr_frm_num; i++)
  {
    ObjLabelInfo[i - 1].iObjLabelCenter[0]   =   objDList_inout->GetAt(i)->m_nOuterRect[0] + (objDList_inout->GetAt(i)->m_nOuterRect[2])/2;
    ObjLabelInfo[i - 1].iObjLabelCenter[1]   =   objDList_inout->GetAt(i)->m_nOuterRect[1] + (objDList_inout->GetAt(i)->m_nOuterRect[3])/2;
    ObjLabelInfo[i - 1].iObjLabelRect[0]     =   objDList_inout->GetAt(i)->m_nOuterRect[0];
    ObjLabelInfo[i - 1].iObjLabelRect[1]     =   objDList_inout->GetAt(i)->m_nOuterRect[1];
    ObjLabelInfo[i - 1].iObjLabelRect[2]     =   objDList_inout->GetAt(i)->m_nOuterRect[2];
    ObjLabelInfo[i - 1].iObjLabelRect[3]     =   objDList_inout->GetAt(i)->m_nOuterRect[3];
    ObjLabelInfo[i - 1].iObjWhiteSpotNum    =   objDList_inout->GetAt(i)->m_nXYDotPlus[2];

  }

  ROK();

}
ErrVal 
CParabolaDetect::AddBlackBlock(uint8_t* pFrame_in, int left, int bottom, int right, int top )
{
  ASSERT(pFrame_in);

  uint16_t j = 0 ; 
  uint16_t const TempWide = right - left ;
  uint8_t * p_data = pFrame_in ;

  if (m_iFrameWidth < right )
    right = m_iFrameWidth;
  if (m_iFrameHeight < top )
    top = m_iFrameHeight;

  for( j = bottom; j < top; j++ )
  {
    memset( p_data+j*m_iFrameWidth + left, 0, TempWide );
  }
  ROK();
}

void
CParabolaDetect::AddStraightLine1(int x1, int y1, int x2, int y2 )
{
	m_fLineFirstLocation[0] = ( (float)y2 - (float)y1)/((float)( x2 - x1) + 0.0001f);
	m_fLineFirstLocation[1] = (float)( y2*x1 - y1*x2)/((float)(x1 - x2) + 0.0001f);
}
void
CParabolaDetect::AddStraightLine2(int x1, int y1, int x2, int y2 )
{
  m_fLineFirstLocation[2] = (float)( y2 - y1)/((float)( x2 - x1) + 0.0001f);
  m_fLineFirstLocation[3] = (float)( y2*x1 - y1*x2)/((float)(x1 - x2) + 0.0001f);
}
void
CParabolaDetect::AddStraightLineCurver(int x1, int y1, int x2, int y2 )
{
  m_fLineSecondLocation[0] = (float)( y2 - y1)/( (float)( x2 - x1) + 0.0001f);
  m_fLineSecondLocation[1] = (float)( y2*x1 - y1*x2)/( (float)(x1 - x2) + 0.0001f);
}
ErrVal 
CParabolaDetect::AddLeftBlackLine(uint8_t* pFrame_in, int x1, int y1, int x2, int y2 )
{
  ASSERT(pFrame_in);

  uint16_t i = 0 ; 
  uint16_t j = 0 ;
  int16_t x = 0 ;
  int16_t y = 0 ;
  uint16_t framewidth  = m_iFrameWidth  ;    //帧宽
  uint16_t frameheight = m_iFrameHeight ;    //图像高

  uint8_t * p_data = pFrame_in ;

  float na = (float)( y2 - y1)/((float)( x2 - x1) + 0.0001f);
  float nb = (float)( y2*x1 - y1*x2)/((float)(x1 - x2) + 0.0001f);

  for (j = 5 ; j < frameheight-5 ; j++)
  {
    x = (int16_t)( ( (float)j - nb ) / na );
    if (x > 5 && x< framewidth-5)
    {
      memset(p_data + framewidth * j , 0 ,  x  );
    }

  }

  ROK();
}
ErrVal 
CParabolaDetect::AddRightBlackLine(uint8_t* pFrame_in, int x1, int y1, int x2, int y2 )
{
  ASSERT(pFrame_in);

  uint16_t i = 0 ; 
  uint16_t j = 0 ;
  int16_t x = 0 ;
  int16_t y = 0 ;
  uint16_t framewidth  = m_iFrameWidth  ;    //帧宽
  uint16_t frameheight = m_iFrameHeight ;    //图像高

  uint8_t * p_data = pFrame_in ;

  float na = (float)( y2 - y1)/((float)( x2 - x1) + 0.0001f);
  float nb = (float)( y2*x1 - y1*x2)/((float)(x1 - x2) + 0.0001f);

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
  int16_t ForecastCenX = pTrackAreaObj->iForecastFrmCenter[0] ;
  int16_t ForecastCenY = pTrackAreaObj->iForecastFrmCenter[1] ;
  int32_t ForeWhiteSpotNum = pTrackAreaObj->iWhiteSpotNum ;
  int16_t CurrTrackX = pTrackAreaObj->iCurFrameCenter[0];


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


  int16_t DetectX = 10 + pTrackAreaObj->iLostFrameNum*2 ;
  int16_t DetectY = 10 + pTrackAreaObj->iLostFrameNum*2 ;

  bool FindFalg = FALSE;

  if (1 == pTrackAreaObj->iTrackDirection[0])
  {

    for (i = 0; i < m_iAreaFrmNum; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        CurrX = AreaInfo[i].iObjLabelCenter[0];          
        CurrY = AreaInfo[i].iObjLabelCenter[1]; 
        curr_white_num = AreaInfo[i].iObjWhiteSpotNum ;


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
    for (i = 0; i < m_iAreaFrmNum; i++)
    {
      if(ObjLabelInfo[i].t_flag)
      {
        CurrX = AreaInfo[i].iObjLabelCenter[0];          
        CurrY = AreaInfo[i].iObjLabelCenter[1]; 
        curr_white_num = AreaInfo[i].iObjWhiteSpotNum ;

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
 // ASSERT( pFrame_decoded_inout );

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
  uint16_t v_frame_width   = m_iFrameWidth ;    //帧宽
  uint16_t v_frame_height  = m_iFrameHeight;    //帧高
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
/*****************************************
$InverseImage :镜像变换
$
$
$
******************************************/
void CParabolaDetect::InverseImage(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out)
{

  ASSERT(pFrame_in);
  ASSERT(pFrame_out);

  long     lWidth = 3* m_iFrameWidth;
  uint8_t* pSrc = pFrame_in->m_BmpBuffer;
  uint8_t* pTempImg = new  uint8_t[lWidth*m_iFrameHeight];  

  for ( int j = 3 ; j < m_iFrameHeight-3 ; j++ )
  {
    for( int i = 3 ; i < lWidth-3 ; i += 3 )
    {
      pTempImg[ lWidth-i + j*lWidth] = pSrc[ i + j*lWidth ];
      pTempImg[ lWidth-i+1 + j*lWidth] = pSrc[ i+1 + j*lWidth ];
      pTempImg[ lWidth-i+2 + j*lWidth] = pSrc[ i+2 + j*lWidth ];
    }
  }

  memcpy(pSrc,pTempImg,m_iFrameHeight*lWidth);
  delete[] pTempImg ;
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

  uint8_t* vp_rgb_0 = m_pParaDetectImage[0];
  uint8_t* vp_rgb_1 = m_pParaDetectImage[1];
  uint8_t* vp_rgb_2 = pFrame_in->m_BmpBuffer;

  uint8_t* vp_in_y = pFrame_in->m_YuvPlane[0];
  uint8_t* vp_out_y = pFrame_out->m_YuvPlane[0];

  uint16_t vi_frame_width  = m_iFrameWidth;   //图像的宽
  uint16_t vi_frame_height = m_iFrameHeight;  //图像的高

  int32_t vi_pos_start = 0   ;
  int32_t vi_pos_y     = 0   ;
  int16_t vi_pos_end   = 0   ;
  uint16_t* vp_rect  = NULL;

  int16_t v_temp_threshold = v_threshold ;

  SYSTEMTIME Systemtime ;
  GetLocalTime(&Systemtime);

  //-----夜间模式
  //if(Systemtime.wHour  >= 18 || Systemtime.wHour <= 6)
  //{
  //  v_temp_threshold = 80;
  //}

  //if(Systemtime.wHour == 5)
  //{
  //  v_temp_threshold = 60;
  //}


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

      vp_out_y[vi_pos_y] = (abs( ((uint16_t)vp_rgb_1[vi_pos_start ]+(uint16_t)vp_rgb_1[vi_pos_start +1]+(uint16_t)vp_rgb_1[vi_pos_start +2])  - \
        ((uint16_t)vp_rgb_2[vi_pos_start ]+(uint16_t)vp_rgb_2[vi_pos_start +1]+(uint16_t)vp_rgb_2[vi_pos_start +2]) ) )
        >= v_temp_threshold \
        ? WHITESPOT : BLACKSPOT;
      vp_out_y[vi_pos_y] = vp_out_y[vi_pos_y] & vp_in_y[vi_pos_y] & m_pBlackBlockImage[vi_pos_y];

    }   
  }

 //   memcpy(pFrame_out->m_YuvPlane[0],vp_out_y, vi_frame_width*vi_frame_height);
  ROK();
}
//////////////////////////////////////////////////////////////////////////
/// 滤波函数，滤掉单个点，pRect为滤波矩形框区域
//////////////////////////////////////////////////////////////////////////
void CParabolaDetect::ImfilterSingle(CFrameContainer* const pFrame_in, int SingleThreshold, uint16_t* const pRectangle) 
{
  ASSERT(pFrame_in);

  uint16_t i = 0 ;
  uint16_t j = 0 ;
  uint16_t v_frame_width = m_iFrameWidth;
  uint16_t v_frame_height = m_iFrameHeight;
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
          p_temp_image[(p_rect[1]+i-1)*v_frame_width+p_rect[0]+j] + p_temp_image[(p_rect[1]+i-1)*v_frame_width+p_rect[0]+j+1] + p_temp_image[(p_rect[1]+i-1)*v_frame_width+p_rect[0]+j-1]) <= SingleThreshold)
          p_temp_image[(p_rect[1]+i)*v_frame_width+p_rect[0]+j] = 0;

  return ;
}
bool CParabolaDetect::NightMedol(unsigned char  *pSrc, LineSet *pRect1, LineSet *pRect2)
{
	int i = 0 ;
	int j = 0 ;

	int iWide = m_iFrameWidth ;
	int iHeight = m_iFrameHeight;
	long lTempVal1 = 0 ;
	long lTempVal2 = 0 ;
	long lTempNum1 = 0 ;
	long lTempNum2 = 0 ;

	for (j=pRect1->BeginPointY; j<pRect1->EndPointY; j++)
	{
		for (i=pRect1->BeginPointX; i<pRect1->EndPointX; i++)
		{
			lTempVal1 += pSrc[i+ j* iWide ];
			lTempNum1++;
		}
	}

	for (j=pRect2->BeginPointY; j<pRect2->EndPointY; j++)
	{
		for (i=pRect2->BeginPointX; i<pRect2->EndPointX; i++)
		{
			lTempVal2 += pSrc[i+ j* iWide ];
			lTempNum2++;
		}
	}

	if (lTempNum1)
	{
		lTempVal1 = lTempVal1/lTempNum1;
	}
	else
	{
		lTempVal1 = 0 ;
	}
	if (lTempNum2)
	{
		lTempVal2 = lTempVal2/lTempNum2;
	}
	else
	{
		lTempVal2 = 0 ;       
	}

	if ( lTempVal1 <  ParamSet.iNightRangeVal || lTempVal2 < ParamSet.iNightRangeVal )
	{
		return true ;
	}
	else
		return false ;
}
ErrVal 
CParabolaDetect::ParaDetectTwo( const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out)
{ 
  ASSERT(pFrame_in);
  ASSERT(pFrame_out);

	if (ParamSet.bTransLensImage)
	{
	  InverseImage(pFrame_in, pFrame_out );
	}
  
	if ( ++m_NightNumber == 150 )//夜间模式判断
	{
	  m_NightNumber = 0 ;

	  if( NightMedol(&pFrame_in->m_YuvBuffer[0], 
					 &ParamSet.tNightRange[0], 
					 &ParamSet.tNightRange[1] )  )
	  {
		  m_NightFlag = true ;
	  }
	  else
	  {
		  m_NightFlag =  false ;
	  }
	}

	if (m_NightFlag)
	{
	  m_BinarizeSubThreshold =  ParamSet.iBinarizeSubThreshold ;
	  m_ImfilterSingleThreshold = ParamSet.iImfilterSingleThreshold + 1 ;
	  averageSmoothRgb(pFrame_in, 2 );
	}
	else
	{
	  m_BinarizeSubThreshold =  ParamSet.iNightSubThreshold ;
	  m_ImfilterSingleThreshold = ParamSet.iImfilterSingleThreshold ;
	}



	if (ParamSet.iPersonFlag )//人检测
	{
		if ( m_bCircleFrmFirst ) //建立背景
		{
			m_bCircleFrmFirst = false ;    
			memcpy (m_pContinueImage[4], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );
		}

		if (++m_iCircleFrmNum == 15)
		{
			m_iCircleFrmNum++ ;
			for ( int j = 0 ; j < m_iFrameHeight*m_iFrameWidth*3 ; j++ )
			{
				m_pContinueImage[4][ j ] = (m_pContinueImage[4][ j ]*9 + pFrame_in->m_BmpBuffer[ j ])/10;
			}
		}

		if (!m_NightFlag)
		{
			if(PersonBinarizeSub(pFrame_in, m_pContinueImage[4], m_pShowImage[3], m_BinarizeSubThreshold) )
			{
                m_bPersonDetectFlag = true ;
			}
			else
			{
				m_bPersonDetectFlag = false ;
			}
		}
		else
		{
            m_bPersonDetectFlag = true ;
		}
		

	}

  if (m_bPersonDetectFlag)
  {
    if (m_iFirFrmNum < 2)
    {
      memcpy (m_pParaDetectImage[m_iFirFrmNum++], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );
      ROK();
    }
    else
    {

      BinarizeSub(pFrame_in, pFrame_out, m_BinarizeSubThreshold);
      ImfilterSingle(pFrame_out, m_ImfilterSingleThreshold);
      RemoveBorder(pFrame_out);
      Imdilate(pFrame_out, ParamSet.iImdilateThreshold);
      ForecastObjectDetect(pFrame_in, pFrame_out);  //检测目标

      uint8_t* temp_add = m_pParaDetectImage[0] ;
      m_pParaDetectImage[0]= m_pParaDetectImage[1] ;
      m_pParaDetectImage[1] = temp_add;

      memcpy (m_pParaDetectImage[1], pFrame_in->m_BmpBuffer, pFrame_in->getRgbSize() );

    }
  }
  else
  {
    m_iFirFrmNum = 0 ;
    m_bTrackedObjFlag = TRUE;
    m_iTrackNum = 0 ;
    memset(TrackObject,0,m_iTrackObjectMaxNum*sizeof(LabelObjStatus));

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

  for (i = 0; i < m_iTrackNum; i++ ) 
  {
    if (TrackObject[i].bTrackAlarmFlag )
    {

      for(j = 0 ; j < 40 ; j ++ )
      {
        if ( TrackObject[i].iXFrameLocation[j] != 0 )
        {

          p_data_out[img_width * TrackObject[i].iYFrameLocation[j] + TrackObject[i].iXFrameLocation[j]*3   ] = 0 ;
          p_data_out[img_width * TrackObject[i].iYFrameLocation[j] + TrackObject[i].iXFrameLocation[j]*3+1 ] = 0 ;
          p_data_out[img_width * TrackObject[i].iYFrameLocation[j] + TrackObject[i].iXFrameLocation[j]*3+2 ] = 255 ;

          p_data_out[img_width * (TrackObject[i].iYFrameLocation[j]+1) + TrackObject[i].iXFrameLocation[j]*3   ] = 0 ;
          p_data_out[img_width * (TrackObject[i].iYFrameLocation[j]+1)+ TrackObject[i].iXFrameLocation[j]*3+1 ] = 0 ;
          p_data_out[img_width * (TrackObject[i].iYFrameLocation[j]+1) + TrackObject[i].iXFrameLocation[j]*3+2 ] = 255 ;

          p_data_out[img_width * (TrackObject[i].iYFrameLocation[j]+1) + TrackObject[i].iXFrameLocation[j]*3+3   ] = 0 ;
          p_data_out[img_width * (TrackObject[i].iYFrameLocation[j]+1) + TrackObject[i].iXFrameLocation[j]*3+4 ] = 0 ;
          p_data_out[img_width * (TrackObject[i].iYFrameLocation[j]+1) + TrackObject[i].iXFrameLocation[j]*3+5 ] = 255 ;

          p_data_out[img_width * TrackObject[i].iYFrameLocation[j] + TrackObject[i].iXFrameLocation[j]*3+3   ] = 0 ;
          p_data_out[img_width * TrackObject[i].iYFrameLocation[j] + TrackObject[i].iXFrameLocation[j]*3+4 ] = 0 ;
          p_data_out[img_width * TrackObject[i].iYFrameLocation[j] + TrackObject[i].iXFrameLocation[j]*3+5 ] = 255 ;
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
void CParabolaDetect::InitParaVal(  )
{
	int i = 0 ;

    //ParamSet.iPersonRange[0][0] = 53 ;
    //ParamSet.iPersonRange[0][1] = 0 ; 
    //ParamSet.iPersonRange[1][0] = 94 ; 
    //ParamSet.iPersonRange[1][1] = 59 ;
    //ParamSet.iPersonRange[2][0] = 4 ; 
    //ParamSet.iPersonRange[2][1] = 282 ;
    //ParamSet.iPersonRange[3][0] = 141 ; 
    //ParamSet.iPersonRange[3][1] = 58 ;
    //ParamSet.iPersonRange[4][0] = 242 ;
    //ParamSet.iPersonRange[4][1] = 271 ;

    PersonAddBlack( );

    AddStraightLine1( ParamSet.tLineStraightFirst.BeginPointX, 
		              ParamSet.tLineStraightFirst.BeginPointY,
					  ParamSet.tLineStraightFirst.EndPointX,
					  ParamSet.tLineStraightFirst.EndPointY);

    AddStraightLine2( ParamSet.tLineStraightSecond.BeginPointX, 
					  ParamSet.tLineStraightSecond.BeginPointY,
				      ParamSet.tLineStraightSecond.EndPointX,
					  ParamSet.tLineStraightSecond.EndPointY);

	AddStraightLineCurver( ParamSet.tLineCurverRange.BeginPointX, 
		                   ParamSet.tLineCurverRange.BeginPointY,
		                   ParamSet.tLineCurverRange.EndPointX,
		                   ParamSet.tLineCurverRange.EndPointY);

	for (i = 0; i < 5; i++)
	{
		if ( ParamSet.tRectBlackBlock[i].bFlag )
		{
			AddBlackBlock(  m_pBlackBlockImage,
				ParamSet.tRectBlackBlock[i].BeginPointX, 
				ParamSet.tRectBlackBlock[i].BeginPointY,
				ParamSet.tRectBlackBlock[i].EndPointX,
				ParamSet.tRectBlackBlock[i].EndPointY );
		}
		
	}

	if ( ParamSet.tLineBlackLeft.bFlag )
	{
		AddLeftBlackLine(  m_pBlackBlockImage,
			ParamSet.tLineBlackLeft.BeginPointX, 
			ParamSet.tLineBlackLeft.BeginPointY,
			ParamSet.tLineBlackLeft.EndPointX,
			ParamSet.tLineBlackLeft.EndPointY );
	}

	if ( ParamSet.tLineBlackRight.bFlag )
	{
		AddRightBlackLine(  m_pBlackBlockImage,
			ParamSet.tLineBlackRight.BeginPointX, 
			ParamSet.tLineBlackRight.BeginPointY,
			ParamSet.tLineBlackRight.EndPointX,
			ParamSet.tLineBlackRight.EndPointY );
	}
	
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
CParabolaDetect::PersonBinarizeSub(const CFrameContainer* const pFrame_in,  unsigned char * pBkImage, unsigned char* pSavImage ,int Threshold)
{
  uint16_t i = 0 ;
  uint16_t j = 0 ;
  bool vPersonFlag = false ;
  unsigned char * pRgbNewPrame = pFrame_in->m_BmpBuffer;

  int vFrameWidth = m_iFrameWidth ;
  int vFrameHeight = m_iFrameHeight ;
 // uint16_t iRect[4]={0,0,vFrameWidth,vFrameHeight};

  long lPostStart = 0   ;
  long lPostEnd   = 0   ;
  long lWhiteSpotNum = 0 ;

  for ( j = 0; j < vFrameHeight; j++)
  {
	  for (i = 0 ; i < vFrameWidth; i++)
	  {
		  lPostStart = i* 3 + vFrameWidth* 3  * j  ;
		  lPostEnd = j * vFrameWidth +i ;

		  pSavImage[lPostEnd] = (abs( ((uint16_t)pBkImage[lPostStart ]+(uint16_t)pBkImage[lPostStart +1]+(uint16_t)pBkImage[lPostStart +2])  - \
			  ((uint16_t)pRgbNewPrame[lPostStart ]+(uint16_t)pRgbNewPrame[lPostStart +1]+(uint16_t)pRgbNewPrame[lPostStart +2]) ) )
			  >= Threshold \
			  ? WHITESPOT : BLACKSPOT;
		  pSavImage[lPostEnd] = pSavImage[lPostEnd] &  m_pShowImage[4][lPostEnd];

	  }   
  }

  PersonImfilterSingle(pSavImage);
//  PersonImdilate(pSavImage, 2);

  //   memcpy(pFrame_in->m_YuvPlane[0],p_0_bk, frame_width*frame_height);
  //   memcpy(pFrame_out->m_YuvPlane[0],p_0_bk, frame_width*frame_height);
 
  lWhiteSpotNum  = PersonDetectWhiteSpot( pSavImage ) ;
  if (lWhiteSpotNum > ParamSet.iPersonWhitePotNum)
  {
	  vPersonFlag = true ;
  }
  return vPersonFlag ;
}
ErrVal 
CParabolaDetect::AreaLabelObj(uint8_t* pFrame_inout,uint16_t* const pRect)
{
  memset(AreaInfo,0,sizeof(ObjLabelInfoStruct)*m_iTrackObjectMaxNum);  // 清空标定数据结构
  m_iAreaFrmNum = 0 ;

  uint16_t x_sign = 0;
  uint16_t m_temp = 0;
  uint16_t x_temp = 0;
  uint16_t y_temp = 0;
  uint32_t objWhiteNum[m_iTrackObjectMaxNum];
  uint32_t xTotal[m_iTrackObjectMaxNum];
  uint32_t yTotal[m_iTrackObjectMaxNum];
  uint16_t framewidth = m_iFrameWidth ;
  uint16_t frameheight = m_iFrameHeight;
  uint16_t labelAreaWidth = pRect[2];
  uint16_t labelAreaHeight = pRect[3];
  uint16_t i,j,m,n;
  uint8_t* p_data = new uint8_t[(labelAreaHeight+1)*(labelAreaWidth+2)];
  uint16_t* p_temp = new uint16_t[(labelAreaHeight+1)*(labelAreaWidth+2)];
  uint16_t*  pLabelDataOut = new uint16_t[labelAreaHeight*labelAreaWidth];
  memset(objWhiteNum,0,m_iTrackObjectMaxNum*sizeof(uint32_t));
  memset(xTotal,0,m_iTrackObjectMaxNum*sizeof(uint32_t));
  memset(yTotal,0,m_iTrackObjectMaxNum*sizeof(uint32_t));
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
            m_iAreaFrmNum = 0 ;
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

    m_iAreaFrmNum = x_sign;

    for(j = 1;j <= labelAreaHeight;j++)
      memcpy(&pLabelDataOut[(j-1)*labelAreaWidth],&p_temp[j*(labelAreaWidth+2)+1],labelAreaWidth*sizeof(uint16_t));

    for(i = 0;i < m_iAreaFrmNum;i++)
    {
      AreaInfo[i].iObjLabelRect[0] = labelAreaWidth-1;
      AreaInfo[i].iObjLabelRect[1] = 0;
      AreaInfo[i].iObjLabelRect[2] = labelAreaHeight-1;
      AreaInfo[i].iObjLabelRect[3] = 0;
    }

    for(m = 0;m < labelAreaHeight;m++)
      for(n = 0;n < labelAreaWidth;n++)
      {
        if(pLabelDataOut[m*labelAreaWidth+n]>0)
        {
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[0] > n)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[0] = n;
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[1] < n)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[1] = n;
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[2] > m)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[2] = m;
          if(AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[3] < m)
            AreaInfo[pLabelDataOut[m*labelAreaWidth+n]-1].iObjLabelRect[3] = m;
          xTotal[pLabelDataOut[m*labelAreaWidth+n]-1]+=n;
          yTotal[pLabelDataOut[m*labelAreaWidth+n]-1]+=m;
        }
      }

      for( i = 0;i < m_iAreaFrmNum; i++)
      {
        AreaInfo[i].t_flag = true ;
        uint16_t bottom_x,bottom_y;
        bottom_x = AreaInfo[i].iObjLabelRect[1];
        bottom_y = AreaInfo[i].iObjLabelRect[3];
        AreaInfo[i].iObjWhiteSpotNum = objWhiteNum[i+1];
        AreaInfo[i].iObjLabelRect[1] = AreaInfo[i].iObjLabelRect[2];
        AreaInfo[i].iObjLabelRect[2] = bottom_x - AreaInfo[i].iObjLabelRect[0] + 1;
        AreaInfo[i].iObjLabelRect[3] = bottom_y - AreaInfo[i].iObjLabelRect[1] + 1;
        AreaInfo[i].iObjLabelCenter[0] = xTotal[i]/objWhiteNum[i+1];
        AreaInfo[i].iObjLabelCenter[1] = yTotal[i]/objWhiteNum[i+1];

        if (AreaInfo[i].iObjWhiteSpotNum < 30 || AreaInfo[i].iObjWhiteSpotNum > 4000 )
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

  float vf_na = (float)(( ParamSet.iPersonRange[0].EndPointY - ParamSet.iPersonRange[0].BeginPointY )/( (float)(ParamSet.iPersonRange[0].EndPointX - ParamSet.iPersonRange[0].BeginPointX)+0.001));
  float vf_nb = (float)(( ParamSet.iPersonRange[0].EndPointY * ParamSet.iPersonRange[0].BeginPointX - ParamSet.iPersonRange[0].BeginPointY* ParamSet.iPersonRange[0].EndPointX )/((float)(ParamSet.iPersonRange[0].BeginPointX - ParamSet.iPersonRange[0].EndPointX)+0.001));

  float vf_nc = (float)(( ParamSet.iPersonRange[1].EndPointY - ParamSet.iPersonRange[1].BeginPointY )/( (float)(ParamSet.iPersonRange[1].EndPointX - ParamSet.iPersonRange[1].BeginPointX )+0.001));
  float vf_nd = (float)(( ParamSet.iPersonRange[1].EndPointY * ParamSet.iPersonRange[1].BeginPointX - ParamSet.iPersonRange[1].BeginPointY* ParamSet.iPersonRange[1].EndPointX )/((float)(ParamSet.iPersonRange[1].BeginPointX - ParamSet.iPersonRange[1].EndPointX)+0.001));

  int16_t XLeft = 0 ;
  int16_t XRight = 0;

  memset ( m_pShowImage[4], 0, m_iFrameWidth*m_iFrameHeight);

  for (j = (int)ParamSet.iPersonRange[2].BeginPointY ; j < m_iFrameHeight-2 ; j++)
  {
    XLeft = (int16_t)( ( (float)j - vf_nb ) / vf_na ) ;
    XRight = (int16_t)( ( (float)j - vf_nd ) / vf_nc );

    if (XLeft > 0 && XLeft < m_iFrameWidth && XRight > 0 && XRight < m_iFrameWidth && XLeft < XRight )
    {
      for(i = XLeft; i < XRight ; i++)
        m_pShowImage[4][i + j* m_iFrameWidth] = 1 ;

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

  for ( i = 0; i < m_iAreaFrmNum-1; i++ )
  {
    for ( j = i; j< m_iAreaFrmNum; j++)
    {
      if ( AreaInfo[i].t_flag && AreaInfo[j].t_flag)
      {
        //判断哪个边小
        vi_left   = ( AreaInfo[i].iObjLabelRect[0]< AreaInfo[j].iObjLabelRect[0])? (AreaInfo[i].iObjLabelRect[0]): (AreaInfo[j].iObjLabelRect[0]);
        vi_right  = ( AreaInfo[i].iObjLabelRect[0]+ AreaInfo[i].iObjLabelRect[2])> (AreaInfo[j].iObjLabelRect[0]+ AreaInfo[j].iObjLabelRect[2] )? (AreaInfo[i].iObjLabelRect[0]+ AreaInfo[i].iObjLabelRect[2]): (AreaInfo[j].iObjLabelRect[0]+AreaInfo[j].iObjLabelRect[2]);
        vi_bottom = ( AreaInfo[i].iObjLabelRect[1]< AreaInfo[j].iObjLabelRect[1])? (AreaInfo[i].iObjLabelRect[1]): (AreaInfo[j].iObjLabelRect[1]);
        vi_top    = ( AreaInfo[i].iObjLabelRect[1]+ AreaInfo[i].iObjLabelRect[3])> (AreaInfo[j].iObjLabelRect[1]+ AreaInfo[j].iObjLabelRect[3] )? (AreaInfo[i].iObjLabelRect[1]+ AreaInfo[i].iObjLabelRect[3]): (AreaInfo[j].iObjLabelRect[1]+AreaInfo[j].iObjLabelRect[3]);
        //判断是否有重合区域
        if ( (vi_right - vi_left) < (AreaInfo[i].iObjLabelRect[2] + AreaInfo[i].iObjLabelRect[2]) )
        {
          if ( AreaInfo[i].iObjLabelCenter[1] < AreaInfo[j].iObjLabelCenter[1] )
          {
            if ( abs( (AreaInfo[i].iObjLabelCenter[1]+AreaInfo[i].iObjLabelCenter[3])- AreaInfo[j].iObjLabelCenter[1]) <=10  )
            {
              AreaInfo[i].iObjLabelRect[0] = vi_left ;
              AreaInfo[i].iObjLabelRect[1] = vi_bottom ;
              AreaInfo[i].iObjLabelRect[2] = vi_right - vi_left ;
              AreaInfo[i].iObjLabelRect[3] = vi_top - vi_bottom ;
              AreaInfo[i].iObjLabelCenter[0] = AreaInfo[i].iObjLabelRect[0] + AreaInfo[i].iObjLabelRect[2]/2 ;
              AreaInfo[i].iObjLabelCenter[1] = AreaInfo[i].iObjLabelRect[1] + AreaInfo[i].iObjLabelRect[3]/2 ;
              AreaInfo[i].iObjWhiteSpotNum += AreaInfo[j].iObjWhiteSpotNum ;
              AreaInfo[j].t_flag = false ;
            }            
          }
          else
          {
            if ( abs( (AreaInfo[j].iObjLabelCenter[1]+AreaInfo[j].iObjLabelCenter[3])- AreaInfo[i].iObjLabelCenter[1]) <=10  )
            {
              AreaInfo[i].iObjLabelRect[0] = vi_left ;
              AreaInfo[i].iObjLabelRect[1] = vi_bottom ;
              AreaInfo[i].iObjLabelRect[2] = vi_right - vi_left ;
              AreaInfo[i].iObjLabelRect[3] = vi_top - vi_bottom ;
              AreaInfo[i].iObjLabelCenter[0] = AreaInfo[i].iObjLabelRect[0] + AreaInfo[i].iObjLabelRect[2]/2 ;
              AreaInfo[i].iObjLabelCenter[1] = AreaInfo[i].iObjLabelRect[1] + AreaInfo[i].iObjLabelRect[3]/2 ;
              AreaInfo[i].iObjWhiteSpotNum += AreaInfo[j].iObjWhiteSpotNum ;
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
  for (i = 0; i < m_iAreaFrmNum; i++)
  {
    if ( AreaInfo[i].t_flag  )
    {
      if ( AreaInfo[i].iObjWhiteSpotNum >  (uint32_t)vi_temp_while_number )
      {
        vi_temp_areaifo_number =  i ;
        vi_temp_while_number = AreaInfo[i].iObjWhiteSpotNum ;
        vb_temp_change_flag = true  ;
      }
    }          
  }
  if (vb_temp_change_flag)
  {
    memcpy(&AreaInfoMax,&AreaInfo[vi_temp_areaifo_number],sizeof(ObjLabelInfoStruct) );
    vi_object_bottom = AreaInfoMax.iObjLabelRect[1] + AreaInfoMax.iObjLabelRect[3] ;
    vi_object_wide   = AreaInfoMax.iObjLabelRect[2] ;
    vi_object_height = AreaInfoMax.iObjLabelRect[3] ;
    //if (vi_object_bottom < m_FrameHeight-10  && vi_object_height*3 > vi_object_wide && vi_object_height < 8*vi_object_wide && AreaInfoMax.WhiteSpotNum > 50 )
    //{
    //  vb_return_value =  true ;
    //}
    //if (vi_object_bottom >= 275 && AreaInfoMax.WhiteSpotNum > 500 )
    //{
    //  vb_return_value =  true ;
    //}
    if (AreaInfoMax.iObjWhiteSpotNum > 80 )
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
  uint16_t framewidth  = m_iFrameWidth  ;    //帧宽
  uint16_t frameheight = m_iFrameHeight ;    //图像高

  float vf_na = (float)(( ParamSet.iPersonRange[0].EndPointY - ParamSet.iPersonRange[0].BeginPointY )/( (float)(ParamSet.iPersonRange[0].EndPointX - ParamSet.iPersonRange[0].BeginPointX)+0.001 ));
  float vf_nb = (float)(( ParamSet.iPersonRange[0].EndPointY * ParamSet.iPersonRange[0].BeginPointX - ParamSet.iPersonRange[0].BeginPointY* ParamSet.iPersonRange[0].EndPointX )/((float)(ParamSet.iPersonRange[0].BeginPointX - ParamSet.iPersonRange[0].EndPointX)+0.001));

  float vf_nc = (float)(( ParamSet.iPersonRange[1].EndPointY - ParamSet.iPersonRange[1].BeginPointY )/( (float)(ParamSet.iPersonRange[1].EndPointX - ParamSet.iPersonRange[1].BeginPointX)+0.001 ));
  float vf_nd = (float)(( ParamSet.iPersonRange[1].EndPointY * ParamSet.iPersonRange[1].BeginPointX - ParamSet.iPersonRange[1].BeginPointY* ParamSet.iPersonRange[1].EndPointX )/((float)(ParamSet.iPersonRange[1].BeginPointX - ParamSet.iPersonRange[1].EndPointX)+0.001));

  for (j = (int)ParamSet.iPersonRange[2].BeginPointY ; j < m_iFrameHeight-2 ; j++)
  {
    XLeft = (int16_t)( ( (float)j - vf_nb ) / vf_na );
    XRight = (int16_t)( ( (float)j - vf_nd ) / vf_nc ) ;

    if (XLeft > 0 && XLeft < m_iFrameWidth && XRight > 0 && XRight < m_iFrameWidth && XLeft < XRight )
    {
      for(i = XLeft; i < XRight ; i++)
        if (p_image_src[i + j* m_iFrameWidth] == 1 )
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
  uint16_t framewidth = m_iFrameWidth;
  uint16_t frameheight = m_iFrameHeight;
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
  uint16_t framewidth  = m_iFrameWidth;  //图像的宽
  uint16_t frameheight = m_iFrameHeight; //图像高
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


void  CParabolaDetect::PersonCreateBK( uint8_t *p_frame ,int ScaleVal )
{
  int i = 0 ;
  int j = 0 ;
  int Number = 0 ;
  int wide = m_iFrameWidth*3 ;
  if ( Number == 15 )
  {
    Number++ ;
    for ( j = 0 ; j < m_iFrameHeight ; j++ )
    {
      for ( i = 0; i < wide ; i++ )
      {
        m_pContinueImage[4][ i + j * wide ] = (m_pContinueImage[4][ i + j * wide ]*9 + p_frame[ i + j * wide ])/10;
      }
    }
  }
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


  for (j = pTrackCurveInfo->iTrackFrameNum/2 ; j < pTrackCurveInfo->iTrackFrameNum  ;j++)
  {
    if (pTrackCurveInfo->iXFrameLocation[j] > 0 )
    {
      v_middle_coordinate_x = pTrackCurveInfo->iXFrameLocation[j];
      v_middle_coordinate_y = pTrackCurveInfo->iYFrameLocation[j];
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
  double v_matrix_y[3] = { pTrackCurveInfo->iCurFrameCenter[1], pTrackCurveInfo->iOriginFrameCenter[1], v_middle_coordinate_y };
  double v_matrix_x[9] = { pTrackCurveInfo->iCurFrameCenter[0]*pTrackCurveInfo->iCurFrameCenter[0],       pTrackCurveInfo->iCurFrameCenter[0],    1,
    pTrackCurveInfo->iOriginFrameCenter[0]*pTrackCurveInfo->iOriginFrameCenter[0], pTrackCurveInfo->iOriginFrameCenter[0], 1,
    v_middle_coordinate_x*v_middle_coordinate_x,   v_middle_coordinate_x , 1  };

  if ( CurveFitting(v_matrix_x,v_matrix_y,v_matrix_a) )
  {
    if (v_matrix_a[0] > 0.004)
    {
      v_inflexion_x = (long)( (-v_matrix_a[1])/ (2.0*v_matrix_a[0]) );

      if (v_inflexion_x > ParamSet.iCurveLeftVal  &&  v_inflexion_x < ParamSet.iCurveRightVal)//450 )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        v_line_y = (long)( m_fLineSecondLocation[0]*v_inflexion_x + m_fLineSecondLocation[1] );
      }

      if (v_inflexion_y < v_line_y)
      {
        return true ;
      }

      if ( ParamSet.tRectLittleRegion.bFlag )
      {
        if (pTrackCurveInfo->iLittleRegionNum >= (pTrackCurveInfo->iFindObjNumber-2))
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

  if (TrackObject[i].bObjDistanceFlg)
  {
    y_height_value = 25 ;
  }
  else
  {
    y_height_value = 10 ;
  }

  if ( ParamSet.tRectLittleRegion.bFlag )
  {
    if ( TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber <= 8 && TrackObject[i].iTrackFrameNum <= 8
      && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iXContinueNum[1] <= 2
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iMigrationDiff[0] >= 0	 
      && TrackObject[i].iLittleRegionNum >= (TrackObject[i].iFindObjNumber-1)
      && (TrackObject[i].iRiseFrameNum[0] >=2 || TrackObject[i].iRiseFrameNum[1] >= 2) 
      && TrackObject[i].iMigrationDiff[1] >= 1 	
      && TrackObject[i].iWhiteSpotNum < 80  
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.6 && TrackObject[i].iMatchNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.6 && TrackObject[i].iMatchNum[1] >= 3 )
      && TrackObject[i].iTrackTopPoint[1] < ParamSet.iLittleRegionTop
      )
    {
      Temp_alarm = TRUE;
    }
  }


  if (ParamSet.bSensitiveFlag)
  {
    if ( TrackObject[i].iTrackFrameNum >= 3 
      && TrackObject[i].bLineRangeFlag[0]
      && TrackObject[i].bLineRangeFlag[1]
      && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iTrackFrameNum <= 10
      && (TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value-3 ) 
      && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
      && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 2 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
      && (TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
      && (TrackObject[i].iRiseFrameNum[0] >=3 || TrackObject[i].iRiseFrameNum[1] >= 3)
      )
    {
      Temp_alarm = TRUE;
    } 
  }


  if ( TrackObject[i].iTrackFrameNum >= 4  
    && TrackObject[i].bLineRangeFlag[0]
    && TrackObject[i].bLineRangeFlag[1]
    && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iLostFrameNum == 0 
    && TrackObject[i].iTrackFrameNum <= 12
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value + 20 )  )
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 4 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 3 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
    && (TrackObject[i].iFindObjNumber >= 6  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
    && (TrackObject[i].iRiseFrameNum[0] >=3 || TrackObject[i].iRiseFrameNum[1] >= 3)
    )
  {
    Temp_alarm = TRUE;
  }
  if ( TrackObject[i].iTrackFrameNum >= 10  && TrackObject[i].iTrackFrameNum <= 18 
    && TrackObject[i].bLineRangeFlag[0]
    && TrackObject[i].bLineRangeFlag[1]
    && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iLostFrameNum == 0 
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 20 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 6 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 5 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 5 )
    && (TrackObject[i].iFindObjNumber >= 10  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 || TrackObject[i].iRiseFrameNum[1] >= 4) 
    )
  {
    Temp_alarm = TRUE;
  }

  if ( TrackObject[i].iTrackFrameNum >= 14   
    && TrackObject[i].bLineRangeFlag[0]
    && TrackObject[i].bLineRangeFlag[1]
    && !TrackObject[i].bTrackAlarmFlag
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 30 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 8 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.3 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.2 )
    && (TrackObject[i].iFindObjNumber >= 12  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 || TrackObject[i].iRiseFrameNum[1] >= 4) 
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

  for (j = pTrackCurveInfo->iTrackFrameNum/2 ; j < pTrackCurveInfo->iTrackFrameNum  ;j++)
  {
    if (pTrackCurveInfo->iXFrameLocation[j] > 0 )
    {
      v_middle_coordinate_x = pTrackCurveInfo->iXFrameLocation[j];
      v_middle_coordinate_y = pTrackCurveInfo->iYFrameLocation[j];
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
  double v_matrix_y[3] = { pTrackCurveInfo->iCurFrameCenter[1], pTrackCurveInfo->iOriginFrameCenter[1], v_middle_coordinate_y };
  double v_matrix_x[9] = { pTrackCurveInfo->iCurFrameCenter[0]*pTrackCurveInfo->iCurFrameCenter[0],       pTrackCurveInfo->iCurFrameCenter[0],    1,
    pTrackCurveInfo->iOriginFrameCenter[0]*pTrackCurveInfo->iOriginFrameCenter[0], pTrackCurveInfo->iOriginFrameCenter[0], 1,
    v_middle_coordinate_x*v_middle_coordinate_x,   v_middle_coordinate_x , 1  };

  if ( CurveFitting(v_matrix_x,v_matrix_y,v_matrix_a) )
  {
    if (v_matrix_a[0] > 0.004)
    {
      v_inflexion_x = (long)( (-v_matrix_a[1])/ (2.0*v_matrix_a[0]) );

      if (v_inflexion_x > ParamSet.iCurveLeftVal  &&  v_inflexion_x < ParamSet.iCurveRightVal )//450 )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        v_line_y = (long)( m_fLineSecondLocation[0]*v_inflexion_x + m_fLineSecondLocation[1] );

        if (v_inflexion_y < v_line_y)
        {
          return true ;
        }
      }
      if (v_inflexion_x > 0 && v_inflexion_x <= ParamSet.iCurveLeftVal )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        if (v_inflexion_y < 60 )
        {
          return true ;
        }
      }

    }


  }

  return false ;

}
bool CParabolaCurve::TrackAlarmObject(uint16_t i)
{
  bool Temp_alarm = false ;
  int16_t y_height_value  = 0 ;

  if (TrackObject[i].iWhiteSpotNum < 200)
  {
    y_height_value = 15 ;
  }
  else
  {
    y_height_value = 20 ;
  }
  if (TrackObject[i].iWhiteSpotNum > 300)
  {
    y_height_value = 30 ;
  }
  if (TrackObject[i].iWhiteSpotNum > 400 )
  {
    y_height_value = 35 ;
  }

  if (TrackObject[i].bObjDistanceFlg)
  {
    y_height_value = 40 ;
  }

  if (TrackObject[i].iTrackTopPoint[0] > 240 )
  {
    y_height_value = 35 ;
  }

  if ( TrackObject[i].iFindObjNumber >= 6  && TrackObject[i].iFindObjNumber <= 12 && TrackObject[i].iTrackFrameNum <= 14
    && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iXContinueNum[1] < 2
    && TrackObject[i].iLostFrameNum == 0 

    // && (abs((int)TrackObject[i]. - (int)TrackObject[i].WhiteSpotNum)< TrackObject[i].PurWhiteSpotNum * 0.6 )
    )
  {

    if ( TrackObject[i].iWhiteSpotNum < 200 
      && TrackObject[i].iTrackTopPoint[0] < 160  
      && ( abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) > 15 && TrackObject[i].iTrackTopPoint[1] <= 61 && TrackObject[i].iTrackBottomPoint[1] > 75)
      //        && TrackObject[i].y_change_number >= ( TrackObject[i].t_find_obj_number - 2 )
      && (TrackObject[i].iMatchNum[0] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.8  ) )
      && (TrackObject[i].iMatchNum[1] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.7  ) )   
      && abs(TrackObject[i].iOriginFrameCenter[0] - TrackObject[i].iCurFrameCenter[0]) > 10
      && TrackObject[i].iMigrationDiff[0] <= 3	)
    {
      Temp_alarm = TRUE;
    }

    if ( TrackObject[i].iWhiteSpotNum >= 200 
      && TrackObject[i].iWhiteSpotNum <= 600
      && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) > 30 
      &&  TrackObject[i].iTrackTopPoint[0] > 200 && TrackObject[i].iTrackTopPoint[1] < 126
      && abs(TrackObject[i].iOriginFrameCenter[0] - TrackObject[i].iCurFrameCenter[0]) > 25
      )
    {
      Temp_alarm = TRUE;
    }
    if ( TrackObject[i].iWhiteSpotNum > 600 
      && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) > 50 
      && (TrackObject[i].iTrackBottomPoint[0] > 260 || TrackObject[i].iTrackTopPoint[0] > 260 )//???????
      && abs(TrackObject[i].iOriginFrameCenter[0] - TrackObject[i].iCurFrameCenter[0]) > 30
      )
    {
      Temp_alarm = TRUE;
    }

  }

  if (ParamSet.bSensitiveFlag)
  {
    if ( TrackObject[i].iTrackFrameNum >= 3  
      && TrackObject[i].bLineRangeFlag[0]
    && TrackObject[i].bLineRangeFlag[1]
    && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iTrackFrameNum <= 10
      && (TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value-3 ) 
      && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
      && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 2 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
      && (TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
      && (TrackObject[i].iRiseFrameNum[0] >=3 || TrackObject[i].iRiseFrameNum[1] >= 3)
      )
    {
      Temp_alarm = TRUE;
    } 
  }

  if ( TrackObject[i].iTrackFrameNum >= 6  && TrackObject[i].iTrackFrameNum <= 14 
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iXContinueNum[1] < 2
    && TrackObject[i].iLostFrameNum == 0 
    && abs(TrackObject[i].iOriginFrameCenter[0] - TrackObject[i].iCurFrameCenter[0]) > 12
    && (  (TrackObject[i].iMigrationDiff[0] >= 0 && (TrackObject[i].iTrackBottomPoint[1]-TrackObject[i].iTrackTopPoint[1]) >= y_height_value && TrackObject[i].iTrackTopPoint[1] < 177 ) ||
    (  (TrackObject[i].iTrackBottomPoint[1]-TrackObject[i].iTrackTopPoint[1]) >= 10 && (TrackObject[i].iRiseFrameNum[0] >=4 && TrackObject[i].iRiseFrameNum[1] >= 4 ) && TrackObject[i].iTrackTopPoint[1] < 83 ) )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 6 )
    && (TrackObject[i].iMatchNum[0] >= (int16_t)(TrackObject[i].iTrackFrameNum * 0.4) && TrackObject[i].iMatchNum[0] >= 5 )
    && (TrackObject[i].iMatchNum[1] >= (int16_t)(TrackObject[i].iTrackFrameNum * 0.3) && TrackObject[i].iMatchNum[1] >= 5 )
    && (TrackObject[i].iFindObjNumber >= 8  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 || TrackObject[i].iRiseFrameNum[1] >= 4 ) 
    )
  {

    Temp_alarm = TRUE;
  }
  if( TrackObject[i].iTrackFrameNum >= 8  && TrackObject[i].iTrackFrameNum <= 18 
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iXContinueNum[1] < 2
    && TrackObject[i].iLostFrameNum == 0 
    && abs(TrackObject[i].iOriginFrameCenter[0] - TrackObject[i].iCurFrameCenter[0]) > 20
    && (  (TrackObject[i].iMigrationDiff[0] >= 0 &&(TrackObject[i].iTrackBottomPoint[1]-TrackObject[i].iTrackTopPoint[1]) >= y_height_value  && TrackObject[i].iTrackTopPoint[1] < 177) ||
    (  (TrackObject[i].iTrackBottomPoint[1]-TrackObject[i].iTrackTopPoint[1]) >= 15 && (TrackObject[i].iRiseFrameNum[0] >=4 && TrackObject[i].iRiseFrameNum[1] >= 4 ) && TrackObject[i].iTrackTopPoint[1] < 83 ) )
    && (TrackObject[i].iXContinueNum[0] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.6)  )
    && (TrackObject[i].iMatchNum[0] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.7)  )
    && (TrackObject[i].iMatchNum[1] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.7)  )
    && (TrackObject[i].iFindObjNumber >= 10  )
    && (TrackObject[i].iRiseFrameNum[0] >=2 || TrackObject[i].iRiseFrameNum[1] >= 2) 
    )
  {
    Temp_alarm = TRUE;
  }
  if( TrackObject[i].iTrackFrameNum >= 11  && TrackObject[i].iTrackFrameNum <= 18 
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iXContinueNum[1] < 2
    && TrackObject[i].iLostFrameNum == 0 
    && abs(TrackObject[i].iOriginFrameCenter[0] - TrackObject[i].iCurFrameCenter[0]) > 30
    && (  (TrackObject[i].iMigrationDiff[0] >= 0 &&(TrackObject[i].iTrackBottomPoint[1]-TrackObject[i].iTrackTopPoint[1]) >= y_height_value && TrackObject[i].iTrackTopPoint[1] < 177 ) ||
    ((TrackObject[i].iTrackBottomPoint[1]-TrackObject[i].iTrackTopPoint[1]) >= 20 && (TrackObject[i].iRiseFrameNum[0] >=4 && TrackObject[i].iRiseFrameNum[1] >= 4 ) && TrackObject[i].iTrackTopPoint[1] < 83 ) )
    && (TrackObject[i].iXContinueNum[0] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.6)  )
    && (   TrackObject[i].iMatchNum[0] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.7)  
    || TrackObject[i].iMatchNum[1] >= (int16_t)(TrackObject[i].iFindObjNumber * 0.7)  )
    && (TrackObject[i].iFindObjNumber >= 13  )
    && (TrackObject[i].iRiseFrameNum[0] >=4 || TrackObject[i].iRiseFrameNum[1] >= 4) 
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
bool CParabolaAbove::CurveContrast( LabelObjStatus* pTrackCurveInfo)
{
  int    i = 0 ; 
  int    j = 0 ;

  bool   v_temp_flag = false ;
  double v_middle_coordinate_x = 0 ;  
  double v_middle_coordinate_y = 0 ;

  long v_inflexion_x = 0 ; 
  long v_inflexion_y = 0 ;
  long v_line_y = 0 ;


  for (j = pTrackCurveInfo->iTrackFrameNum/2 ; j < pTrackCurveInfo->iTrackFrameNum  ;j++)
  {
    if (pTrackCurveInfo->iXFrameLocation[j] > 0 )
    {
      v_middle_coordinate_x = pTrackCurveInfo->iXFrameLocation[j];
      v_middle_coordinate_y = pTrackCurveInfo->iYFrameLocation[j];
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
  double v_matrix_y[3] = { pTrackCurveInfo->iCurFrameCenter[1], pTrackCurveInfo->iOriginFrameCenter[1], v_middle_coordinate_y };
  double v_matrix_x[9] = { pTrackCurveInfo->iCurFrameCenter[0]*pTrackCurveInfo->iCurFrameCenter[0],       pTrackCurveInfo->iCurFrameCenter[0],    1,
    pTrackCurveInfo->iOriginFrameCenter[0]*pTrackCurveInfo->iOriginFrameCenter[0], pTrackCurveInfo->iOriginFrameCenter[0], 1,
    v_middle_coordinate_x*v_middle_coordinate_x,   v_middle_coordinate_x , 1  };

  if ( CurveFitting(v_matrix_x,v_matrix_y,v_matrix_a) )
  {
    if (v_matrix_a[0] > 0.004)
    {
      v_inflexion_x = (long)( (-v_matrix_a[1])/ (2.0*v_matrix_a[0]) );

      if (v_inflexion_x > ParamSet.iCurveLeftVal  &&  v_inflexion_x < ParamSet.iCurveRightVal )//450 )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        v_line_y = (long)( m_fLineSecondLocation[0]*v_inflexion_x + m_fLineSecondLocation[1] );
      }

      if (v_inflexion_y < v_line_y)
      {
        return true ;
      }

      if ( ParamSet.tRectLittleRegion.bFlag )
      {
        if (pTrackCurveInfo->iLittleRegionNum >= (pTrackCurveInfo->iFindObjNumber-2))
        {
          return true ;
        }
      }

    }


  }

  return false ;

}
bool CParabolaAbove::TrackAlarmObject(uint16_t i)
{
  bool Temp_alarm = false ;
  int16_t y_height_value  = 0 ;

  if (TrackObject[i].bObjDistanceFlg)
  {
    y_height_value = 25 ;
  }
  else
  {
    y_height_value = 10 ;
  }

  if ( ParamSet.tRectLittleRegion.bFlag )
  {
    if ( TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber <= 8 && TrackObject[i].iTrackFrameNum <= 8
      && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iXContinueNum[1] <= 2
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iMigrationDiff[0] >= 0	 
      && TrackObject[i].iLittleRegionNum >= (TrackObject[i].iFindObjNumber-1)
      && (TrackObject[i].iRiseFrameNum[0] >=2 || TrackObject[i].iRiseFrameNum[1] >= 2) 
      && TrackObject[i].iMigrationDiff[1] >= 1 	
      && TrackObject[i].iWhiteSpotNum < 80  
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.6 && TrackObject[i].iMatchNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.6 && TrackObject[i].iMatchNum[1] >= 3 )
      && TrackObject[i].iTrackTopPoint[1] < ParamSet.iLittleRegionTop
      )
    {
      Temp_alarm = TRUE;
    }
  }


  if (ParamSet.bSensitiveFlag)
  {
    if ( TrackObject[i].iTrackFrameNum >= 3  
      && TrackObject[i].bLineRangeFlag[0]
    && TrackObject[i].bLineRangeFlag[1]
    && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iTrackFrameNum <= 10
      && (TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value-3 ) 
      && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
      && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 2 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
      && (TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
      && (TrackObject[i].iRiseFrameNum[0] >=3 || TrackObject[i].iRiseFrameNum[1] >= 3)
      )
    {
      Temp_alarm = TRUE;
    } 
  }

  if ( TrackObject[i].iTrackFrameNum >= 4  
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iLostFrameNum == 0 
    && TrackObject[i].iTrackFrameNum <= 12
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value + 20 )  )
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 4 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 3 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
    && (TrackObject[i].iFindObjNumber >= 6  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
    && (TrackObject[i].iRiseFrameNum[0] >=3 && TrackObject[i].iRiseFrameNum[1] >= 3)
    )
  {
    Temp_alarm = TRUE;
  }
  if ( TrackObject[i].iTrackFrameNum >= 10  && TrackObject[i].iTrackFrameNum <= 18 
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iLostFrameNum == 0 
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 20 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 6 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 5 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 5 )
    && (TrackObject[i].iFindObjNumber >= 10  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 && TrackObject[i].iRiseFrameNum[1] >= 4) 
    )
  {
    Temp_alarm = TRUE;
  }

  if ( TrackObject[i].iTrackFrameNum >= 14   
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 30 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 8 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.3 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.2 )
    && (TrackObject[i].iFindObjNumber >= 12  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 && TrackObject[i].iRiseFrameNum[1] >= 4) 
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
bool CParabolaTree::CurveContrast( LabelObjStatus* pTrackCurveInfo)
{
  int    i = 0 ; 
  int    j = 0 ;

  bool   v_temp_flag = false ;
  double v_middle_coordinate_x = 0 ;  
  double v_middle_coordinate_y = 0 ;

  long v_inflexion_x = 0 ; 
  long v_inflexion_y = 0 ;
  long v_line_y = 0 ;


  for (j = pTrackCurveInfo->iTrackFrameNum/2 ; j < pTrackCurveInfo->iTrackFrameNum  ;j++)
  {
    if (pTrackCurveInfo->iXFrameLocation[j] > 0 )
    {
      v_middle_coordinate_x = pTrackCurveInfo->iXFrameLocation[j];
      v_middle_coordinate_y = pTrackCurveInfo->iYFrameLocation[j];
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
  double v_matrix_y[3] = { pTrackCurveInfo->iCurFrameCenter[1], pTrackCurveInfo->iOriginFrameCenter[1], v_middle_coordinate_y };
  double v_matrix_x[9] = { pTrackCurveInfo->iCurFrameCenter[0]*pTrackCurveInfo->iCurFrameCenter[0],       pTrackCurveInfo->iCurFrameCenter[0],    1,
    pTrackCurveInfo->iOriginFrameCenter[0]*pTrackCurveInfo->iOriginFrameCenter[0], pTrackCurveInfo->iOriginFrameCenter[0], 1,
    v_middle_coordinate_x*v_middle_coordinate_x,   v_middle_coordinate_x , 1  };

  if ( CurveFitting(v_matrix_x,v_matrix_y,v_matrix_a) )
  {
    if (v_matrix_a[0] > 0.004)
    {
      v_inflexion_x = (long)( (-v_matrix_a[1])/ (2.0*v_matrix_a[0]) );

      if (v_inflexion_x > ParamSet.iCurveLeftVal  &&  v_inflexion_x < ParamSet.iCurveRightVal )//450 )
      {
        v_inflexion_y = (long)( v_matrix_a[0]*v_inflexion_x*v_inflexion_x + v_inflexion_x*v_matrix_a[1] + v_matrix_a[2] );
        v_line_y = (long)( m_fLineSecondLocation[0]*v_inflexion_x + m_fLineSecondLocation[1] );
      }

      if (v_inflexion_y < v_line_y)
      {
        return true ;
      }

      if ( ParamSet.tRectLittleRegion.bFlag )
      {
        if (pTrackCurveInfo->iLittleRegionNum >= (pTrackCurveInfo->iFindObjNumber-2))
        {
          return true ;
        }
      }

    }


  }

  return false ;

}
bool CParabolaTree::TrackAlarmObject(uint16_t i)
{
  bool Temp_alarm = false ;
  int16_t y_height_value  = 0 ;

  if (TrackObject[i].bObjDistanceFlg)
  {
    y_height_value = 25 ;
  }
  else
  {
    y_height_value = 10 ;
  }

  if ( ParamSet.tRectLittleRegion.bFlag )
  {
    if ( TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber <= 8 && TrackObject[i].iTrackFrameNum <= 8
      && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iXContinueNum[1] <= 2
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iMigrationDiff[0] >= 0	 
      && TrackObject[i].iLittleRegionNum >= (TrackObject[i].iFindObjNumber-1)
      && (TrackObject[i].iRiseFrameNum[0] >=2 || TrackObject[i].iRiseFrameNum[1] >= 2) 
      && TrackObject[i].iMigrationDiff[1] >= 1 	
      && TrackObject[i].iWhiteSpotNum < 80  
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.6 && TrackObject[i].iMatchNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.6 && TrackObject[i].iMatchNum[1] >= 3 )
      && TrackObject[i].iTrackTopPoint[1] < ParamSet.iLittleRegionTop
      )
    {
      Temp_alarm = TRUE;
    }
  }

  if (ParamSet.tRectTreeLittleRegion.bFlag )
  {
    if ( TrackObject[i].iFindObjNumber >= 4  
      && TrackObject[i].iFindObjNumber <= 8 
      && TrackObject[i].iTrackFrameNum <= 8
      && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iXContinueNum[1] < 2
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iMigrationDiff[0] >= 1	 
      && TrackObject[i].iTreeLittleRegionNum >= 4 
      && abs((int16_t)TrackObject[i].iOriginFrameCenter[0]-(int16_t)TrackObject[i].iCurFrameCenter[0]) >= 12 
      )
    {
      Temp_alarm = TRUE;
    }
  }

  if (ParamSet.bSensitiveFlag)
  {
    if ( TrackObject[i].iTrackFrameNum >= 3  
      && TrackObject[i].bLineRangeFlag[0]
    && TrackObject[i].bLineRangeFlag[1]
    && !TrackObject[i].bTrackAlarmFlag
      && TrackObject[i].iLostFrameNum == 0 
      && TrackObject[i].iTrackFrameNum <= 10
      && (TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value-3 ) 
      && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
      && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 3 )
      && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 2 )
      && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
      && (TrackObject[i].iFindObjNumber >= 5  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
      && (TrackObject[i].iRiseFrameNum[0] >=3 || TrackObject[i].iRiseFrameNum[1] >= 3)
      )
    {
      Temp_alarm = TRUE;
    } 
  }

  if ( TrackObject[i].iTrackFrameNum >= 4  
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iLostFrameNum == 0 
    && TrackObject[i].iTrackFrameNum <= 12
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value + 20 )  )
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 5 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 4 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 3 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 2 )
    && (TrackObject[i].iFindObjNumber >= 6  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.6)
    && (TrackObject[i].iRiseFrameNum[0] >=3 || TrackObject[i].iRiseFrameNum[1] >= 3)
    )
  {
    Temp_alarm = TRUE;
  }
  if ( TrackObject[i].iTrackFrameNum >= 10  && TrackObject[i].iTrackFrameNum <= 18 
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && TrackObject[i].iLostFrameNum == 0 
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 20 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 6 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.4 && TrackObject[i].iMatchNum[0] >= 5 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.3 && TrackObject[i].iMatchNum[1] >= 5 )
    && (TrackObject[i].iFindObjNumber >= 10  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 || TrackObject[i].iRiseFrameNum[1] >= 4) 
    )
  {
    Temp_alarm = TRUE;
  }

  if ( TrackObject[i].iTrackFrameNum >= 14   
    && TrackObject[i].bLineRangeFlag[0]
  && TrackObject[i].bLineRangeFlag[1]
  && !TrackObject[i].bTrackAlarmFlag
    && ((TrackObject[i].iMigrationDiff[0] >= 1 && abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >= y_height_value ) || (abs(TrackObject[i].iTrackBottomPoint[1] - TrackObject[i].iTrackTopPoint[1]) >=  y_height_value + 15))
    && (abs(TrackObject[i].iCurFrameCenter[0]-TrackObject[i].iOriginFrameCenter[0]) > 30 )
    && (TrackObject[i].iXContinueNum[0] >= TrackObject[i].iTrackFrameNum * 0.5 ||TrackObject[i].iXContinueNum[0] >= 8 )
    && (TrackObject[i].iMatchNum[0] >= TrackObject[i].iTrackFrameNum * 0.3 )
    && (TrackObject[i].iMatchNum[1] >= TrackObject[i].iTrackFrameNum * 0.2 )
    && (TrackObject[i].iFindObjNumber >= 12  && TrackObject[i].iFindObjNumber >= TrackObject[i].iTrackFrameNum * 0.7)
    && (TrackObject[i].iRiseFrameNum[0] >=4 || TrackObject[i].iRiseFrameNum[1] >= 4) 
    )
  {
    Temp_alarm = TRUE;    
  }

  return Temp_alarm;

}


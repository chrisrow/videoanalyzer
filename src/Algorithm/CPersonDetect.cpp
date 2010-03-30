/*!
************************************************************************
* \file                   CPersonDetect.cpp
*    
* \brief
*    <<?? 
* \date
*    2008_09_08
* \author
*    - Yang SONG                      <soany777@163.com>
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
************************************************************************
*/
#include "stdafx.h"

#include "CPersonDetect.h"
#include "Macro.h"

extern int g_debug;


CPersonDetect::CPersonDetect(uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in,const uint8_t nChannel)
{
  initBkdet( nYWidth_in, nYHeight_in, YUVTYPE_444 );
  initUpdateBk( nYWidth_in, nYHeight_in, YUVTYPE_444 );
  init_detect( nYWidth_in, nYHeight_in, YUVTYPE_444 ,nChannel);
  init_pudong( nYWidth_in, nYHeight_in, YUVTYPE_444 );
}

CPersonDetect::~CPersonDetect()
{
  clearBkdet();
  clear_detect();
  clear_pudong();
  clearUpdateBk();
}

//==求背景
ErrVal
CPersonDetect::initBkdet( uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in) // init function
{
  m_nGraspFrameInterval     =   BKFRAMEINTERVAL ;     //n_fg帧取一帧数据
  m_nGraspFramesBufferSize  =   BKFRAMEBUFSZ    ;     //保存m_ndate帧数据计算一个背景
  m_nRefreshFrameNum        =   0               ;     //m_nIncomingFrameNum<=m_nGraspFrameInterval,当m_nIncomingFrameNum = m_nGraspFrameInterval时,计算背景。m_nIncomingFrameNum = 0时，重新计算背景。
  m_GraspFramesBuffer       =   new CFramesBuffer( m_nGraspFramesBufferSize, nYWidth_in, nYHeight_in, YuvType_in );

  ASSERT( m_GraspFramesBuffer );
  ROK();
}

ErrVal
CPersonDetect::clearBkdet()
{
  SAFEDELETE( m_GraspFramesBuffer );
  ROK();
}

ErrVal
CPersonDetect::initUpdateBk( uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in) // init function
{
  for (int i = 0 ; i < m_nbufsz ; ++i)
  {
    m_nGetBKBuffer[i] = new uint8_t[3 * nYWidth_in * nYHeight_in];
    memset(m_nGetBKBuffer[i] , 0 ,3 * nYWidth_in * nYHeight_in);
  }
  ROK();
}

ErrVal 
CPersonDetect::clearUpdateBk()
{
  for ( int i = 0; i < m_nbufsz ; ++i)
  {
    SAFEDELETEARRAY(m_nGetBKBuffer[i])
  }
  ROK();
}

ErrVal
CPersonDetect::generateRgbBkgnd_fromRgbFrames( CFrameContainer*   frame_bkDetected,
                                              const CFrameContainer* const  pFrame_curr_in,                                                                    
                                              const CFrameContainer* const binBkgnd )  
{
  ASSERT(frame_bkDetected);
  ASSERT(pFrame_curr_in);
  ASSERT(binBkgnd);
  uint16_t sum = 0;
  uint32_t i = 0, j = 0, m = 0;
  bool flag = false;
  uint8_t** pFramesBmpBuffer = new uint8_t* [m_nGraspFramesBufferSize];//m_GraspFramesBuffer中2到7帧移到1到6的位置
  //   CFrameContainer*  frame_bkDetected_temp = new CFrameContainer(*frame_bkDetected); 
  const uint32_t imRgbByteSize = pFrame_curr_in->getRgbSize();
  const uint32_t imRByteSize = pFrame_curr_in->getRSize();

  if( m_GraspFramesBuffer->status.nAllocatedFrames == m_GraspFramesBuffer->getBufferSize() )
  {
    if( (m_nRefreshFrameNum - m_nGraspFramesBufferSize +1)  % m_nGraspFrameInterval == 0)   //保存数据
    {
      m_GraspFramesBuffer->pushFrame( pFrame_curr_in );
      flag = true;
    }
  }
  else //m_GraspFramesBuffer没有填满
  {
    m_GraspFramesBuffer->pushFrame( pFrame_curr_in );
    if (m_nRefreshFrameNum == m_nGraspFramesBufferSize -1)
    {
      for( i = 0; i < m_nGraspFramesBufferSize; ++i )
      {
        pFramesBmpBuffer[i] = m_GraspFramesBuffer->getFrame(i)->m_BmpBuffer;
      }
      for( m = 0; m < imRgbByteSize; ++m )
      {
        for( i=0; i < m_nGraspFramesBufferSize; ++i )
        {
          sum += pFramesBmpBuffer[i][m] ;
        }
        frame_bkDetected->m_BmpBuffer[m] = (sum / m_nGraspFramesBufferSize );
        sum = 0;
      }
    }
  }
  if (flag)
  {
    for( m = 0; m < imRgbByteSize; ++m )
    {
      frame_bkDetected->m_BmpBuffer[m] = (uint8_t)(((float)frame_bkDetected->m_BmpBuffer[m])*6/7 
                                                 + (float)pFrame_curr_in->m_BmpBuffer[m]*1/7 + 0.5);
    }

    //     for( m=0; m<imRByteSize; ++m )//打补丁
    //     {
    //       if(255 == binBkgnd->m_YuvPlane[0][m])
    //       {
    //         memcpy( &frame_bkDetected->m_BmpBuffer[3*m + 0], &frame_bkDetected_temp->m_BmpBuffer[3*m + 0], 3*sizeof(frame_bkDetected->m_BmpBuffer[0]) );
    //       }
    //     }

    flag = false;
  }
  if ( m_nRefreshFrameNum == m_nGraspFrameInterval )
    m_nRefreshFrameNum = 1;
  else
    m_nRefreshFrameNum++;
  //   SAFEDELETE( frame_bkDetected_temp );
  SAFEDELETEARRAY( pFramesBmpBuffer );
  ROK();
}

//==处理
ErrVal
CPersonDetect::init_detect(uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in,const uint8_t nChannel)
{
  pMatlabFunc           = new CMatlabFunc;
  pFrame_RgbSmoothed    = new CFrameContainer(nYWidth_in, nYHeight_in, YuvType_in);
  pFrame_bkgndDetected  = new CFrameContainer(nYWidth_in, nYHeight_in, YuvType_in);
  pFrame_binremovebody  = new CFrameContainer(nYWidth_in, nYHeight_in, YuvType_in) ;
  ObjectLabeledDList    = new CDList< CObjLabeled*, CPointerDNode >();
  ObjectLabeledDList1   = new CDList< CObjLabeled*, CPointerDNode >();
  pFrame_previous       = new CFrameContainer(nYWidth_in, nYHeight_in, YuvType_in);

  m_nChannel_ID         = nChannel;

  memset(pFrame_binremovebody->m_YuvPlane[0],WHITESPOT,pFrame_binremovebody->getYSize());
  CurFrameNum = 0;

  ROK();
}

ErrVal
CPersonDetect::init_pudong (uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in)
{
  /*浦东机场，人员越过预警线检测并报警功能。*/

  /*低分辨率用*/
  //   pFrame_matlabFunced_low   = new CFrameContainer(nYWidth_in/2, nYHeight_in/2, YuvType_in);
  //   pFrame_input_low          = new CFrameContainer(nYWidth_in/2, nYHeight_in/2, YuvType_in);
  //   pFrame_RgbSmoothed_low    = new CFrameContainer(nYWidth_in/2, nYHeight_in/2, YuvType_in);
  //   pFrame_bkgndDetected_low  = new CFrameContainer(nYWidth_in/2, nYHeight_in/2, YuvType_in);

  /*Initialize obj match and track*/
  init_match_track( );
  /*Initialize obj match and track   end*/


  //根据每条轨迹告警，这里删除掉
  b_First_Alarm     =  false;//一级报警信号
  b_Second_Alarm    =  false;//二级报警信号

  /*
  * 多路运行时，应根据实际场景修改以下参数。包括：
  *  1. 注释多余的预警线、报警线
  *  2. 在Intelligent_video.ini中修改预警线、报警线参数
  */

  memset(&Warning_Line,0,m_nMaxCameraNum*sizeof(Cordon_Par));
  memset(&Alarm_Line,0,m_nMaxCameraNum*sizeof(Cordon_Par));

  int16_t warning_pt1_x,warning_pt1_y,warning_pt2_x,warning_pt2_y;
  int16_t alarm_pt1_x,alarm_pt1_y,alarm_pt2_x,alarm_pt2_y;

  /*Channel_1*/

  warning_pt1_x = ReadIni("C://Intelligent_video.ini","WarningLine0","warning_pt1_x");
  warning_pt1_y = ReadIni("C://Intelligent_video.ini","WarningLine0","warning_pt1_y");
  warning_pt2_x = ReadIni("C://Intelligent_video.ini","WarningLine0","warning_pt2_x");
  warning_pt2_y = ReadIni("C://Intelligent_video.ini","WarningLine0","warning_pt2_y");
  alarm_pt1_x   = ReadIni("C://Intelligent_video.ini","AlarmLine0","alarm_pt1_x");
  alarm_pt1_y   = ReadIni("C://Intelligent_video.ini","AlarmLine0","alarm_pt1_y");
  alarm_pt2_x   = ReadIni("C://Intelligent_video.ini","AlarmLine0","alarm_pt2_x");
  alarm_pt2_y   = ReadIni("C://Intelligent_video.ini","AlarmLine0","alarm_pt2_y");
  Warning_Line[0].m_distancetotop     = ReadIni("C://Intelligent_video.ini","WarningLine0","warning_distancetotop");
  Warning_Line[0].m_distancetobottom  = ReadIni("C://Intelligent_video.ini","WarningLine0","warning_distancetobottom");
  Alarm_Line[0].m_distancetotop       = ReadIni("C://Intelligent_video.ini","AlarmLine0","alarm_distancetotop");
  Alarm_Line[0].m_distancetobottom    = ReadIni("C://Intelligent_video.ini","AlarmLine0","alarm_distancetobottom");

  Calculate_Line_Parameter(Warning_Line[0].m_slope,Warning_Line[0].m_pitch,warning_pt1_x,warning_pt1_y,warning_pt2_x,warning_pt2_y);//扩展到4路
  Calculate_Line_Parameter(Alarm_Line[0].m_slope,Alarm_Line[0].m_pitch,alarm_pt1_x,alarm_pt1_y,alarm_pt2_x,alarm_pt2_y);
  /*Channel_1  end*/


  /*Channel_2*/

  warning_pt1_x = ReadIni("C://Intelligent_video.ini","WarningLine1","warning_pt1_x");
  warning_pt1_y = ReadIni("C://Intelligent_video.ini","WarningLine1","warning_pt1_y");
  warning_pt2_x = ReadIni("C://Intelligent_video.ini","WarningLine1","warning_pt2_x");
  warning_pt2_y = ReadIni("C://Intelligent_video.ini","WarningLine1","warning_pt2_y");
  alarm_pt1_x   = ReadIni("C://Intelligent_video.ini","AlarmLine1","alarm_pt1_x");
  alarm_pt1_y   = ReadIni("C://Intelligent_video.ini","AlarmLine1","alarm_pt1_y");
  alarm_pt2_x   = ReadIni("C://Intelligent_video.ini","AlarmLine1","alarm_pt2_x");
  alarm_pt2_y   = ReadIni("C://Intelligent_video.ini","AlarmLine1","alarm_pt2_y");
  Warning_Line[1].m_distancetotop     = ReadIni("C://Intelligent_video.ini","WarningLine1","warning_distancetotop");
  Warning_Line[1].m_distancetobottom  = ReadIni("C://Intelligent_video.ini","WarningLine1","warning_distancetobottom");
  Alarm_Line[1].m_distancetotop       = ReadIni("C://Intelligent_video.ini","AlarmLine1","alarm_distancetotop");
  Alarm_Line[1].m_distancetobottom    = ReadIni("C://Intelligent_video.ini","AlarmLine1","alarm_distancetobottom");
  Calculate_Line_Parameter(Warning_Line[1].m_slope,Warning_Line[1].m_pitch,warning_pt1_x,warning_pt1_y,warning_pt2_x,warning_pt2_y);//扩展到4路
  Calculate_Line_Parameter(Alarm_Line[1].m_slope,Alarm_Line[1].m_pitch,alarm_pt1_x,alarm_pt1_y,alarm_pt2_x,alarm_pt2_y);
  /*Channel_2  end*/

  /*Channel_3*/
  warning_pt1_x = ReadIni("C://Intelligent_video.ini","WarningLine2","warning_pt1_x");
  warning_pt1_y = ReadIni("C://Intelligent_video.ini","WarningLine2","warning_pt1_y");
  warning_pt2_x = ReadIni("C://Intelligent_video.ini","WarningLine2","warning_pt2_x");
  warning_pt2_y = ReadIni("C://Intelligent_video.ini","WarningLine2","warning_pt2_y");
  alarm_pt1_x   = ReadIni("C://Intelligent_video.ini","AlarmLine2","alarm_pt1_x");
  alarm_pt1_y   = ReadIni("C://Intelligent_video.ini","AlarmLine2","alarm_pt1_y");
  alarm_pt2_x   = ReadIni("C://Intelligent_video.ini","AlarmLine2","alarm_pt2_x");
  alarm_pt2_y   = ReadIni("C://Intelligent_video.ini","AlarmLine2","alarm_pt2_y");
  Warning_Line[2].m_distancetotop     = ReadIni("C://Intelligent_video.ini","WarningLine2","warning_distancetotop");
  Warning_Line[2].m_distancetobottom  = ReadIni("C://Intelligent_video.ini","WarningLine2","warning_distancetobottom");
  Alarm_Line[2].m_distancetotop       = ReadIni("C://Intelligent_video.ini","AlarmLine2","alarm_distancetotop");
  Alarm_Line[2].m_distancetobottom    = ReadIni("C://Intelligent_video.ini","AlarmLine2","alarm_distancetobottom");

  Calculate_Line_Parameter(Warning_Line[2].m_slope,Warning_Line[2].m_pitch,warning_pt1_x,warning_pt1_y,warning_pt2_x,warning_pt2_y);//扩展到4路
  Calculate_Line_Parameter(Alarm_Line[2].m_slope,Alarm_Line[2].m_pitch,alarm_pt1_x,alarm_pt1_y,alarm_pt2_x,alarm_pt2_y);
  /*Channel_3  end*/

  /*Channel_4*/

  warning_pt1_x = ReadIni("C://Intelligent_video.ini","WarningLine3","warning_pt1_x");
  warning_pt1_y = ReadIni("C://Intelligent_video.ini","WarningLine3","warning_pt1_y");
  warning_pt2_x = ReadIni("C://Intelligent_video.ini","WarningLine3","warning_pt2_x");
  warning_pt2_y = ReadIni("C://Intelligent_video.ini","WarningLine3","warning_pt2_y");
  alarm_pt1_x   = ReadIni("C://Intelligent_video.ini","AlarmLine3","alarm_pt1_x");
  alarm_pt1_y   = ReadIni("C://Intelligent_video.ini","AlarmLine3","alarm_pt1_y");
  alarm_pt2_x   = ReadIni("C://Intelligent_video.ini","AlarmLine3","alarm_pt2_x");
  alarm_pt2_y   = ReadIni("C://Intelligent_video.ini","AlarmLine3","alarm_pt2_y");
  Warning_Line[3].m_distancetotop     = ReadIni("C://Intelligent_video.ini","WarningLine3","warning_distancetotop");
  Warning_Line[3].m_distancetobottom  = ReadIni("C://Intelligent_video.ini","WarningLine3","warning_distancetobottom");
  Alarm_Line[3].m_distancetotop       = ReadIni("C://Intelligent_video.ini","AlarmLine3","alarm_distancetotop");
  Alarm_Line[3].m_distancetobottom    = ReadIni("C://Intelligent_video.ini","AlarmLine3","alarm_distancetobottom");

  Calculate_Line_Parameter(Warning_Line[3].m_slope,Warning_Line[3].m_pitch,/*144,64,253,282*/warning_pt1_x,warning_pt1_y,warning_pt2_x,warning_pt2_y);//扩展到4路
  Calculate_Line_Parameter(Alarm_Line[3].m_slope,Alarm_Line[3].m_pitch,alarm_pt1_x,alarm_pt1_y,alarm_pt2_x,alarm_pt2_y);
  /*Channel_4  end*/

  ROK();
}
ErrVal
CPersonDetect::clear_pudong()
{
  /*浦东机场，人员越过预警线检测并报警功能。*/
  /*低分辨率用*/
  //   SAFEDELETE( pFrame_input_low );
  //   SAFEDELETE( pFrame_matlabFunced_low );
  //   SAFEDELETE( pFrame_RgbSmoothed_low );
  //   SAFEDELETE( pFrame_bkgndDetected_low );
  ROK();
}
ErrVal
CPersonDetect::clear_detect()
{
  SAFEDELETE( pMatlabFunc );
  SAFEDELETE( pFrame_RgbSmoothed );
  SAFEDELETE( pFrame_bkgndDetected );
  SAFEDELETE( pFrame_binremovebody );
  SAFEDELETE( ObjectLabeledDList );
  SAFEDELETE( ObjectLabeledDList1 );
  SAFEDELETE( pFrame_previous );
  ROK();
}


ErrVal 
CPersonDetect::averagesmoothRgb(  CFrameContainer* pFrame_RgbSmoothed_inout , \
                                const CFrameContainer*  const pFrame_decoded_in,\
                                const uint16_t NoSmoothLineNum ) const//均值平滑
{
  ASSERT( pFrame_RgbSmoothed_inout );
  ASSERT( pFrame_decoded_in );

  const uint16_t imWidth  = pFrame_decoded_in->getWidth();
  const uint16_t imHeight = pFrame_decoded_in->getHeight();

  if( (NoSmoothLineNum >= imHeight) || (NoSmoothLineNum <1) )
    return 0;

  *pFrame_RgbSmoothed_inout = *pFrame_decoded_in;
  int16_t i = 0, j = 0, m = 0, n = 0; 
  uint16_t sumB = 0, sumG = 0, sumR = 0;
  for( i = NoSmoothLineNum; i < imHeight-1; ++i )
  {
    for( j = 1; j < imWidth-1; ++j )
    {
      sumB = 0, sumG = 0, sumR = 0;
      for (m = -1; m < 2; m++)
      {
        for (n = -1; n < 2; n++)
        {
          sumB += pFrame_RgbSmoothed_inout->m_BmpBuffer[ 3 * ( i + m ) * imWidth + 3 * ( j + n ) + 0];
          sumG += pFrame_RgbSmoothed_inout->m_BmpBuffer[ 3 * ( i + m ) * imWidth + 3 * ( j + n ) + 1];
          sumR += pFrame_RgbSmoothed_inout->m_BmpBuffer[ 3 * ( i + m ) * imWidth + 3 * ( j + n ) + 2];
        }
      }
      pFrame_RgbSmoothed_inout->m_BmpBuffer[ 3 * i * imWidth + 3 * j + 0] = ( sumB / 9 );
      pFrame_RgbSmoothed_inout->m_BmpBuffer[ 3 * i * imWidth + 3 * j + 1] = ( sumG / 9 );
      pFrame_RgbSmoothed_inout->m_BmpBuffer[ 3 * i * imWidth + 3 * j + 2] = ( sumR / 9 );
    }
  }
  ROK();
}

ErrVal CPersonDetect::binarizeY_fromRgbBkgnd( CFrameContainer* pFrame_RgbtoYBinarized_inout,  \
                                             const CFrameContainer* const pFrame_curr_in,                 \
                                             const CFrameContainer* const pFrame_bkgnd_in,
                                             uint16_t RgbThreshold) const
{
  ASSERT( pFrame_RgbtoYBinarized_inout );
  ASSERT( pFrame_curr_in );
  ASSERT( pFrame_bkgnd_in );

  const uint8_t* const pRgb_bkgnd = pFrame_bkgnd_in->m_BmpBuffer;
  const uint8_t* const pRgb_curr  = pFrame_curr_in->m_BmpBuffer;

  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();

  uint8_t* pY_RgbtoYBinarized = pFrame_RgbtoYBinarized_inout->m_YuvPlane[0];
  int16_t Temp_R, Temp_G, Temp_B, Temp_BGR_Sum;

  //二值
  for (uint32_t j = 0 ; j < imHeight ; ++j)
  {
    for (uint32_t i = 0 ; i < imWidth ; ++i)
    {
      Temp_B = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 0] , pRgb_curr[3*j * imWidth + 3*i + 0]);
      Temp_G = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 1] , pRgb_curr[3*j * imWidth + 3*i + 1]);
      Temp_R = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 2] , pRgb_curr[3*j * imWidth + 3*i + 2]);
      Temp_BGR_Sum = Temp_R + Temp_G + Temp_B;
      pY_RgbtoYBinarized[j * imWidth + i] = Temp_BGR_Sum >= RgbThreshold ? WHITESPOT : BLACKSPOT;
    }
  }

  uint16_t nLeftRemoveColumn  = 5;     
  uint16_t nRightRemoveColumn = 5;     
  uint16_t nTopRemoveLine     = 5;     
  uint16_t nBottomRemoveLine  = 5;     

  //上5行
  memset(pY_RgbtoYBinarized, 0, nTopRemoveLine*imWidth*sizeof(uint8_t)); 
  //下5行
  memset(pY_RgbtoYBinarized + (imHeight - nBottomRemoveLine)*imWidth, 0, nBottomRemoveLine*imWidth*sizeof(uint8_t));

  for (uint16_t i = 1; i < imHeight; i++)
  {
    //左右各5列
    memset( pY_RgbtoYBinarized+imWidth*i + imWidth-nRightRemoveColumn, 0, nLeftRemoveColumn+nRightRemoveColumn );
  }

  ROK();
}
/****************************
*       binarizeRgbtoY();
* 1.在标定矩形框内差分图像
* 2.图像上下两部分阈值不同
* 3.可选择标定矩形往外扩张expand个像素
* 4.可选择标定矩形往外扩张0.5倍。
***************************/

ErrVal 
CPersonDetect::binarizeRgbtoY( CFrameContainer* pFrame_RgbtoYBinarized_inout , 
                              const CFrameContainer* const pFrame_curr_in,  
                              const CFrameContainer*  const pFrame_bkgnd_in, 
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_inout,
                              uint16_t Large_RgbThreshold ,
                              uint16_t Small_RgbThreshold)

{
  ASSERT( pFrame_curr_in );
  ASSERT( pFrame_bkgnd_in );
  ASSERT( pFrame_RgbtoYBinarized_inout );
  ASSERT( objDList_inout );

  uint32_t blocknum = objDList_inout->GetCount();
  if (0 == blocknum)
    return 0; //return ?

  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();

  const uint8_t* const pRgb_bkgnd = pFrame_bkgnd_in->m_BmpBuffer;
  const uint8_t* const pRgb_curr  = pFrame_curr_in->m_BmpBuffer;
  uint8_t* pY_RgbtoYBinarized = pFrame_RgbtoYBinarized_inout->m_YuvPlane[0];

  const uint8_t expand_hor = 2; 
  const uint8_t expand_ver = 10; 

  int16_t Temp_R, Temp_G, Temp_B, Temp_BGR_Sum;
  uint32_t i,j,k;
  int16_t top_x=0,top_y=0,bottom_x=0,bottom_y=0;//连通区域外接矩形左上、右下坐标。
  uint32_t sumDot = 0;

  for ( k = 1; k <= blocknum; k++)
  {
    sumDot += objDList_inout->GetAt(k)->m_nXYDotPlus[2];

    //外接矩形往外扩展expand_hor/expand_ver个像素
    top_x     =   MAX( 0 , objDList_inout->GetAt(k)->m_nOuterRect[0] - expand_hor);
    top_y     =   MAX( 0 , objDList_inout->GetAt(k)->m_nOuterRect[1] - expand_ver);
    bottom_x  =   MIN( imWidth  , top_x + objDList_inout->GetAt(k)->m_nOuterRect[2] + 2*expand_hor);
    bottom_y  =   MIN( imHeight , top_y + objDList_inout->GetAt(k)->m_nOuterRect[3] + 2*expand_ver);

    //==外接矩形往外扩展0.5倍。

    //     top_x     =   MAX( 0, objDList_inout->GetAt(k)->m_nOuterRect[0] - objDList_inout->GetAt(k)->m_nOuterRect[2]*0.5);
    //     top_y     =   MAX( 0, objDList_inout->GetAt(k)->m_nOuterRect[1] - objDList_inout->GetAt(k)->m_nOuterRect[3]*0.5);
    //     bottom_x  =   MIN( imWidth ,top_x+objDList_inout->GetAt(k)->m_nOuterRect[2] + objDList_inout->GetAt(k)->m_nOuterRect[2]);
    //     bottom_y  =   MIN( imHeight,top_y+objDList_inout->GetAt(k)->m_nOuterRect[3] + objDList_inout->GetAt(k)->m_nOuterRect[3]);


    if(bottom_y > 180)//下部RgbThreshold
    {
      for( j = top_y; j < (uint32_t)bottom_y; j++)
      {
        for( i = top_x; i < (uint32_t)bottom_x; i++)
        {
          Temp_B = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 0] , pRgb_curr[3*j * imWidth + 3*i + 0]);
          Temp_G = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 1] , pRgb_curr[3*j * imWidth + 3*i + 1]);
          Temp_R = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 2] , pRgb_curr[3*j * imWidth + 3*i + 2]);
          Temp_BGR_Sum = Temp_R + Temp_G + Temp_B;
          pY_RgbtoYBinarized[j * imWidth + i] = Temp_BGR_Sum >= Large_RgbThreshold ? WHITESPOT : BLACKSPOT;
        }
      }
    }
    else
    {
      for( j=top_y; j<(uint32_t)bottom_y; j++)
      {
        for( i=top_x; i<(uint32_t)bottom_x; i++)
        {
          Temp_B = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 0] , pRgb_curr[3*j * imWidth + 3*i + 0]);
          Temp_G = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 1] , pRgb_curr[3*j * imWidth + 3*i + 1]);
          Temp_R = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 2] , pRgb_curr[3*j * imWidth + 3*i + 2]);
          Temp_BGR_Sum = Temp_R + Temp_G + Temp_B;
          pY_RgbtoYBinarized[j * imWidth + i] = Temp_BGR_Sum >= Small_RgbThreshold ? WHITESPOT : BLACKSPOT;

        }
      }
    }
  }
  if (sumDot >(uint32_t)(imWidth*imHeight*WHITERATIO/100))
  {
    m_nRefreshFrameNum = 0;
  }
  ROK();
}


/*****************************
* 1.从binarizeRgbtoY扩展而来
* 2.在小分辨率差分并标定后，用binRgbtoY_LowtoHigh在大分辨率图像中做差分
* 
*
* 注:
* 3.objDList_inout是在小分辨率处理时标定生成的目标信息。
* 4.将objDList_inout中的标定矩形差值扩大后，在做差分。
*****************************/
ErrVal
CPersonDetect::binRgbtoY_LowtoHigh( CFrameContainer* pFrame_RgbtoYBinarized_inout , 
                                   const CFrameContainer* const pFrame_curr_in, //标定后，得到分块区域外接矩形后，求二值图像。  
                                   const CFrameContainer*  const pFrame_bkgnd_in, //根据矩形框的上下位置不同，区分远景和近景，设定不同阈值，远景阈值低，近景阈值高。
                                   const CDList< CObjLabeled*, CPointerDNode >* objDList_inout,
                                   uint16_t Large_RgbThreshold ,
                                   uint16_t Small_RgbThreshold,
                                   const uint16_t demarcation_line)//将矩形框的范围放大，进行小阈值差分。

{
  ASSERT( pFrame_curr_in );
  ASSERT( pFrame_bkgnd_in );
  ASSERT( pFrame_RgbtoYBinarized_inout );
  ASSERT( objDList_inout );

  uint32_t blocknum = objDList_inout->GetCount();
  if (0==blocknum)
    return 0; //return ?

  const uint8_t* const pRgb_bkgnd = pFrame_bkgnd_in->m_BmpBuffer;
  const uint8_t* const pRgb_curr = pFrame_curr_in->m_BmpBuffer;
  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();
  uint8_t* pY_RgbtoYBinarized = pFrame_RgbtoYBinarized_inout->m_YuvPlane[0];

  /* !!!!  删除上一帧pFrame_RgbtoYBinarized_inout中的信息，防止二值化图像的累积效应。*/
  memset(pY_RgbtoYBinarized,0,pFrame_RgbtoYBinarized_inout->getYSize()); 

  const uint8_t expand_hor = 2; // 外接矩形框水平方向，往外扩大expand_hor*2个像素。
  const uint8_t expand_ver = 5; // 外接矩形框竖直方向，往外扩大expand_ver*2个像素。

  int16_t Temp_R, Temp_G, Temp_B, Temp_BGR_Sum;
  uint32_t i,j,k;
  int16_t top_x=0,top_y=0,bottom_x=0,bottom_y=0;
  uint32_t sumDot = 0;//连通区域外接矩形左上、右下坐标。

  int16_t temp = 0;

  for ( k = 1; k <= blocknum; k++)
  {
    sumDot += objDList_inout->GetAt(k)->m_nXYDotPlus[2];

    //外接矩形往外扩展expand_hor/expand_ver个像素
    top_x     =   MAX( 0 , objDList_inout->GetAt(k)->m_nOuterRect[0]*2 - expand_hor);
    top_y     =   MAX( 0 , objDList_inout->GetAt(k)->m_nOuterRect[1]*2 - expand_ver);
    bottom_x  =   MIN( imWidth  , top_x + objDList_inout->GetAt(k)->m_nOuterRect[2]*2 + 2*expand_hor);
    bottom_y  =   MIN( imHeight , top_y + objDList_inout->GetAt(k)->m_nOuterRect[3]*2 + 2*expand_ver);

    //==外接矩形往外扩展0.5倍。

    //     top_x     =   MAX( 0, objDList_inout->GetAt(k)->m_nOuterRect[0]*2 - objDList_inout->GetAt(k)->m_nOuterRect[2]*2*0.5);
    //     top_y     =   MAX( 0, objDList_inout->GetAt(k)->m_nOuterRect[1]*2 - objDList_inout->GetAt(k)->m_nOuterRect[3]*2*0.5);
    //     bottom_x  =   MIN( imWidth ,top_x+objDList_inout->GetAt(k)->m_nOuterRect[2]*2 + objDList_inout->GetAt(k)->m_nOuterRect[2]*2);
    //     bottom_y  =   MIN( imHeight,top_y+objDList_inout->GetAt(k)->m_nOuterRect[3]*2 + objDList_inout->GetAt(k)->m_nOuterRect[3]*2);


    if(bottom_y > demarcation_line)//下部Large_RgbThreshold
    {
      for( j = top_y; j < (uint32_t)bottom_y; j++)
      {
        for( i = top_x; i < (uint32_t)bottom_x; i++)
        {
          Temp_B = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 0] , pRgb_curr[3*j * imWidth + 3*i + 0]);
          Temp_G = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 1] , pRgb_curr[3*j * imWidth + 3*i + 1]);
          Temp_R = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 2] , pRgb_curr[3*j * imWidth + 3*i + 2]);
          Temp_BGR_Sum = Temp_R + Temp_G+ Temp_B;
          pY_RgbtoYBinarized[j * imWidth + i] = Temp_BGR_Sum >= Large_RgbThreshold ? WHITESPOT : BLACKSPOT;

        }
      }
    }
    else//上部Small_RgbThreshold
    {
      for( j=top_y; j<(uint32_t)bottom_y; j++)
      {
        for( i=top_x; i<(uint32_t)bottom_x; i++)
        {
          Temp_B = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 0] , pRgb_curr[3*j * imWidth + 3*i + 0]);
          Temp_G = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 1] , pRgb_curr[3*j * imWidth + 3*i + 1]);
          Temp_R = ABS(pRgb_bkgnd[3*j * imWidth + 3*i + 2] , pRgb_curr[3*j * imWidth + 3*i + 2]);
          Temp_BGR_Sum = Temp_R + Temp_G+ Temp_B;
          pY_RgbtoYBinarized[j * imWidth + i] = Temp_BGR_Sum >= Small_RgbThreshold ? WHITESPOT : BLACKSPOT;
        }
      }
    }
  }
  if (sumDot >(uint32_t)(imWidth*imHeight*WHITERATIO/100))
  {
    m_nRefreshFrameNum = 0;
  }
  ROK();
}

ErrVal
CPersonDetect::UpdateBk(CFrameContainer* pFrame_BK_in_out,
                        const CFrameContainer* const pFrame_in,
                        const uint16_t bin_threshold, 
                        const uint16_t dot_rate_pre,        
                        const uint16_t dot_rate_connection,
                        const uint32_t framenum)
{
  ASSERT(pFrame_BK_in_out);
  ASSERT(pFrame_in);

  const uint16_t imWidth  = pFrame_in->getWidth();
  const uint16_t imHeight = pFrame_in->getHeight();
  int16_t Temp_R, Temp_G, Temp_B, Temp_BGR_Sum;
  uint32_t dot_sum = 0;
  int similar_count = 0;
  int16_t temp = 0;
  //   CFrameContainer* pframe_test = new CFrameContainer(imWidth,imHeight,pFrame_in->getYuvType());

  for (int i = 0; i < m_nbufsz - 1 ; ++i)
  {
    memcpy(m_nGetBKBuffer[i],m_nGetBKBuffer[i + 1],pFrame_BK_in_out->getRgbSize());
  }
  memcpy(m_nGetBKBuffer[m_nbufsz - 1],pFrame_in->m_BmpBuffer,pFrame_BK_in_out->getRgbSize());

  for (int j = 0; j < imHeight; ++j)
  {
    for (int i = 0 ; i < imWidth ; ++i)
    {
      Temp_B = ABS(pFrame_BK_in_out->m_BmpBuffer[3 * j * imWidth + 3 * i + 0] , m_nGetBKBuffer[m_nbufsz - 1][3 * j * imWidth + 3 * i + 0]);
      Temp_G = ABS(pFrame_BK_in_out->m_BmpBuffer[3 * j * imWidth + 3 * i + 1] , m_nGetBKBuffer[m_nbufsz - 1][3 * j * imWidth + 3 * i + 1]);
      Temp_R = ABS(pFrame_BK_in_out->m_BmpBuffer[3 * j * imWidth + 3 * i + 2] , m_nGetBKBuffer[m_nbufsz - 1][3 * j * imWidth + 3 * i + 2]);
      Temp_BGR_Sum = Temp_R + Temp_G + Temp_B;
      dot_sum += ((Temp_BGR_Sum >= bin_threshold) ? 1 : 0) ;
    }
  }
  if (dot_sum < pFrame_in->getRSize() * dot_rate_pre / 10000)
  {
    memcpy(pFrame_BK_in_out->m_BmpBuffer,m_nGetBKBuffer[m_nbufsz - 1],pFrame_in->getRgbSize());
    ROK();
  }

  dot_sum = 0;

  for (int k = 0; k < m_nbufsz - 1; ++k)
  {
    for (int j = 0; j < imHeight; ++j)
    {
      for (int i = 0 ; i < imWidth ; ++i)
      {
        Temp_B = ABS(m_nGetBKBuffer[k + 1][3 * j * imWidth + 3 * i + 0] , m_nGetBKBuffer[0][3 * j * imWidth + 3 * i + 0]);
        Temp_G = ABS(m_nGetBKBuffer[k + 1][3 * j * imWidth + 3 * i + 1] , m_nGetBKBuffer[0][3 * j * imWidth + 3 * i + 1]);
        Temp_R = ABS(m_nGetBKBuffer[k + 1][3 * j * imWidth + 3 * i + 2] , m_nGetBKBuffer[0][3 * j * imWidth + 3 * i + 2]);
        Temp_BGR_Sum = Temp_R + Temp_G + Temp_B;
        dot_sum += ((Temp_BGR_Sum >= bin_threshold) ? 1 : 0) ;
        //         pframe_test->m_YuvPlane[0][j * imWidth + i] = ((Temp_BGR_Sum >= bin_threshold) ? WHITESPOT : BLACKSPOT) ;
      }
    }
    if (dot_sum < pFrame_in->getRSize() * dot_rate_connection / 10000)
    {
      //       SaveJpeg_File_bin(pframe_test, framenum , dot_sum);
      ++similar_count;
      dot_sum = 0;
    }
    else
    {
      ROK();
    }
  }

  if ( m_nbufsz - 1 == similar_count)
  {
    memcpy(pFrame_BK_in_out->m_BmpBuffer,m_nGetBKBuffer[m_nbufsz - 1],pFrame_in->getRgbSize());
    for (int i = 0; i < m_nbufsz ; ++i)
    {
      memset(m_nGetBKBuffer[i],0,pFrame_BK_in_out->getRgbSize());
    }
  }
  // SAFEDELETE(pframe_test);

  ROK();
}


ErrVal  CPersonDetect::extract_public_part(CFrameContainer* pFrame_bin_data_out,  //输出
                                           const CFrameContainer* const pFrame_bin_bk,
                                           const CDList< CObjLabeled*, CPointerDNode >* objDList_bin_bk,
                                           const CDList< CObjLabeled*, CPointerDNode >* objDList_bin_pre)
{
  ASSERT(pFrame_bin_data_out);
  ASSERT(pFrame_bin_bk);
  ASSERT(objDList_bin_bk);
  ASSERT(objDList_bin_pre);

  const uint16_t obj_num_bk   = objDList_bin_bk->GetCount();
  const uint16_t obj_num_pre  = objDList_bin_pre->GetCount();
  if (0 == obj_num_bk || 0 == obj_num_pre)
  {
    ROK();
  }

  const uint16_t imwidth      = pFrame_bin_data_out->getWidth();
  const uint16_t imheight     = pFrame_bin_data_out->getHeight();

  uint16_t obj_bk_topx = 0,obj_bk_topy = 0,obj_bk_bottomx = 0,obj_bk_bottomy = 0;
  uint16_t obj_pre_topx = 0, obj_pre_topy = 0,obj_pre_bottomx = 0,obj_pre_bottomy = 0;
  uint16_t xdistance1 = 0, ydistance1 = 0,xdistance2 = 0, ydistance2 = 0;

  uint8_t* pbindataYbuf = pFrame_bin_data_out->m_YuvPlane[0];
  uint8_t* pbinbkYbuf   = pFrame_bin_bk->m_YuvPlane[0];

  for (int i = 1; i <= obj_num_pre; ++i)
  {
    obj_pre_topx      = objDList_bin_pre->GetAt(i)->m_nOuterRect[0];
    obj_pre_topy      = objDList_bin_pre->GetAt(i)->m_nOuterRect[1];
    obj_pre_bottomx   = objDList_bin_pre->GetAt(i)->m_nOuterRect[2] + objDList_bin_pre->GetAt(i)->m_nOuterRect[0];
    obj_pre_bottomy   = objDList_bin_pre->GetAt(i)->m_nOuterRect[3] + objDList_bin_pre->GetAt(i)->m_nOuterRect[1];

    for (int j = 1; j <= obj_num_bk; ++j)
    {
      obj_bk_topx      = objDList_bin_bk->GetAt(j)->m_nOuterRect[0];
      obj_bk_topy      = objDList_bin_bk->GetAt(j)->m_nOuterRect[1];
      obj_bk_bottomx   = objDList_bin_bk->GetAt(j)->m_nOuterRect[2] + objDList_bin_bk->GetAt(j)->m_nOuterRect[0];
      obj_bk_bottomy   = objDList_bin_bk->GetAt(j)->m_nOuterRect[3] + objDList_bin_bk->GetAt(j)->m_nOuterRect[1];

      xdistance1 = (obj_pre_topx >= obj_bk_topx) ? (obj_pre_topx - obj_bk_topx) : (obj_bk_topx - obj_pre_topx);
      xdistance2 = (obj_pre_bottomx >= obj_bk_bottomx) ? (obj_pre_bottomx - obj_bk_bottomx) : (obj_bk_bottomx - obj_pre_bottomx);
      ydistance1 = (obj_pre_topy >= obj_bk_topy) ? (obj_pre_topy - obj_bk_topy) : (obj_bk_topy - obj_pre_topy);
      ydistance2 = (obj_pre_bottomy >= obj_bk_bottomy) ? (obj_pre_bottomy - obj_bk_bottomy) : (obj_bk_bottomy - obj_pre_bottomy);

      if ((xdistance1 + xdistance2 < objDList_bin_pre->GetAt(i)->m_nOuterRect[2] + objDList_bin_bk->GetAt(j)->m_nOuterRect[2])
        &&(ydistance1 + ydistance2 < objDList_bin_pre->GetAt(i)->m_nOuterRect[3] + objDList_bin_bk->GetAt(j)->m_nOuterRect[3]))
      {
        for (int m = obj_bk_topy ; m < obj_bk_bottomy; ++m)
        {
          memcpy(&pbindataYbuf[m * imwidth + obj_bk_topx],&pbinbkYbuf[m * imwidth + obj_bk_topx],objDList_bin_bk->GetAt(j)->m_nOuterRect[2]);
        }
      }
    }
  }

  ROK();
}

ErrVal
CPersonDetect::erodeY(const CFrameContainer* const pFrame_inout, 
                      uint32_t nErodeTimesPerPixel_in ,
                      uint16_t StartLineNum) const
{
  ASSERT( pFrame_inout );

  int m, n, i = 0, j = 0;
  uint32_t k=0;

  const uint16_t imWidth = pFrame_inout->getWidth();
  const uint16_t imHeight = pFrame_inout->getHeight();

  if (StartLineNum >= imHeight || StartLineNum < 1)
    return 0;//return ?

  uint8_t mpiex, mpiex1;
  bool flag = FALSE;
  uint8_t *pData = new uint8_t[imWidth * imHeight];
  ASSERT( pData );

  for( k = 0; k < nErodeTimesPerPixel_in; ++k )
  {
    memcpy( pData , pFrame_inout->m_YuvPlane[0], imWidth * imHeight * sizeof(pData[0]));
    for( i = StartLineNum; i < imHeight-1; ++i )
    {
      for( j = 1; j < imWidth-1; ++j )
      {
        mpiex = pData[i * imWidth + j];
        if ( WHITESPOT == mpiex )
        {
          for (m=-1;m<2;m++)
          {
            for (n=-1;n<2;n++)
            {
              mpiex1=pData[( i + m ) * imWidth + ( j + n )];
              if( BLACKSPOT == mpiex1 )
              {
                pFrame_inout->m_YuvPlane[0][i * imWidth + j] = BLACKSPOT;
                flag = TRUE;
                break;
              }
            }
            if(flag)
            {
              flag = FALSE;
              break;
            }
          }
        }
      }
    }  
  }

  if(pData)
  {
    delete [] pData;
    pData = NULL;
  }

  ROK();
}

void
CPersonDetect::erodeY( CFrameContainer*  const pYuvframe, 
                      const uint8_t nMatrixWidth,
                      const uint8_t nVotedOne_in,
                      uint16_t StartLineNum,
                      const uint8_t nErodeStep)const
{
  int m,n,i=0,j=0,k=0;
  uint16_t imWidth  = pYuvframe->getWidth();//图像的宽
  uint16_t imHeight = pYuvframe->getHeight();//图像高

  bool flag = false;
  uint8_t *pData;
  int temp_num = 0;
  uint16_t erode_start_line = (StartLineNum > (nMatrixWidth - 1) / 2) ? StartLineNum : (nMatrixWidth - 1) / 2 ;
  pData = new uint8_t[imWidth * imHeight];


  for(k = 0; k < nErodeStep; ++k)
  {
    memcpy(pData, pYuvframe->m_YuvPlane[0], imWidth * imHeight * sizeof(uint8_t));

    for( i = erode_start_line; i < imHeight - (nMatrixWidth - 1) / 2; ++i)
    {
      for(j = (nMatrixWidth - 1) / 2; j < imWidth - (nMatrixWidth - 1)/ 2; ++j)
      {
        if (WHITESPOT == pData[i * imWidth + j])
        {
          for (m = -1 * (nMatrixWidth - 1) / 2; m <= (nMatrixWidth - 1) / 2;m++)
          {
            for (n = -1 * (nMatrixWidth - 1) / 2; n <= (nMatrixWidth - 1) / 2;n++)
            {
              if (WHITESPOT == pData[(i + m) * imWidth + (j + n)])
              {
                ++temp_num;
              }
              if (temp_num >= nVotedOne_in)
              {
                pYuvframe->m_YuvPlane[0][i * imWidth + j] = WHITESPOT;
                flag = true;
                break;
              }
            }
            if(flag)
            {
              flag = false;
              break;
            }
          }
          if (temp_num < nVotedOne_in)
          {
            pYuvframe->m_YuvPlane[0][i * imWidth + j] = BLACKSPOT;
          }
        }
        temp_num = 0;
      }
    }  
  }
  if(pData)
  {
    delete [] pData;
    pData = NULL;
  }
}

///////////////////////////////////////////////////////////////////////
/// \brief 
///     detect active object
/// \return
///     OK for success
///////////////////////////////////////////////////////////////////////
//todo: clear and optimize the following function
ErrVal
CPersonDetect::dilateY(const CFrameContainer* const pFrame_in, uint32_t nDilateTimesPerPixel_in,uint16_t NoDilateLineNum ) const
{
  ASSERT( pFrame_in );

  const uint16_t imWidth = pFrame_in->getWidth();
  const uint16_t imHeight = pFrame_in->getHeight();

  int m, n, i = 0,j = 0;
  uint32_t k = 0;
  if (NoDilateLineNum >= imHeight || NoDilateLineNum < 1)
    return 0;

  const uint16_t width  = pFrame_in->getWidth();
  const uint16_t height = pFrame_in->getHeight();

  uint8_t mpiex,mpiex1;
  bool flag = FALSE;
  uint8_t *pData = new uint8_t[width*height];
  ASSERT( pData );

  for( k = 0; k < nDilateTimesPerPixel_in; ++k )
  {
    memcpy(pData,pFrame_in->m_YuvPlane[0],width*height*sizeof(pData[0]));
    for( i = NoDilateLineNum ; i < height-1; ++i )
    {
      for( j = 1; j < width-1; ++j )
      {
        mpiex=pData[i * width + j];
        if( BLACKSPOT == mpiex )
        {
          for( m = -1; m < 2; ++m )
          {
            for( n = -1; n < 2; ++n )
            {
              mpiex1 = pData[(i + m) * width + ( j + n )];
              if( WHITESPOT == mpiex1 )
              {
                pFrame_in->m_YuvPlane[0][i*width+j] = WHITESPOT;
                flag = TRUE;
                break;
              }
            }
            if(flag)
            {
              flag = FALSE;
              break;
            }
          }
        }
      }
    }  
  }

  if(pData)
  {
    delete [] pData;
    pData = NULL;
  }

  ROK();
}


void CPersonDetect::RGBmulY( CFrameContainer* pFrame_input , const CFrameContainer* const pFrame_matlabFunced )
{
  ASSERT(pFrame_input);
  ASSERT(pFrame_matlabFunced);
  uint32_t i;
  const uint32_t imYByteSize = pFrame_matlabFunced->getYSize();
  for ( i = 0; i < imYByteSize; i++)
  {
    if ( BLACKSPOT == pFrame_matlabFunced->m_YuvPlane[0][i])
    {
      memset(&pFrame_input->m_BmpBuffer[3 * i + 0] , 0 , 3*sizeof(uint8_t));
    }
  }
}

void CPersonDetect::binremovebody_list(CFrameContainer* pFrame_matlabFunced,
                                       const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout)
{
  ASSERT(pFrame_matlabFunced);
  ASSERT(objDList_inout);

  uint32_t blocknum = objDList_inout->GetCount();
  if (0 == blocknum)
  {
    memset(pFrame_binremovebody->m_YuvPlane[0],1,pFrame_matlabFunced->getYSize());
    return ; 
  }

  const uint16_t imWidth      = pFrame_matlabFunced->getWidth();
  const uint16_t imHeight     = pFrame_matlabFunced->getHeight();
  const uint32_t imYByteSize  = pFrame_matlabFunced->getYSize();

  const uint8_t expandhor = 0; // 外接矩形框的长宽，往外扩大expandhor*2个像素。
  const uint8_t expandver = 0; // 外接矩形框的长宽，往外扩大expandver*2个像素。

  uint32_t i = 0 , j = 0, k = 0;
  uint32_t top_x=0, top_y = 0, bottom_x = 0, bottom_y = 0;//连通区域外接矩形左上、右下坐标。
  memset(pFrame_binremovebody->m_YuvPlane[0],1,imYByteSize);
  for (k = 1; k <= blocknum; k++)
  {
    top_x = objDList_inout->GetAt(k)->m_nOuterRect[0];
    top_y = objDList_inout->GetAt(k)->m_nOuterRect[1] ;
    bottom_x = top_x + objDList_inout->GetAt(k)->m_nOuterRect[2] ;
    bottom_y = top_y + objDList_inout->GetAt(k)->m_nOuterRect[3] ;

    for( j = top_y; j < bottom_y; j++)
    {
      memset(&pFrame_binremovebody->m_YuvPlane[0][j*imWidth + top_x] , 0 ,objDList_inout->GetAt(k)->m_nOuterRect[2]);
    }
  }
}

void  CPersonDetect::deletminobj(CDList< CObjLabeled*, CPointerDNode >* objDList_inout,const uint16_t demarcation_line)
{
  ASSERT(objDList_inout);
  uint32_t blocknum = objDList_inout->GetCount();
  if (0 == blocknum)
    return; //return ?
  uint32_t i = 0 , j = 0, k = 0;

  for (k = 1; k <= (uint32_t)objDList_inout->GetCount(); )
  {
    if (objDList_inout->GetAt(k)->m_nOuterRect[1] + objDList_inout->GetAt(k)->m_nOuterRect[3] > demarcation_line   \
      &&(objDList_inout->GetAt(k)->m_nOuterRect[2] < PARABOLAWIDTHNEAR                                             \
      ||objDList_inout->GetAt(k)->m_nOuterRect[3] < PARABOLAHEIGHTNEAR))                                   
    {
      objDList_inout->DestroyAtPos(k);
    }
    else if (objDList_inout->GetAt(k)->m_nOuterRect[1] + objDList_inout->GetAt(k)->m_nOuterRect[3] <= demarcation_line    
      && (objDList_inout->GetAt(k)->m_nOuterRect[2] < PARABOLAWIDTHFAR                                                  \
      || objDList_inout->GetAt(k)->m_nOuterRect[3] < PARABOLAHEIGHTFAR))
    {
      objDList_inout->DestroyAtPos(k);
    }
    else
    {
      k++;
    }
  }
}
void CPersonDetect::Calculate_BGR_Mean( uint8_t& b_mean,uint8_t& g_mean,uint8_t& r_mean,
                                       const  CFrameContainer* const pFrame_curr_in,
                                       const CFrameContainer* const pFrame_bin_in,
                                       uint16_t top_x,uint16_t top_y,uint16_t bottom_x,uint16_t bottom_y)         
{
  ASSERT( pFrame_curr_in );
  ASSERT( pFrame_bin_in );

  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();


  CFrameContainer* pFrame_bin_in_temp = new CFrameContainer;
  *pFrame_bin_in_temp = *pFrame_bin_in;
  //   erodeY( pFrame_bin_in_temp, 2 ); // !!!!!!!

  uint8_t*  prgb_Objectblock = pFrame_curr_in->m_BmpBuffer;//存放平滑图像的BmpBuffer
  uint8_t*  pY_Objectblock   = pFrame_bin_in_temp->m_YuvPlane[0];
  uint32_t  b_sum = 0, g_sum = 0, r_sum = 0, ptnum = 0;

  int i,j;

  for( j=top_y; j<bottom_y; j++)
  {
    for( i=top_x; i<bottom_x; i++)
    {
      if (WHITESPOT == pY_Objectblock[j*imWidth+i])
      {
        b_sum += prgb_Objectblock[3*j*imWidth + 3*i +0];
        g_sum += prgb_Objectblock[3*j*imWidth + 3*i +1];
        r_sum += prgb_Objectblock[3*j*imWidth + 3*i +2];
        ptnum++;
      }
    }
  }

  if (0 == ptnum )
  {
    b_mean = g_mean = r_mean = 0;
  }
  else
  {
    b_mean = (b_sum/ptnum);
    g_mean = (g_sum/ptnum);
    r_mean = (r_sum/ptnum);
  }

  if (pFrame_bin_in_temp)
  {
    delete pFrame_bin_in_temp;
    pFrame_bin_in_temp = NULL;
  }
}


//
//ErrVal
//CPersonDetect::PersenDetect_Process1( CFrameContainer* pFrame_matlabFunced, 
//                                     CFrameContainer* pRgbhumaninfo,
//                                     CFrameContainer* Binremovebody, 
//                                     const CFrameContainer*  const  pFrame_curr_in)
//{
//
//  if ( 0 == CurFrameNum%SAMPLING_INTERVAL) //隔帧抽样
//  {
//    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
//    if( 0 == m_nRefreshFrameNum )
//    {
//      SAFEDELETE(m_GraspFramesBuffer);
//      initBkdet(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
//      *pFrame_bkgndDetected = *pFrame_RgbSmoothed;
//      EXM_NOK( generateRgbBkgnd_fromRgbFrames(  pFrame_bkgndDetected,pFrame_RgbSmoothed,pFrame_matlabFunced ), "matlabFunc fail!" );
//    }
//    else
//    {
//      EXM_NOK( generateRgbBkgnd_fromRgbFrames(pFrame_bkgndDetected ,pFrame_RgbSmoothed ,pFrame_matlabFunced ), "matlabFunc fail!" );
//      EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced ,pFrame_RgbSmoothed,pFrame_bkgndDetected ,MAXTHRESHOLD ), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
//      erodeY( pFrame_matlabFunced, 1 );
//      EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
//      binarizeRgbtoY(pFrame_matlabFunced,pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList,MAXTHRESHOLD,MINTHRESHOLD );//pFramesBuffer->getFrame(1)
//      ObjectLabeledDList->DestroyAll();
//
//      *pRgbhumaninfo = *pFrame_curr_in;
//      RGBmulY( pRgbhumaninfo , pFrame_matlabFunced);
//
//      EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
//      binremovebody(pFrame_matlabFunced,ObjectLabeledDList);
//      *Binremovebody = *pFrame_binremovebody;
//      Binremovebody->setChromaTo128();
//      Binremovebody->cvtY1toY255();
//      Binremovebody->updateRGB24FromYUV444();
//
//      deletminobj(ObjectLabeledDList);
//      ObjectLabeledDList->DestroyAll();
//
//    }
//  }
//  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
//  ROK();
//}
//
//
///*******************************************
//*             PersenDetect_Process2()
//*
//*     为适应D1处理，先隔行、跟列采集做第一次差分，
//*     再按位置返回D1做第二次差分。
//*
//********************************************/
//ErrVal
//CPersonDetect::PersenDetect_Process2(CFrameContainer* pFrame_matlabFunced,       
//                                     CFrameContainer* pRgbhumaninfo,
//                                     CFrameContainer* Binremovebody,
//                                     CFrameContainer* pFrame_curr_in,
//                                     const uint8_t* const pRGB_template )  
//
//
//{
//  if ( 0 == CurFrameNum%SAMPLING_INTERVAL) //隔帧抽样
//  {
//    Shadow_Mask(pFrame_curr_in,pRGB_template);
//    *pFrame_RgbSmoothed = *pFrame_curr_in;
//
//    CFrameContainer*   pFrame_input_low         = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer*   pFrame_matlabFunced_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//
//    if( 0 == m_nRefreshFrameNum )
//    {
//      SAFEDELETE(m_GraspFramesBuffer);
//      initBkdet(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
//      *pFrame_bkgndDetected = *pFrame_curr_in;
//      m_nRefreshFrameNum++;
//    }
//    else
//    {
//      Interlaced_Scanning(pFrame_input_low,pFrame_curr_in);
//      Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
//      Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);
//
//      /*低分辨率图像中差分*/
//      EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
//      erodeY( pFrame_matlabFunced_low, 1);
//      EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
//      binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,45);//pFramesBuffer->getFrame(1)
//      ObjectLabeledDList->DestroyAll();
////       dilateY( pFrame_matlabFunced, 2 );
//
//             *pRgbhumaninfo = *pFrame_curr_in;
//             RGBmulY( pRgbhumaninfo , pFrame_matlabFunced);
//
//      EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
//      deletminobj(ObjectLabeledDList);
//             binremovebody_list(pFrame_matlabFunced,ObjectLabeledDList);
//             *Binremovebody = *pFrame_binremovebody;
//             Binremovebody->setChromaTo128();
//             Binremovebody->cvtY1toY255();
//             Binremovebody->updateRGB24FromYUV444();
////       memset(pRgbhumaninfo->m_BmpBuffer,0,pRgbhumaninfo->getRgbSize());
//      ForecastObjectDetect(ObjectLabeledDList, pRgbhumaninfo, pFrame_matlabFunced,&Warning_Line[0],&Alarm_Line[0]);
//      Draw_Warning_Line(&Warning_Line[0],pRgbhumaninfo);    
//      ObjectLabeledDList->DestroyAll();
//      *pFrame_bkgndDetected = *pFrame_curr_in;
//    }
//
//    SAFEDELETE( pFrame_input_low);
//    SAFEDELETE( pFrame_matlabFunced_low);
//    SAFEDELETE( pFrame_RgbSmoothed_low);
//    SAFEDELETE( pFrame_bkgndDetected_low);
//
//  }
//  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
//  ROK();
//}
//
///*大门口区分人与车*/
//
//ErrVal
//CPersonDetect::PersenDetect_Process3(CFrameContainer* pFrame_matlabFunced,       
//                                     CFrameContainer* pRgbhumaninfo,
//                                     CFrameContainer* Binremovebody,
//                                     CFrameContainer* pFrame_curr_in,
//                                     const uint8_t* const pRGB_template )  
//
//{
//
//  if ( 0 == CurFrameNum%SAMPLING_INTERVAL) //隔帧抽样
//  {
//    Shadow_Mask(pFrame_curr_in,pRGB_template);
//    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
//    //    *pFrame_RgbSmoothed = *pFrame_curr_in;
//    CFrameContainer* pframe_bin_bk_low    = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer* pframe_bin_pre_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer* pframe_bin_data_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer* pframe_pre_low       = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//
//    CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//    CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//
//    if( 0 == m_nRefreshFrameNum )
//    {
//      SAFEDELETE(m_GraspFramesBuffer);
//      initBkdet(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
//      EXM_NOK( generateRgbBkgnd_fromRgbFrames(  pFrame_bkgndDetected,pFrame_RgbSmoothed,pFrame_matlabFunced ), "matlabFunc fail!" );
//      *pFrame_bkgndDetected = *pFrame_RgbSmoothed;
//      *pFrame_previous = *pFrame_RgbSmoothed;
//    }
//    else
//    {
//      EXM_NOK( generateRgbBkgnd_fromRgbFrames(pFrame_bkgndDetected ,pFrame_RgbSmoothed ,pFrame_matlabFunced ), "matlabFunc fail!" );
//
//      Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
//      Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);
//      Interlaced_Scanning(pframe_pre_low,pFrame_previous);
//
//      /*低分辨率图像中差分*/
//      EXM_NOK( binarizeY_fromRgbBkgnd(pframe_bin_bk_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
//      EXM_NOK( binarizeY_fromRgbBkgnd(pframe_bin_pre_low ,pFrame_RgbSmoothed_low,pframe_pre_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
//
//      //       pframe_bin_pre_low->setChromaTo128();
//      //       pframe_bin_pre_low->cvtY1toY255();
//      //       pframe_bin_pre_low->updateRGB24FromYUV444();
//
//      //       FILE *pFile1=fopen("D:\\_Debug\\3.txt","wb+");
//      //       fwrite(pframe_bin_pre_low->m_BmpBuffer,1,pframe_bin_pre_low->getRgbSize(),pFile1);
//      //       fclose(pFile1);
//
//      EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pframe_bin_bk_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
//      EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList1, pframe_bin_pre_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
//      extract_public_part(pframe_bin_data_low,pframe_bin_bk_low,ObjectLabeledDList,ObjectLabeledDList1);
//      ObjectLabeledDList->DestroyAll();
//      ObjectLabeledDList1->DestroyAll();
//
//      EXM_NOK( pMatlabFunc->labelObj(ObjectLabeledDList, pframe_bin_data_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
//      binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,45);//pFramesBuffer->getFrame(1)
//      ObjectLabeledDList->DestroyAll();
//
//      //       erodeY( pFrame_matlabFunced, 1);
//      //       dilateY( pFrame_matlabFunced, 3 );
//
//      memset(pRgbhumaninfo->m_BmpBuffer,0,pRgbhumaninfo->getRgbSize());
//
//      *pRgbhumaninfo = *pFrame_curr_in;
//      RGBmulY( pRgbhumaninfo , pFrame_matlabFunced);  
//
//      EXM_NOK( pMatlabFunc->labelObj(ObjectLabeledDList, pFrame_matlabFunced, pFrame_RgbSmoothed), "labelObject fail!" );
//      deletminobj(ObjectLabeledDList);
//      ForecastObjectDetect(ObjectLabeledDList, pRgbhumaninfo, pFrame_matlabFunced,&Warning_Line[0],&Alarm_Line[0]);
//      Draw_Warning_Line(&Warning_Line[0],pRgbhumaninfo);
//
//      binremovebody_list(pFrame_matlabFunced,ObjectLabeledDList);
//      *Binremovebody = *pFrame_binremovebody;
//      Binremovebody->setChromaTo128(); 
//      Binremovebody->cvtY1toY255(); 
//      Binremovebody->updateRGB24FromYUV444();
//
//      ObjectLabeledDList->DestroyAll();
//      *pFrame_previous = *pFrame_RgbSmoothed;
//    }
//
//    SAFEDELETE(pframe_bin_bk_low);
//    SAFEDELETE(pframe_bin_pre_low);
//    SAFEDELETE(pframe_bin_data_low);
//    SAFEDELETE( pframe_pre_low );
//
//    SAFEDELETE( pFrame_RgbSmoothed_low);
//    SAFEDELETE( pFrame_bkgndDetected_low);
//
//  }
//  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
//  ROK();
//}
//
// /*新背景算法*/
// ErrVal
// CPersonDetect::PersenDetect_Process4(CFrameContainer* pFrame_matlabFunced,
//                                      CFrameContainer* pRgbhumaninfo,
//                                      CFrameContainer* Binremovebody,
//                                      CFrameContainer* pFrame_curr_in,
//                                      const uint8_t* const pRGB_template,
//                                      const ALARMTYPE alarm_type,
//                                      uint16_t demarcation_line,
//                                      const uint32_t framenum )  
// 
// {
//   if(0 == CurFrameNum%BKUPDATE_INTERVAL)
//   {
//     Shadow_Mask(pFrame_curr_in,pRGB_template);
//     EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
//     //     *pFrame_RgbSmoothed = *pFrame_curr_in;  //!> 是否平滑  .
// 
//     if (0 == CurFrameNum)
//     {
//       clearUpdateBk();
//       initUpdateBk(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
//       *pFrame_bkgndDetected = *pFrame_RgbSmoothed/*pFrame_curr_in*/;
//     }
//     else
//     {
//       UpdateBk(pFrame_bkgndDetected,pFrame_RgbSmoothed,70,10,10,framenum);  /*2,3*/
//     }
//   }
// 
//   if ( 0 == CurFrameNum%SAMPLING_INTERVAL) 
//   {
//     if (0 != CurFrameNum%BKUPDATE_INTERVAL)
//     {
//       Shadow_Mask(pFrame_curr_in,pRGB_template);
//       EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
//       //           *pFrame_RgbSmoothed = *pFrame_curr_in;         //!>  是否平滑
//     }
// 
//     CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//     CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
//     CFrameContainer*   pFrame_matlabFunced_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
// 
//     Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
//     Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);
// 
//     /*低分辨率图像中差分*/
//     EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
//     //erodeY( pFrame_matlabFunced_low, 7,25,1);
//     EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
//     binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,45,demarcation_line);//pFramesBuffer->getFrame(1)
//     ObjectLabeledDList->DestroyAll();
//     erodeY( pFrame_matlabFunced, 7,25,1);
//     *pRgbhumaninfo = *pFrame_curr_in;
//     //     RGBmulY( pRgbhumaninfo , pFrame_matlabFunced);
// 
//     EXM_NOK( pMatlabFunc->labelObj(ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
//     deletminobj(ObjectLabeledDList , demarcation_line);
// 
//     ForecastObjectDetect(ObjectLabeledDList, pFrame_curr_in/*pRgbhumaninfo*/, pFrame_matlabFunced,&Warning_Line[0],&Alarm_Line[0], alarm_type);
// #if ( 1 == SY_DEBUG)
// 
//     binremovebody_list(pFrame_matlabFunced,ObjectLabeledDList);
//     *Binremovebody = *pFrame_binremovebody;
//     Binremovebody->setChromaTo128();
//     Binremovebody->cvtY1toY255();
//     Binremovebody->updateRGB24FromYUV444();
// 
//     Draw_Warning_Line(&Warning_Line[0],pFrame_curr_in/*pRgbhumaninfo*/);
//     ImgMoveObjectDetect(pFrame_curr_in/*pRgbhumaninfo*/);
//     if (b_First_Alarm /*|| b_Second_Alarm*/)
//     {
//       SaveJpeg_File_color(pFrame_curr_in/*pRgbhumaninfo*/);
//     }
// #endif
//     ObjectLabeledDList->DestroyAll();
//     memset(pFrame_matlabFunced->m_BmpBuffer, 0, pFrame_matlabFunced->getRgbSize());
//     SAFEDELETE( pFrame_RgbSmoothed_low);
//     SAFEDELETE( pFrame_bkgndDetected_low);
//     SAFEDELETE( pFrame_matlabFunced_low);
// 
//   }
//   CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
//   ROK();
// }



// *1~4通道*/
ErrVal
CPersonDetect::PersenDetect_Process_Channel_1(CFrameContainer* pFrame_matlabFunced,       
                                              CFrameContainer* pRgbhumaninfo,
                                              CFrameContainer* pFrame_curr_in,
                                              const uint8_t* const pRGB_template, 
                                              const ALARMTYPE alarm_type,
                                              uint16_t demarcation_line,
                                              const uint32_t framenum)
{
  if(0 == CurFrameNum%BKUPDATE_INTERVAL)
  {
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
    if (0 == CurFrameNum)
    {
      clearUpdateBk();
      initUpdateBk(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
      *pFrame_bkgndDetected = *pFrame_RgbSmoothed/*pFrame_curr_in*/;
    }
    else
    {
      UpdateBk(pFrame_bkgndDetected,pFrame_RgbSmoothed,70,8,8,framenum);  /*2,3*/
    }
  }
  if ( 0 == CurFrameNum%SAMPLING_INTERVAL)
  {
#if (1 == SY_DEBUG)
    *pRgbhumaninfo = *pFrame_curr_in;
    memset(pFrame_matlabFunced->m_YuvPlane[0], 0, pFrame_matlabFunced->getRSize());
#endif
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );

    CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_matlabFunced_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());

    Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
    Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);
    EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
    binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,45,demarcation_line);//pFramesBuffer->getFrame(1)
    ObjectLabeledDList->DestroyAll();
    erodeY( pFrame_matlabFunced, 7,18,5,1);
    dilateY( pFrame_matlabFunced, 1 );
    EXM_NOK( pMatlabFunc->labelObj(ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
    deletminobj(ObjectLabeledDList, demarcation_line);
    ForecastObjectDetect(ObjectLabeledDList, pFrame_curr_in/*pRgbhumaninfo*/, pFrame_matlabFunced,&Warning_Line[0],&Alarm_Line[0] , alarm_type);
    ObjectLabeledDList->DestroyAll();

#if (1 == SY_DEBUG)
    Draw_Warning_Line(&Warning_Line[0],pFrame_curr_in/*pRgbhumaninfo*/);
    ImgMoveObjectDetect(pFrame_curr_in/*pRgbhumaninfo*/);
    if (b_First_Alarm /*|| b_Second_Alarm*/)
    {
      SaveJpeg_File_color(pFrame_curr_in/*pRgbhumaninfo*/,1);
    }
#endif
    SAFEDELETE( pFrame_RgbSmoothed_low);
    SAFEDELETE( pFrame_bkgndDetected_low);
    SAFEDELETE( pFrame_matlabFunced_low);
  }
  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
  ROK();
}

ErrVal
CPersonDetect::PersenDetect_Process_Channel_2(CFrameContainer* pFrame_matlabFunced,       
                                              CFrameContainer* pRgbhumaninfo,
                                              CFrameContainer* pFrame_curr_in,
                                              const uint8_t* const pRGB_template,
                                              const ALARMTYPE alarm_type,
                                              uint16_t demarcation_line,
                                              const uint32_t framenum)

{
  if(0 == CurFrameNum%BKUPDATE_INTERVAL)
  {
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
    if (0 == CurFrameNum)
    {
      clearUpdateBk();
      initUpdateBk(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
      *pFrame_bkgndDetected = *pFrame_RgbSmoothed/*pFrame_curr_in*/;
    }
    else
    {
      UpdateBk(pFrame_bkgndDetected,pFrame_RgbSmoothed,70,2,2,framenum);  /*2,3*/
    }
  }
  if ( 0 == CurFrameNum%SAMPLING_INTERVAL) 
  {
#if (1 == SY_DEBUG)
    *pRgbhumaninfo = *pFrame_curr_in;
    memset(pFrame_matlabFunced->m_YuvPlane[0], 0, pFrame_matlabFunced->getRSize());
#endif
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );

    CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_matlabFunced_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());

    Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
    Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);
    EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
    binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,35,demarcation_line);//pFramesBuffer->getFrame(1)
    ObjectLabeledDList->DestroyAll();
    erodeY( pFrame_matlabFunced, 3,3,5,1);
    dilateY( pFrame_matlabFunced, 1 );
    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
    deletminobj(ObjectLabeledDList, demarcation_line);
    ForecastObjectDetect(ObjectLabeledDList, pFrame_curr_in/*pRgbhumaninfo*/, pFrame_matlabFunced,&Warning_Line[1],&Alarm_Line[1] ,alarm_type);
    ObjectLabeledDList->DestroyAll();

#if (1 == SY_DEBUG)
    Draw_Warning_Line(&Warning_Line[1],pFrame_curr_in/*pRgbhumaninfo*/);  
    ImgMoveObjectDetect(pFrame_curr_in/*pRgbhumaninfo*/);
    if (b_First_Alarm /*|| b_Second_Alarm*/)
    {
      SaveJpeg_File_color(pFrame_curr_in/*pRgbhumaninfo*/,2);
    }
#endif
    SAFEDELETE( pFrame_RgbSmoothed_low);
    SAFEDELETE( pFrame_bkgndDetected_low);
    SAFEDELETE( pFrame_matlabFunced_low);
  }
  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
  ROK();
}

ErrVal
CPersonDetect::PersenDetect_Process_Channel_3(CFrameContainer* pFrame_matlabFunced,       
                                              CFrameContainer* pRgbhumaninfo,
                                              CFrameContainer* pFrame_curr_in,
                                              const uint8_t* const pRGB_template,
                                              const ALARMTYPE alarm_type,
                                              uint16_t demarcation_line,
                                              const uint32_t framenum)


{
  if(0 == CurFrameNum%BKUPDATE_INTERVAL)
  {
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
    if (0 == CurFrameNum)
    {
      clearUpdateBk();
      initUpdateBk(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
      *pFrame_bkgndDetected = *pFrame_RgbSmoothed/*pFrame_curr_in*/;
    }
    else
    {
      UpdateBk(pFrame_bkgndDetected,pFrame_RgbSmoothed,70,5,5,framenum);  /*2,3*/
    }
  }
  if ( 0 == CurFrameNum%SAMPLING_INTERVAL) 
  {
#if (1 == SY_DEBUG)
    *pRgbhumaninfo = *pFrame_curr_in;
#endif
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );


    CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_matlabFunced_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());

    //隔行采样
    Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
    Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);

    //二值
    EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)

    //标定，顺便得到他们的RGB信息（暂时没用）
    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced_low, pFrame_RgbSmoothed_low), "labelObject fail!" );

    //根据标定信息对原图进行二值
    memset(pFrame_matlabFunced->m_YuvPlane[0], 0, pFrame_matlabFunced->getWidth()*pFrame_matlabFunced->getHeight());
    binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,45,demarcation_line);//pFramesBuffer->getFrame(1)

    SHOW_BIN_IMAGE("pFrame_matlabFunced", 
        pFrame_matlabFunced->getWidth(), 
        pFrame_matlabFunced->getHeight(), 
        (char*)pFrame_matlabFunced->m_YuvPlane[0]);

    ObjectLabeledDList->DestroyAll();
    erodeY( pFrame_matlabFunced, 3,3,5,1);  //腐蚀
    dilateY( pFrame_matlabFunced, 1 );      //膨胀

    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
    deletminobj(ObjectLabeledDList, demarcation_line);
    ForecastObjectDetect(ObjectLabeledDList, pFrame_curr_in/*pRgbhumaninfo*/, pFrame_matlabFunced,&Warning_Line[2],&Alarm_Line[2] , alarm_type);
    ObjectLabeledDList->DestroyAll();

    Draw_Warning_Line(&Warning_Line[2],pFrame_RgbSmoothed/*pRgbhumaninfo*/);    
    ImgMoveObjectDetect(pFrame_RgbSmoothed/*pRgbhumaninfo*/);
    SHOW_IMAGE("smooth", pFrame_RgbSmoothed->getImage());


#if (1 == SY_DEBUG)
    Draw_Warning_Line(&Warning_Line[2],pFrame_curr_in/*pRgbhumaninfo*/);    
    ImgMoveObjectDetect(pFrame_curr_in/*pRgbhumaninfo*/);
    if (b_First_Alarm /*|| b_Second_Alarm*/)
    {
//       SaveJpeg_File_color(pFrame_curr_in/*pRgbhumaninfo*/,3);
    }
#endif
    SAFEDELETE( pFrame_RgbSmoothed_low);
    SAFEDELETE( pFrame_bkgndDetected_low);
    SAFEDELETE( pFrame_matlabFunced_low);
  }
  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
  ROK();
}
ErrVal
CPersonDetect::PersenDetect_Process_Channel_4(CFrameContainer* pFrame_matlabFunced,       
                                              CFrameContainer* pRgbhumaninfo,
                                              CFrameContainer* pFrame_curr_in,
                                              const uint8_t* const pRGB_template,
                                              const ALARMTYPE alarm_type,
                                              uint16_t demarcation_line,
                                              const uint32_t framenum)
{
  if(0 == CurFrameNum%BKUPDATE_INTERVAL)
  {
    Shadow_Mask(pFrame_curr_in,pRGB_template);
    EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
    if (0 == CurFrameNum)
    {
      clearUpdateBk();
      initUpdateBk(pFrame_curr_in->getWidth(),pFrame_curr_in->getHeight(),pFrame_curr_in->getYuvType());
      *pFrame_bkgndDetected = *pFrame_RgbSmoothed/*pFrame_curr_in*/;
    }
    else
    {
      UpdateBk(pFrame_bkgndDetected,pFrame_RgbSmoothed,70,10,10,framenum);  /*2,3*/
    }
  }
  if ( 0 == CurFrameNum%SAMPLING_INTERVAL) 
  {
#if (1 == SY_DEBUG)
    *pRgbhumaninfo = *pFrame_curr_in;
#endif
    if (0 != CurFrameNum%BKUPDATE_INTERVAL)
    {
      Shadow_Mask(pFrame_curr_in,pRGB_template);
      EXM_NOK( averagesmoothRgb( pFrame_RgbSmoothed,pFrame_curr_in/*, SMOOTHSTARTLINE */), "smoothRgb fail!" );
    }
    CFrameContainer*   pFrame_RgbSmoothed_low   = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_bkgndDetected_low = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    CFrameContainer*   pFrame_matlabFunced_low  = new CFrameContainer(pFrame_curr_in->getWidth()/2,pFrame_curr_in->getHeight()/2,pFrame_curr_in->getYuvType());
    Interlaced_Scanning(pFrame_RgbSmoothed_low,pFrame_RgbSmoothed);
    Interlaced_Scanning(pFrame_bkgndDetected_low,pFrame_bkgndDetected);
    EXM_NOK( binarizeY_fromRgbBkgnd(pFrame_matlabFunced_low ,pFrame_RgbSmoothed_low,pFrame_bkgndDetected_low ,MAXTHRESHOLD), "pMatlabFunc fail!" );//pFramesBuffer->getFrame(1)
    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced_low, pFrame_RgbSmoothed_low), "labelObject fail!" );
    binRgbtoY_LowtoHigh( pFrame_matlabFunced, pFrame_RgbSmoothed,pFrame_bkgndDetected, ObjectLabeledDList, 70,45,demarcation_line);//pFramesBuffer->getFrame(1)
    ObjectLabeledDList->DestroyAll();
    erodeY( pFrame_matlabFunced, 7,30,1,1);
    EXM_NOK( pMatlabFunc->labelObj( ObjectLabeledDList, pFrame_matlabFunced ,pFrame_RgbSmoothed), "labelObject fail!" );
    deletminobj(ObjectLabeledDList, demarcation_line);
    ForecastObjectDetect(ObjectLabeledDList, pFrame_curr_in/*pRgbhumaninfo*/, pFrame_matlabFunced,&Warning_Line[3],&Alarm_Line[3] ,alarm_type);
    ObjectLabeledDList->DestroyAll();
#if (1 == SY_DEBUG)
    Draw_Warning_Line(&Warning_Line[3],pFrame_curr_in/*pRgbhumaninfo*/);
    ImgMoveObjectDetect(pFrame_curr_in/*pRgbhumaninfo*/);
    if (b_First_Alarm /*|| b_Second_Alarm*/)
    {
      SaveJpeg_File_color(pFrame_curr_in/*pRgbhumaninfo*/,4);
    }
#endif
    SAFEDELETE( pFrame_RgbSmoothed_low);
    SAFEDELETE( pFrame_bkgndDetected_low);
    SAFEDELETE( pFrame_matlabFunced_low);
  }
  CurFrameNum = ( CurFrameNum >= MAXFRAMENUM ) ? 1 : ++CurFrameNum;
  ROK();
}


// *多路处理主函数*/
ErrVal
CPersonDetect::PersenDetect_Process_Main(CFrameContainer* pFrame_matlabFunced,       
                                         CFrameContainer* pRgbhumaninfo,
                                         CFrameContainer* pFrame_input,
                                         const uint8_t* const pRGB_template,
                                         const ALARMTYPE alarm_type_channle_1,
                                         const ALARMTYPE alarm_type_channle_2,
                                         const ALARMTYPE alarm_type_channle_3,
                                         const ALARMTYPE alarm_type_channle_4,
                                         uint16_t demarcation_line_channel_1,
                                         uint16_t demarcation_line_channel_2,
                                         uint16_t demarcation_line_channel_3,
                                         uint16_t demarcation_line_channel_4,
                                         const uint32_t framenum)

{
  switch (m_nChannel_ID)
  {
  case 0 :
    PersenDetect_Process_Channel_1( pFrame_matlabFunced,       
      pRgbhumaninfo,
      pFrame_input,
      pRGB_template,
      alarm_type_channle_1,
      demarcation_line_channel_1,
      framenum);
    break;
  case 1 :
    PersenDetect_Process_Channel_2( pFrame_matlabFunced,       
      pRgbhumaninfo,
      pFrame_input,
      pRGB_template,
      alarm_type_channle_2,
      demarcation_line_channel_2,
      framenum);
    break;
  case 2 :
    PersenDetect_Process_Channel_3( pFrame_matlabFunced,       
      pRgbhumaninfo,
      pFrame_input,
      pRGB_template,
      alarm_type_channle_3,
      demarcation_line_channel_3,
      framenum);
    break;
  case 3 :
    PersenDetect_Process_Channel_4( pFrame_matlabFunced,       
      pRgbhumaninfo,
      pFrame_input,
      pRGB_template,
      alarm_type_channle_4,
      demarcation_line_channel_4,
      framenum);
    break;
  }
  ROK();

}
ErrVal
CPersonDetect::ForecastObjectDetect (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout ,
                                     CFrameContainer* pFrame_in,
                                     const CFrameContainer* const pFrame_bin_in,
                                     Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                                     Cordon_Par* Alarm_Line_Pra ,
                                     const ALARMTYPE alarm_type)       //! 报警线参数
{
  ASSERT(objDList_inout);
  ASSERT(pFrame_in);
  ASSERT(pFrame_bin_in);
  ASSERT(Warning_Line_Pra);
  ASSERT(Alarm_Line_Pra);

  if(objDList_inout->GetCount() > m_nTrackObjectMaxNum)
  {
    m_tracked_obj_flg = false ;
    memset(PreLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;
    memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
    m_TrackNum = 0 ;
    m_curr_frm_num = 0 ;
    ROK();
  }

  for (int i = 0; i < m_TrackNum; i++ )
  {
    TrackObject[i].b_Warning = TrackObject[i].b_Alarm = false;
  }
  Statistics_obj_curinfo(objDList_inout,pFrame_in,pFrame_bin_in);
  if(!m_tracked_obj_flg)
  {
    if (m_curr_frm_num == 0)
    {
      RERR();
    }
    //----跟踪的目标各项赋值
    memcpy(PreLabelInfo, ObjLabelInfo,sizeof(ObjLabelInfoStruct) * MIN(m_curr_frm_num , m_nTrackObjectMaxNum)) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;

    m_pre_frm_num     = m_curr_frm_num ;
    m_tracked_obj_flg = TRUE ;
    m_curr_frm_num    = 0 ;
    ROK();
  }
  else
  {
    if (m_curr_frm_num > m_nTrackObjectMaxNum || m_pre_frm_num > m_nTrackObjectMaxNum )
    {
      m_tracked_obj_flg = false ;
      memset(PreLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;
      memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
      m_TrackNum = 0 ;
      m_curr_frm_num = 0 ;
      ROK();
    }
    //----预测跟踪帧的目标
    for (int i = 0 ;i < m_TrackNum; i++)
    {
      DetectedTrackedObject(&TrackObject[i],pFrame_bin_in,Warning_Line_Pra,Alarm_Line_Pra , alarm_type); 
    }
    //----预测当前帧的目标搜索位置
    for (int i = 0 ;i < m_curr_frm_num; i++)
    {
      ObjLabelInfo[i].flag = true;
      if (ObjLabelInfo[i].flag)
      {
        FindDetectedObject( &ObjLabelInfo[i],pFrame_bin_in,Warning_Line_Pra,Alarm_Line_Pra); 
      }
    }

    //dubing add 删除多余的轨迹
    //     if(m_TrackNum > 2)
    //     {
    //       DeleteAdditionalLine();
    //     }
#if ( 1 == SY_DEBUG)
//     switch (m_nChannel_ID)
//     {
//     case 0 :
//       SavetoFile(m_nChannel_ID + 1);
//       break;
//     case 1 :
//       SavetoFile(m_nChannel_ID + 1);
//       break;
//     case 2 :
//       SavetoFile(m_nChannel_ID + 1);
//       break;
//     case 3 :
//       SavetoFile(m_nChannel_ID + 1);
//       break;
//     case DEFAULTCHANNELMODE :    //! 调试用
//       SavetoFile();
//       break;
//     }
#endif
    memcpy(PreLabelInfo, ObjLabelInfo,sizeof(ObjLabelInfoStruct) * m_curr_frm_num) ;
    memset(ObjLabelInfo, 0, sizeof(ObjLabelInfoStruct) * m_nTrackObjectMaxNum) ;

    m_pre_frm_num = m_curr_frm_num ;
    m_curr_frm_num = 0 ;

    if(m_TrackNum == 0)
    {
      m_tracked_obj_flg = FALSE;
    }

    //----delete repeated object
    uint16_t m_nTempTrackObjNum = 0 ;
    LabelObjStatus m_pTempObjTrackStatus[m_nTrackObjectMaxNum] ;
    memset(m_pTempObjTrackStatus, 0, m_nTrackObjectMaxNum * sizeof(LabelObjStatus) ) ;

    for (int i = 0; i < m_TrackNum; i++)
    {
      if (TrackObject[i].flag)
      {
        memcpy(&m_pTempObjTrackStatus[m_nTempTrackObjNum],&TrackObject[i],sizeof(LabelObjStatus));
        m_nTempTrackObjNum++;
      }			
    }

    if (0 == m_nTempTrackObjNum)
    {
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
      m_tracked_obj_flg = FALSE;
      m_TrackNum = 0 ;
    }
    else if (m_TrackNum == m_nTempTrackObjNum)
    {
      m_tracked_obj_flg = TRUE;
    }
    else if( m_nTempTrackObjNum > m_nTrackObjectMaxNum * 0.8 )
    {
      m_tracked_obj_flg = TRUE;
      m_TrackNum = 0 ;
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
    }
    else
    {
      memset(TrackObject,0,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
      memcpy(TrackObject,m_pTempObjTrackStatus,m_nTrackObjectMaxNum*sizeof(LabelObjStatus));

      m_TrackNum = m_nTempTrackObjNum;
      m_tracked_obj_flg = TRUE;
    }
#if ( 1 == SY_DEBUG)
    Drawtrack(pFrame_in);//! 调试用，以后删
#endif
    ROK();
  }
  ROK();
}
ErrVal CPersonDetect::init_match_track()
{
  uint16_t i = 0;
  memset(TrackObject, 0, m_nTrackObjectMaxNum*sizeof(LabelObjStatus));
  memset(ObjLabelInfo, 0, m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
  memset(PreLabelInfo, 0, m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));

  m_tracked_obj_flg     = FALSE ;       //找到预测目标标志
  m_curr_frm_num        = 0     ;       //得到的当前帧数
  m_pre_frm_num         = 0     ;       //前一帧的目标数
  m_TrackNum            = 0     ;       //
  ROK();
}
ErrVal
CPersonDetect::Statistics_obj_curinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                                       const  CFrameContainer* const pFrame_curr_in,
                                       const CFrameContainer* const pFrame_bin_in)

{
  ASSERT(objDList_inout);
  ASSERT(pFrame_curr_in);
  ASSERT(pFrame_bin_in);

  m_curr_frm_num = objDList_inout->GetCount();
  if (0 == m_curr_frm_num)
  {
    memset(&ObjLabelInfo,0,m_nTrackObjectMaxNum*sizeof(ObjLabelInfoStruct));
    ROK();
  }

  uint8_t b_mean = 0, g_mean = 0, r_mean = 0;

  for (uint16_t i = 1 ; i <= m_curr_frm_num; i++)
  {
    ObjLabelInfo[i - 1].ObjLabelCenter[0]   =   objDList_inout->GetAt(i)->m_nOuterRect[0] + (objDList_inout->GetAt(i)->m_nOuterRect[2])/2;
    ObjLabelInfo[i - 1].ObjLabelCenter[1]   =   objDList_inout->GetAt(i)->m_nOuterRect[1] + (objDList_inout->GetAt(i)->m_nOuterRect[3])/2;
    ObjLabelInfo[i - 1].ObjLabelRect[0]     =   objDList_inout->GetAt(i)->m_nOuterRect[0];
    ObjLabelInfo[i - 1].ObjLabelRect[1]     =   objDList_inout->GetAt(i)->m_nOuterRect[1];
    ObjLabelInfo[i - 1].ObjLabelRect[2]     =   objDList_inout->GetAt(i)->m_nOuterRect[2];
    ObjLabelInfo[i - 1].ObjLabelRect[3]     =   objDList_inout->GetAt(i)->m_nOuterRect[3];
    ObjLabelInfo[i - 1].WhiteSpotNum        =   objDList_inout->GetAt(i)->m_nXYDotPlus[2];
  }

  ROK();

}

ErrVal
CPersonDetect::DetectedTrackedObject(LabelObjStatus* pTrackObjInfo,
                                     const CFrameContainer*  const  pFrame_curr_in,
                                     Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                                     Cordon_Par* Alarm_Line_Pra ,
                                     const ALARMTYPE alarm_type)       //! 报警线参数
{
  ASSERT(pTrackObjInfo);
  ASSERT(pFrame_curr_in);

  uint16_t i = 0 ;

  int32_t curr_white_num   = 0 ;

  int16_t ForecastCenX     = pTrackObjInfo->m_nForecastCenter[0] ;
  int16_t ForecastCenY     = pTrackObjInfo->m_nForecastCenter[1] ;
  int32_t ForeWhiteSpotNum = pTrackObjInfo->WhiteSpotNum ;
  int16_t CurrTrackX       = pTrackObjInfo->m_nCurCenter[0];

  int16_t CurrX       = 0 ;                 
  int16_t CurrY       = 0 ;                 

  int16_t ptemp0      = 0 ;
  int16_t ptemp1      = 0 ;
  int16_t objNum      = 0 ;

  int16_t DetectX = 25   ;
  int16_t DetectY = 25   ;

  bool FindFalg = false;

  for (i = 0; i < m_curr_frm_num; i++)
  {
    ObjLabelInfo[i].flag = true;
    if(ObjLabelInfo[i].flag)
    {
      CurrX = ObjLabelInfo[i].ObjLabelCenter[0];
      CurrY = ObjLabelInfo[i].ObjLabelCenter[1];
      curr_white_num = ObjLabelInfo[i].WhiteSpotNum ;

      if(  abs(CurrX - ForecastCenX) <= DetectX
        &&  abs(CurrY - ForecastCenY) <= DetectY
        && abs( (int16_t)(ForeWhiteSpotNum - curr_white_num) ) < ForeWhiteSpotNum * 2 )
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
  ChangeObjectValue(&ObjLabelInfo[objNum], pTrackObjInfo, FindFalg,pFrame_curr_in,Warning_Line_Pra,Alarm_Line_Pra ,alarm_type);

  return m_nTracked;
}
ErrVal
CPersonDetect::ChangeObjectValue(ObjLabelInfoStruct* pLabelObjStatus, 
                                 LabelObjStatus* pTrackObjInfo, 
                                 bool FindObjectFlag ,
                                 const CFrameContainer*  const  pFrame_curr_in,
                                 Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                                 Cordon_Par* Alarm_Line_Pra ,
                                 const ALARMTYPE alarm_type)       //! 报警线参数
{
  ASSERT(pLabelObjStatus);
  ASSERT(pTrackObjInfo);
  ASSERT(pFrame_curr_in);
  uint16_t i = 0 ;

  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();

  //-----更新预测目标信息
  if (FindObjectFlag) 
  {
    pTrackObjInfo->FindNum++;
    pTrackObjInfo->m_nTrackNum++;
    pTrackObjInfo->m_nLostFrameNum = 0;
    memcpy(&pTrackObjInfo->m_nObjRect,&pLabelObjStatus->ObjLabelRect,4*sizeof(pTrackObjInfo->m_nObjRect[0]));
    memcpy(&pTrackObjInfo->m_nPreCenter,&pTrackObjInfo->m_nCurCenter,2*sizeof(pTrackObjInfo->m_nCurCenter[0]));
    memcpy(&pTrackObjInfo->m_nCurCenter,&pLabelObjStatus->ObjLabelCenter,2*sizeof(pLabelObjStatus->ObjLabelCenter[2]));
    //     pTrackObjInfo->WhiteSpotNum = (uint32_t)((float)(pLabelObjStatus->WhiteSpotNum + pTrackObjInfo->m_nTrackNum * pTrackObjInfo->WhiteSpotNum ) /  (float)(pTrackObjInfo->m_nTrackNum + 1) +0.5f);  
    pTrackObjInfo->WhiteSpotNum = pLabelObjStatus->WhiteSpotNum;  

    pTrackObjInfo->xDiff = abs((int16_t)(pTrackObjInfo->m_nPreCenter[0] - pTrackObjInfo->m_nCurCenter[0]));
    pTrackObjInfo->yDiff = abs((int16_t)(pLabelObjStatus->ObjLabelCenter[1] - pTrackObjInfo->m_nPreCenter[1]));

    //==SY==

    pTrackObjInfo->m_size_Plus[0] += pTrackObjInfo->m_nObjRect[2];
    pTrackObjInfo->m_size_Plus[1] += pTrackObjInfo->m_nObjRect[3];

    pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] = MIN(imWidth,pTrackObjInfo->m_nObjRect[0] +pTrackObjInfo->m_nObjRect[2]/2);
    pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] = MIN(imHeight,pTrackObjInfo->m_nObjRect[1] +pTrackObjInfo->m_nObjRect[3]/2);
    //pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] = pTrackObjInfo->m_nCurCenter[0];
    //pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] = pTrackObjInfo->m_nCurCenter[1];


    //     if (!b_First_Alarm)
    //     {
    //       b_First_Alarm = Judge_Slop_Over_Line(pTrackObjInfo,Warning_Line_Pra,/*LEFTTORIGNT*/ alarm_type);
    //     }

    if (!pTrackObjInfo->b_Warning)
    {
      pTrackObjInfo->b_Warning = Judge_Slop_Over_Line(pTrackObjInfo,Warning_Line_Pra,/*LEFTTORIGNT*/ alarm_type);
    }
    if (pTrackObjInfo->b_Warning)
    {
      b_First_Alarm = true;
    }

    /*注释二次报警，备用*/
    //     if (!pTrackObjInfo->b_Alarm)
    //     {
    //       pTrackObjInfo->b_Alarm= Judge_Slop_Over_Line(pTrackObjInfo,Alarm_Line_Pra,/*LEFTTORIGNT*/ alarm_type);
    //     }
    //     if (pTrackObjInfo->b_Alarm)
    //     {
    //       b_Second_Alarm = true;
    //     }

    if (pTrackObjInfo->track_pot_count >= m_nTrackObjShowNum-1)
    {
      for (int i = 0; i < m_nTrackObjShowNum-1; i++)
      {
        pTrackObjInfo->m_nTrack_pt[0][i] = pTrackObjInfo->m_nTrack_pt[0][i + 1];
        pTrackObjInfo->m_nTrack_pt[1][i] = pTrackObjInfo->m_nTrack_pt[1][i + 1];
      }
    }

    if (pTrackObjInfo->track_pot_count < m_nTrackObjShowNum -1)
    {
      pTrackObjInfo->track_pot_count++;
    }
    //==SY==


    //-----预测
    if ( pTrackObjInfo->m_nCurCenter[0] > pTrackObjInfo->m_nPreCenter[0] )//往右走
    {
      pTrackObjInfo->m_nForecastCenter[0] =MIN(imWidth,pTrackObjInfo->m_nCurCenter[0] + pTrackObjInfo->xDiff);
    }
    else//往左走
    {
      pTrackObjInfo->m_nForecastCenter[0] = MAX(0, ((int)pTrackObjInfo->m_nCurCenter[0] - pTrackObjInfo->xDiff ));
    }
    if (  pTrackObjInfo->m_nCurCenter[1] < pTrackObjInfo->m_nPreCenter[1] )//往上走
    {
      pTrackObjInfo->m_nForecastCenter[1] = MAX(0,((int)pTrackObjInfo->m_nCurCenter[1] - pTrackObjInfo->yDiff));
    }
    else//
    {
      pTrackObjInfo->m_nForecastCenter[1] = MIN(imHeight , pTrackObjInfo->m_nCurCenter[1] + pTrackObjInfo->yDiff);
    }
  }
  //-----未找到与预测目标相匹配的对象
  else
  {
    pTrackObjInfo->m_nTrackNum++;
    pTrackObjInfo->m_nLostFrameNum++;		 

    if ( pTrackObjInfo->m_nLostFrameNum > 2 )
    {
      pTrackObjInfo->flag = false ;
      return m_nTracked;
    }

    pTrackObjInfo->m_nPreCenter[0] = pTrackObjInfo->m_nCurCenter[0];  
    pTrackObjInfo->m_nPreCenter[1] = pTrackObjInfo->m_nCurCenter[1];
    pTrackObjInfo->m_nCurCenter[0] = pTrackObjInfo->m_nForecastCenter[0]; 
    pTrackObjInfo->m_nCurCenter[1] = pTrackObjInfo->m_nForecastCenter[1];

    pTrackObjInfo->xDiff = abs((int16_t)(pTrackObjInfo->m_nCurCenter[0] - pTrackObjInfo->m_nPreCenter[0]));
    pTrackObjInfo->yDiff = abs((int16_t)(pTrackObjInfo->m_nCurCenter[1] - pTrackObjInfo->m_nPreCenter[1]));

    //==SY== 
    /*当丢帧时，跟踪点位置不变*/
    pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] = pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count - 1] ;
    pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] = pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count - 1] ;

    /*当丢帧时，跟踪点预测得出*/
    //     pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] = MIN(imWidth, pTrackObjInfo->m_nCurCenter[0] + pTrackObjInfo->m_nObjRect[2]);
    //     pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] = MIN(imHeight,pTrackObjInfo->m_nCurCenter[1] + pTrackObjInfo->m_nObjRect[3]);

    if (pTrackObjInfo->track_pot_count >= m_nTrackObjShowNum-1)
    {
      for (int i = 0; i < m_nTrackObjShowNum-1; i++)
      {
        pTrackObjInfo->m_nTrack_pt[0][i] = pTrackObjInfo->m_nTrack_pt[0][i + 1];
        pTrackObjInfo->m_nTrack_pt[1][i] = pTrackObjInfo->m_nTrack_pt[1][i + 1];
      }
    }

    if (pTrackObjInfo->track_pot_count < m_nTrackObjShowNum -1)
    {
      pTrackObjInfo->track_pot_count++;
    }
    //==SY== 


    if ( pTrackObjInfo->m_nCurCenter[0] > pTrackObjInfo->m_nPreCenter[0] )  //! 往右走
    {
      pTrackObjInfo->m_nForecastCenter[0] = MIN(imWidth , (pTrackObjInfo->m_nCurCenter[0] + pTrackObjInfo->xDiff));
    }
    else
    {
      pTrackObjInfo->m_nForecastCenter[0] = MAX(0 , ((int)pTrackObjInfo->m_nCurCenter[0] - pTrackObjInfo->xDiff));
    }

    if (  pTrackObjInfo->m_nCurCenter[1] < pTrackObjInfo->m_nPreCenter[1] ) // 往上走
    {
      pTrackObjInfo->m_nForecastCenter[1] = MAX(0 , ((int)pTrackObjInfo->m_nCurCenter[1] - pTrackObjInfo->yDiff ));
    }
    else
    {
      pTrackObjInfo->m_nForecastCenter[1] = MIN(imHeight , (pTrackObjInfo->m_nCurCenter[1] + pTrackObjInfo->yDiff));
    }
  }
  return m_nTracked;
}

ErrVal 
CPersonDetect::FindDetectedObject( ObjLabelInfoStruct* pCurrObjInfo,
                                  const CFrameContainer*  const  pFrame_curr_in ,
                                  Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                                  Cordon_Par* Alarm_Line_Pra )       //! 报警线参数

{
  ASSERT(pCurrObjInfo);
  ASSERT(pFrame_curr_in);

  if (m_pre_frm_num == 0) 
  {
    return m_nRemove;
  }

  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();

  uint16_t i = 0 ;
  uint16_t j = 0 ;

  int16_t ptemp0 = 0;
  int16_t ptemp1 = 0;
  int16_t v_obj_num = 0 ;

  bool cbv_find_flg = false;
  bool cbv_add_flg  = false;
  bool no_track_flg = false;

  int32_t previous_WhiteSpotNum = 0 ;  // 上一帧物体白点个数
  int16_t LastX = 0 ;                  // 上一帧物体重心X
  int16_t LastY = 0 ;                  // 上一帧物体重心Y

  int16_t x_currCen       = pCurrObjInfo->ObjLabelCenter[0];
  int16_t y_currCen       = pCurrObjInfo->ObjLabelCenter[1];
  int32_t curr_white_num  = pCurrObjInfo->WhiteSpotNum;

  if (0 == m_TrackNum)
  {
    no_track_flg = true ;
  }

  for (i = 0; i < m_pre_frm_num; i++)
  {
    PreLabelInfo[i].flag = true;
    if(PreLabelInfo[i].flag)
    {
      LastX = PreLabelInfo[i].ObjLabelCenter[0];
      LastY = PreLabelInfo[i].ObjLabelCenter[1];
      previous_WhiteSpotNum = PreLabelInfo[i].WhiteSpotNum;

      if (   ( abs( LastX - x_currCen ) <= 30 ) 
        && ( abs( LastY - y_currCen ) <= 30 )
        && ( abs( previous_WhiteSpotNum - curr_white_num ) < previous_WhiteSpotNum * 0.9 )
        )
      {
        if (cbv_find_flg)
        {
          ptemp1 = abs((uint16_t)(previous_WhiteSpotNum - curr_white_num));
          if(ptemp0 > ptemp1)
          {
            ptemp0 = ptemp1;
            v_obj_num = i;
          }
        }
        else
        {
          ptemp0 = abs((uint16_t)(previous_WhiteSpotNum - curr_white_num));
          v_obj_num = i;
        }
        cbv_find_flg = true ;
      } 
    }
  }

  //----判断该目标是否是跟踪预测中的目标
  if (cbv_find_flg)
  {
    cbv_add_flg = true;

    LastX = PreLabelInfo[v_obj_num].ObjLabelCenter[0];          
    LastY = PreLabelInfo[v_obj_num].ObjLabelCenter[1];

    if (!no_track_flg)
    {
      for (j = 0 ; j < m_TrackNum ; j++ )
      {
        if (  LastX     == TrackObject[j].m_nPreCenter[0] 
        &&    LastY     == TrackObject[j].m_nPreCenter[1]
        &&    x_currCen == TrackObject[j].m_nCurCenter[0]
        &&    y_currCen == TrackObject[j].m_nCurCenter[1]  )
        {
          cbv_add_flg = false ;
          break ;
        }
      }
    }
  }
  //----建立跟踪信息
  if (cbv_add_flg) 
  {
    TrackObject[m_TrackNum].flag            = true ;
    TrackObject[m_TrackNum].FindNum         = 2 ;
    TrackObject[m_TrackNum].m_nTrackNum     = 0 ;  /*考虑改为  = 2*/
    TrackObject[m_TrackNum].m_nLostFrameNum = 0 ;
    TrackObject[m_TrackNum].WhiteSpotNum    = pCurrObjInfo->WhiteSpotNum ;

    memcpy(&TrackObject[m_TrackNum].m_nObjRect,&pCurrObjInfo->ObjLabelRect,4*sizeof(pCurrObjInfo->ObjLabelRect[0]));
    memcpy(&TrackObject[m_TrackNum].m_nPreCenter,&PreLabelInfo[v_obj_num].ObjLabelCenter,2*sizeof(PreLabelInfo[v_obj_num].ObjLabelCenter[0]));
    memcpy(&TrackObject[m_TrackNum].m_nCurCenter,&pCurrObjInfo->ObjLabelCenter,2*sizeof(pCurrObjInfo->ObjLabelCenter[2]));

    TrackObject[m_TrackNum].xDiff = abs(x_currCen - LastX);
    TrackObject[m_TrackNum].yDiff = abs(y_currCen - LastY);
    //==SY==
    TrackObject[m_TrackNum].m_size_Plus[0] += TrackObject[m_TrackNum].m_nObjRect[2];
    TrackObject[m_TrackNum].m_size_Plus[1] += TrackObject[m_TrackNum].m_nObjRect[3];
    TrackObject[m_TrackNum].m_nTrack_pt[0][0] = MIN(imWidth  - 1,TrackObject[m_TrackNum].m_nObjRect[0]  + TrackObject[m_TrackNum].m_nObjRect[2]/2);//待修改  近似相等
    TrackObject[m_TrackNum].m_nTrack_pt[1][0] = MIN(imHeight - 1,TrackObject[m_TrackNum].m_nObjRect[1]  + TrackObject[m_TrackNum].m_nObjRect[3]/2);//待修改  近似相等
    TrackObject[m_TrackNum].m_nTrack_pt[0][1] = MIN(imWidth  - 1,TrackObject[m_TrackNum].m_nObjRect[0]  + TrackObject[m_TrackNum].m_nObjRect[2]/2);
    TrackObject[m_TrackNum].m_nTrack_pt[1][1] = MIN(imHeight - 1,TrackObject[m_TrackNum].m_nObjRect[1]  + TrackObject[m_TrackNum].m_nObjRect[3]/2);

    //add 针对每条轨迹进行报警，防止出现漏报的情况dubing 2009
    //     if (!TrackObject[m_TrackNum].b_First_Alarm)
    //     {
    //       TrackObject[m_TrackNum].b_First_Alarm = Judge_Slop_Over_Line(&TrackObject[m_TrackNum],Warning_Line_Pra,LEFTTORIGNT);
    //     }
    //     if (!TrackObject[m_TrackNum].b_Second_Alarm)
    //     {
    //       TrackObject[m_TrackNum].b_Second_Alarm = Judge_Slop_Over_Line(&TrackObject[m_TrackNum],Alarm_Line_Pra,LEFTTORIGNT);
    //     }
    //add end   

    //! sy motify on 2009-12-23


    if (!TrackObject[m_TrackNum].b_Warning)
    {
      TrackObject[m_TrackNum].b_Warning = Judge_Slop_Over_Line(&TrackObject[m_TrackNum],Warning_Line_Pra,LEFTTORIGNT);
    }
    if (TrackObject[m_TrackNum].b_Warning)
    {
      b_First_Alarm = true;
    }

    /*注释二次报警，备用*/

    //     if (!TrackObject[m_TrackNum].b_Alarm)
    //     {
    //       TrackObject[m_TrackNum].b_Alarm = Judge_Slop_Over_Line(&TrackObject[m_TrackNum],Alarm_Line_Pra,LEFTTORIGNT);
    //     }
    //     if (TrackObject[m_TrackNum].b_Alarm)
    //     {
    //       b_Second_Alarm = true;
    //     }


    TrackObject[m_TrackNum].track_pot_count = 2;

    // ----确定物体水平运动方向，从左至右 1，从右至左 0
    if(TrackObject[m_TrackNum].m_nCurCenter[0] > LastX)
    {
      TrackObject[m_TrackNum].m_nDirection = 1;

      TrackObject[m_TrackNum].m_nForecastCenter[0] = MIN(imWidth ,TrackObject[m_TrackNum].m_nCurCenter[0] + TrackObject[m_TrackNum].xDiff);
    }
    else
    {
      TrackObject[m_TrackNum].m_nDirection = 0;

      TrackObject[m_TrackNum].m_nForecastCenter[0] = MAX(0 ,((int)TrackObject[m_TrackNum].m_nCurCenter[0] - TrackObject[m_TrackNum].xDiff));
    }
    //----确定物体竖直运动方向，从下至上 1，从上至下 0 
    if(TrackObject[m_TrackNum].m_nCurCenter[1] <= LastY) // 往上走
    {
      TrackObject[m_TrackNum].m_nUpDownCheck = 1 ;
      TrackObject[m_TrackNum].m_nForecastCenter[1] = MAX(0 , (int)TrackObject[m_TrackNum].m_nCurCenter[1] - TrackObject[m_TrackNum].yDiff)  ;
    }
    else// 往下走
    {
      TrackObject[m_TrackNum].m_nUpDownCheck = 0 ;
      TrackObject[m_TrackNum].m_nForecastCenter[1] = MIN(imHeight , (TrackObject[m_TrackNum].m_nCurCenter[1] + TrackObject[m_TrackNum].yDiff));
    }
    m_TrackNum++;
    cbv_find_flg = false ;
  }
  return m_nTracked;
}



bool CPersonDetect::Judge_Slop_Over_Line(LabelObjStatus* pTrackObjInfo, Cordon_Par* Slop_Over_Line_Pra,ALARMTYPE alarm_type)
{
  if (pTrackObjInfo->track_pot_count < 1) 
  {
    return false;
  }

  //     pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count]    = 232;
  //     pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count]    = 92;
  //     pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count-1]  = 255;
  //     pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count-1]  = 115;
  switch (alarm_type)
  {
  case DOUBLE_DIRECTION:
    if (((float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  <= 0
      &&(float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count - 1] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count - 1] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  >= 0)
      ||((float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  >= 0
      &&(float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count - 1] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count - 1] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  <= 0))
    {
      if (pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] > Slop_Over_Line_Pra->m_distancetotop
        &&pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] < Slop_Over_Line_Pra->m_distancetobottom)
      {
        return true;
      }
    }
    else
    {
      return false;
    }
    break;

  case RIGHTTOLEFT:
    if (((float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  <= 0
      &&(float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count - 1] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count - 1] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  >= 0))
    {
      if (pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] > Slop_Over_Line_Pra->m_distancetotop
        &&pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] < Slop_Over_Line_Pra->m_distancetobottom)
      {
        return true;
      }
    }
    else
    {
      return false;
    }
    break;

  case LEFTTORIGNT:
    if (((float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  >= 0
      &&(float)pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count - 1] -(float)pTrackObjInfo->m_nTrack_pt[0][pTrackObjInfo->track_pot_count - 1] * Slop_Over_Line_Pra->m_slope - Slop_Over_Line_Pra->m_pitch  <= 0))
    {
      if (pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] > Slop_Over_Line_Pra->m_distancetotop
        &&pTrackObjInfo->m_nTrack_pt[1][pTrackObjInfo->track_pot_count] < Slop_Over_Line_Pra->m_distancetobottom)
      {
        return true;
      }
    }
    else
    {
      return false;
    }
    break;

  default :
    break;
  }
  return false;
}
/*
*       1 车
*       2 人
*       3 其他
*
*/
// int CPersonDetect::Differentiate_From_Car_Man(LabelObjStatus* pTrackObjInfo)
// {
//   uint16_t ave_width = 0, ave_heght = 0;
//   ave_width = pTrackObjInfo->m_size_Plus[0] / pTrackObjInfo->FindNum ;
//   ave_heght = pTrackObjInfo->m_size_Plus[1] / pTrackObjInfo->FindNum ;
// 
//   if (ave_width > 30 && ave_heght > 30)
//   {
//     return 1;
//   }
// 
//   if (ave_width > 30 && ave_heght > 30)
//   {
//     return 2;
//   }
// 
//   return 3;
// }
void CPersonDetect::Drawtrack(CFrameContainer*  pFrame_curr_in)
{
  ASSERT(pFrame_curr_in);

  uint8_t* m_rgbbuf = pFrame_curr_in->m_BmpBuffer;
  uint32_t top_x=0, top_y = 0, bottom_x = 0, bottom_y = 0;//连通区域外接矩形左上、右下坐标。

  const uint16_t imWidth  = pFrame_curr_in->getWidth();
  const uint16_t imHeight = pFrame_curr_in->getHeight();

  uint16_t cen_x = 0 ;
  uint16_t cen_y = 0 ;

  for ( int k = 0; k < m_TrackNum; k++)
  {
    top_x = TrackObject[k].m_nObjRect[0];
    top_y = TrackObject[k].m_nObjRect[1];
    bottom_x = top_x + TrackObject[k].m_nObjRect[2];
    bottom_y = top_y + TrackObject[k].m_nObjRect[3];

    for(int m = 0; m < TrackObject[k].track_pot_count; m++)
    {
      cen_x = TrackObject[k].m_nTrack_pt[0][m] ;
      cen_y = TrackObject[k].m_nTrack_pt[1][m] ;

      if (cen_y>5 && cen_y < imHeight-5 && cen_x < imWidth-5 &&cen_x > 5 )
      {
        pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + 3 * cen_x + 0] = 0;
        pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + 3 * cen_x + 1] = 0;
        pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + 3 * cen_x + 2] = 255;

        pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + MIN(3*(imWidth - 1) , 3 * cen_x + 3)] = 0;
        pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + MIN(3*(imWidth - 1) , 3 * cen_x + 4)] = 0;
        pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + MIN(3*(imWidth - 1) , 3 * cen_x + 5)] = 255;

        //         pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + MAX(0 , 3 * cen_x - 3)] = 0;
        //         pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + MAX(0 , 3 * cen_x - 2)] = 0;
        //         pFrame_curr_in->m_BmpBuffer[3 * cen_y * imWidth + MAX(0 , 3 * cen_x - 1)] = 255;

      }
    }
  }
}


void CPersonDetect::Calculate_Line_Parameter(float &slppe,float &pitch,float pt1_x,float pt1_y,float pt2_x,float pt2_y)
{
  if ( 0 == (pt2_x - pt1_x))
  {
    slppe = pitch = 0;
//     MessageBoxA( NULL, "划线斜率不能为0!请检查配置文件是否正确!", "TestUI", MB_OK );
    return ;
  }

  slppe = (pt2_y - pt1_y)/(pt2_x - pt1_x);
  pitch = (pt2_x * pt1_y - pt1_x * pt2_y)/(pt2_x - pt1_x);
}

void CPersonDetect::Draw_Warning_Line(Cordon_Par* Slop_Over_Line_Pra,CFrameContainer* pFrame_in )
{
  ASSERT(pFrame_in);

  const uint16_t imWidth  = pFrame_in->getWidth();
  const uint16_t imHeight = pFrame_in->getHeight();

  if (0 == Slop_Over_Line_Pra->m_slope|| 0 == Slop_Over_Line_Pra->m_pitch)
  {
    return;
  }

  //   for (int j = Slop_Over_Line_Pra->m_distancetotop;j < imHeight - 5;j++)
  for (int j = Slop_Over_Line_Pra->m_distancetotop;j <Slop_Over_Line_Pra->m_distancetobottom;j++)
  {
    for (int i = 0; i < imWidth ;i++)
    {
      if (abs(j - Slop_Over_Line_Pra->m_slope * i - Slop_Over_Line_Pra->m_pitch )<1)
      {
        //            memset(&pFrame_in->m_BmpBuffer[3*j*imWidth+3*i+0],255,3);
        pFrame_in->m_BmpBuffer[3*j*imWidth+3*i+0] = 0;
        pFrame_in->m_BmpBuffer[3*j*imWidth+3*i+1] = 0;
        pFrame_in->m_BmpBuffer[3*j*imWidth+3*i+2] = 255;
      }
    }
  }
}

ErrVal 
CPersonDetect::Interlaced_Scanning (CFrameContainer* pFrame_low,const CFrameContainer* const pFrame_high,uint8_t SampleIntervallines)
{
  ASSERT(pFrame_low);
  ASSERT(pFrame_high);

  const uint16_t width = pFrame_high->getWidth();
  const uint16_t height = pFrame_high->getHeight();
  int s = SampleIntervallines;

  for (int j = 0 ; j < height; j += SampleIntervallines)
  {
    for (int i = 0 ; i < width; i += SampleIntervallines)
    {
      pFrame_low->m_BmpBuffer[3* (j/s) * (width/s) + 3*(i/s) + 0] = pFrame_high->m_BmpBuffer[3*j*width+ 3*i + 0];
      pFrame_low->m_BmpBuffer[3* (j/s) * (width/s) + 3*(i/s) + 1] = pFrame_high->m_BmpBuffer[3*j*width+ 3*i + 1];
      pFrame_low->m_BmpBuffer[3* (j/s) * (width/s) + 3*(i/s) + 2] = pFrame_high->m_BmpBuffer[3*j*width+ 3*i + 2];
    }
  }
  ROK();
}

ErrVal
CPersonDetect::Shadow_Mask (CFrameContainer* pFrame_in_out,const uint8_t* const pRGB_template)
{
  ASSERT(pFrame_in_out);
  ASSERT(pRGB_template);
  uint16_t imWidth = pFrame_in_out->getWidth();
  uint16_t imHeight = pFrame_in_out->getHeight();
  for (int j = 0; j < imHeight; ++j )
  {
    for (int i = 0 ; i < imWidth; ++i)
    {
      if (0 == pRGB_template[j*imWidth + i] )
      {
        memset(&pFrame_in_out->m_BmpBuffer[3*j*imWidth + 3*i], 0 , 3);
      }
    }
  }
  ROK();
}
void CPersonDetect::SavetoFile(int channel)
{
  SYSTEMTIME Systemtime ;
  GetLocalTime(&Systemtime);
  CString   name;
  char AviFileName[256];
  if( DEFAULTCHANNELMODE == channel )
  {
    //     sprintf(AviFileName,"F:\\desktop\\data.txt" );

    name.Format(_T("C:\\%d月%d日_单路调试"),Systemtime.wMonth,Systemtime.wDay);
    sprintf(AviFileName,"C:\\%d月%d日_单路调试\\data.txt", Systemtime.wMonth,Systemtime.wDay);
  }
  else
  {
    name.Format(_T("C:\\%d月%d日_多路调试"),Systemtime.wMonth,Systemtime.wDay);
    sprintf(AviFileName,"C:\\%d月%d日_多路调试\\channel_%d_data.txt",Systemtime.wMonth,Systemtime.wDay,channel);
  }
  CreateDirectory(name,NULL);

  uint16_t len;
  CString str;
  CFile file;
  file.Open( AviFileName,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
  file.SeekToEnd();

  str.Format("\r\nFrame_Num:%d\r\n",CurFrameNum);
  len=str.GetLength();
  file.Write(str,len);
  str.Format("m_curr_frm_num:%d\r\n",m_curr_frm_num);
  len=str.GetLength();
  file.Write(str,len);

  for ( int i = 0 ; i < m_TrackNum ; ++i)
  {
    //     str.Format("TrackObject[%d].flag:%d\r\n",i,TrackObject[i].flag);
    //     len=str.GetLength();  file.Write(str,len);
    //     str.Format("TrackObject[%d].FindNum:%d\r\n",i,TrackObject[i].FindNum);
    //     len=str.GetLength();  file.Write(str,len);
    str.Format("TrackObject[%d].m_nTrackNum:%d\r\n",i,TrackObject[i].m_nTrackNum);
    len=str.GetLength();  file.Write(str,len);
    str.Format("TrackObject[%d].m_nLostFrameNum:%d\r\n",i,TrackObject[i].m_nLostFrameNum);
    len=str.GetLength();  file.Write(str,len);
    str.Format("TrackObject[%d].WhiteSpotNum:%d\r\n",i,TrackObject[i].WhiteSpotNum);
    len=str.GetLength();  file.Write(str,len);
    //     str.Format("TrackObject[%d].m_nObjRect:%d,%d,%d,%d\r\n",i,TrackObject[i].m_nObjRect[0],TrackObject[i].m_nObjRect[1],TrackObject[i].m_nObjRect[2],TrackObject[i].m_nObjRect[3]);
    //     len=str.GetLength();  file.Write(str,len);
    //     str.Format("TrackObject[%d].m_nPreCenter:%d,%d\r\n",i,TrackObject[i].m_nPreCenter[0],TrackObject[i].m_nPreCenter[1]);
    //     len=str.GetLength();  file.Write(str,len);
    //     str.Format("TrackObject[%d].m_nCurCenter:%d,%d\r\n",i,TrackObject[i].m_nCurCenter[0],TrackObject[i].m_nCurCenter[1]);
    //     len=str.GetLength();  file.Write(str,len);
    //     str.Format("TrackObject[%d].xDiff:%d\r\n",i,TrackObject[i].xDiff);
    //     len=str.GetLength();  file.Write(str,len);
    //     str.Format("TrackObject[%d].yDiff:%d\r\n",i,TrackObject[i].yDiff);
    //     len=str.GetLength();  file.Write(str,len);
    str.Format("TrackObject[%d].track_pot_count:%d\r\n",i,TrackObject[i].track_pot_count);
    len=str.GetLength();  file.Write(str,len);

    str.Format("TrackObject[%d].m_nTrack_pt_x:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",i,
      TrackObject[i].m_nTrack_pt[0][0],
      TrackObject[i].m_nTrack_pt[0][1],
      TrackObject[i].m_nTrack_pt[0][2],
      TrackObject[i].m_nTrack_pt[0][3],
      TrackObject[i].m_nTrack_pt[0][4],
      TrackObject[i].m_nTrack_pt[0][5],
      TrackObject[i].m_nTrack_pt[0][6],
      TrackObject[i].m_nTrack_pt[0][7],
      TrackObject[i].m_nTrack_pt[0][8],
      TrackObject[i].m_nTrack_pt[0][9]
    );
    len=str.GetLength();  file.Write(str,len);

    str.Format("TrackObject[%d].m_nTrack_pt_y:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",i,
      TrackObject[i].m_nTrack_pt[1][0],
      TrackObject[i].m_nTrack_pt[1][1],
      TrackObject[i].m_nTrack_pt[1][2],
      TrackObject[i].m_nTrack_pt[1][3],
      TrackObject[i].m_nTrack_pt[1][4],
      TrackObject[i].m_nTrack_pt[1][5],
      TrackObject[i].m_nTrack_pt[1][6],
      TrackObject[i].m_nTrack_pt[1][7],
      TrackObject[i].m_nTrack_pt[1][8],
      TrackObject[i].m_nTrack_pt[1][9]
    );
    len=str.GetLength();  file.Write(str,len);
  }
  file.Close();
}

void CPersonDetect::SaveJpeg_File_color(const CFrameContainer* const pFrame_in,
                                        const  int channel)
{
//   ASSERT(pFrame_in);
//   SYSTEMTIME Systemtime ;
//   GetLocalTime(&Systemtime);
//   CString   name;
//   int size ;
//   int wide = pFrame_in->getWidth();
//   int height = pFrame_in->getHeight();
//   unsigned  char * jpeg_data, *RGB_data,*rgbbuftemp;
//   rgbbuftemp = pFrame_in->m_BmpBuffer ;
//   RGB_data = new BYTE[3*wide * height];
//   jpeg_data = new BYTE[3*wide * height];
//   for(int i = 0 ; i < height ; i ++ )
//   {
//     memcpy(RGB_data + i *wide*3 , rgbbuftemp + (height - i -1)*wide*3 ,3*wide);
//   }
//   CTonyJpegEncoder  jpeg;
//   jpeg.CompressImage(RGB_data,jpeg_data,wide,	height,	size);
// 
//   char AviFileName[256];
//   if( DEFAULTCHANNELMODE == channel )
//   {
//     //     name.Format(_T("f:\\desktop\\%d月%d日"),Systemtime.wMonth,Systemtime.wDay);
//     //     sprintf(AviFileName,"f:\\desktop\\%d月%d日\\%d_%d_%d_%d.jpg",Systemtime.wMonth,Systemtime.wDay,channel,Systemtime.wHour,Systemtime.wMinute,Systemtime.wSecond );
//     name.Format(_T("C:\\%d月%d日_单路调试"),Systemtime.wMonth,Systemtime.wDay);
//     sprintf(AviFileName,"C:\\%d月%d日_单路调试\\%d_%d_%d_%d_%d.jpg",Systemtime.wMonth,Systemtime.wDay,channel,Systemtime.wHour,Systemtime.wMinute,Systemtime.wSecond ,Systemtime.wMilliseconds);
//   }
//   else
//   {
//     name.Format(_T("C:\\%d月%d日_多路调试"),Systemtime.wMonth,Systemtime.wDay);
//     sprintf(AviFileName,"C:\\%d月%d日_多路调试\\%d_%d_%d_%d_%d.jpg",Systemtime.wMonth,Systemtime.wDay,channel,Systemtime.wHour,Systemtime.wMinute,Systemtime.wSecond ,Systemtime.wMilliseconds);
//   }
//   CreateDirectory(name,NULL);
//   FILE *pFile1=fopen(AviFileName,"wb+");
//   fwrite(jpeg_data,1,size,pFile1);
//   fclose(pFile1);
// 
//   SAFEDELETEARRAY(jpeg_data);
//   SAFEDELETEARRAY(RGB_data);
}


void CPersonDetect::SaveJpeg_File_bin(const CFrameContainer* const pFrame_in,
                                      const uint32_t framenum,
                                      const uint32_t whitedots/*,const  int channel*/)
{
//   ASSERT(pFrame_in);
//   CFrameContainer* pframe_temp    = new CFrameContainer(pFrame_in->getWidth(),pFrame_in->getHeight(),pFrame_in->getYuvType());
//   *pframe_temp =  *pFrame_in;
//   pframe_temp->setChromaTo128();
//   pframe_temp->cvtY1toY255();
//   pframe_temp->updateRGB24FromYUV444();
//   SYSTEMTIME Systemtime ;
//   GetLocalTime(&Systemtime);
//   CString   name;
//   int size ;
//   int wide = pFrame_in->getWidth();
//   int height = pFrame_in->getHeight();
//   unsigned  char * jpeg_data, *RGB_data,*rgbbuftemp;
//   rgbbuftemp = pframe_temp->m_YuvPlane[0];
//   RGB_data = new BYTE[3*wide * height];
//   jpeg_data = new BYTE[wide * height];
//   for (int i = 0; i<wide; i ++) 
//   {
//     for (int j = 0; j < height; j++)
//     {
//       RGB_data[3*wide*j + 3*i + 0] =    \
//         RGB_data[3*wide*j + 3*i + 1] =    \
//         RGB_data[3*wide*j + 3*i + 2] =    \
//         rgbbuftemp[wide*(height - j - 1) + i ];
//     }
//   }
//   CTonyJpegEncoder  jpeg;
//   jpeg.CompressImage(	RGB_data,jpeg_data,wide,height,size);
//   // name.Format(_T("C:\\%d月%d日"),Systemtime.wMonth,Systemtime.wDay);
//   name.Format(_T("f:\\desktop\\%d月%d日"),Systemtime.wMonth,Systemtime.wDay);
//   CreateDirectory(name,   NULL);
//   char AviFileName[256];
//   //   sprintf(AviFileName,"C:\\%d月%d日\\%d_%d_%d_%d.jpg",Systemtime.wMonth,Systemtime.wDay,channel,Systemtime.wHour,Systemtime.wMinute,Systemtime.wSecond );
//   sprintf(AviFileName,"f:\\desktop\\%d月%d日\\%d_%d.jpg",Systemtime.wMonth,Systemtime.wDay,/*channel,*/framenum,whitedots );
//   FILE *pFile1=fopen(AviFileName,"wb+");
//   fwrite(jpeg_data,1,size,pFile1);
//   fclose(pFile1);
// 
//   SAFEDELETEARRAY(jpeg_data);
//   SAFEDELETEARRAY(RGB_data);
//   SAFEDELETE(pframe_temp);
}
ErrVal CPersonDetect::ImgMoveObjectDetect(CFrameContainer* p_frame_in_out)
{
  ASSERT (p_frame_in_out );
  CFrameContainer* pFrame_temp = new CFrameContainer(p_frame_in_out->getWidth(), p_frame_in_out->getHeight(), p_frame_in_out->getYuvType());
  ASSERT(pFrame_temp);
  *pFrame_temp = *p_frame_in_out;
  int16_t i = 0, j = 0, k = 0;
  int16_t rect[4] = {0, 0, 0, 0};
  int16_t tmp_top = 0, tmp_right = 0;
  uint16_t v_R = 0, v_G = 0, v_B = 255;
  uint8_t* p_data_in  = p_frame_in_out->m_BmpBuffer ;
  uint8_t* p_data_out = pFrame_temp->m_BmpBuffer ;

  uint16_t img_width  = p_frame_in_out->getWidth()* 3 ;
  uint16_t img_height = p_frame_in_out->getHeight()   ;
  uint16_t img_size   = p_frame_in_out->getRgbSize()  ;

  memcpy(p_data_out,p_data_in, img_size);

  for (i = 0; i < m_TrackNum; i++ )
  {
    if (TrackObject[i].b_Warning /*&& TrackObject[i].b_Alarm*/)//! 若启动b_Second_Alarm报警，将此注释打开
    {
      rect[0] = TrackObject[i].m_nObjRect[0] *3 ;// /3 * 9;
      rect[1] = TrackObject[i].m_nObjRect[1] ;
      rect[2] = TrackObject[i].m_nObjRect[2] ;
      rect[3] = TrackObject[i].m_nObjRect[3] ;
      tmp_top = TrackObject[i].m_nObjRect[1] + TrackObject[i].m_nObjRect[3] ;
      tmp_right = ( TrackObject[i].m_nObjRect[0] + TrackObject[i].m_nObjRect[2] )*3 ;// /3 * 9;
      for (j = rect[1]; j < tmp_top; j++ ) //left 
      {
        p_data_out[img_width * j + rect[0]   ] = (uint8_t)v_R ;
        p_data_out[img_width * j + rect[0]+1 ] = (uint8_t)v_G ;
        p_data_out[img_width * j + rect[0]+2 ] = (uint8_t)v_B ;
      }
      for (j = rect[1]; j < tmp_top; j++ ) //right
      {
        p_data_out[img_width * j + tmp_right   ] = (uint8_t)v_R ;
        p_data_out[img_width * j + tmp_right+1 ] = (uint8_t)v_G ;
        p_data_out[img_width * j + tmp_right+2 ] = (uint8_t)v_B ;
      }
      for (j = rect[0]; j < tmp_right; j+=3 ) //top
      {
        p_data_out[img_width * tmp_top + j   ] = (uint8_t)v_R ;
        p_data_out[img_width * tmp_top + j+1 ] = (uint8_t)v_G ;
        p_data_out[img_width * tmp_top + j+2 ] = (uint8_t)v_B ;
      }
      for (j = rect[0]; j < tmp_right; j+=3 ) //bottom
      {
        p_data_out[img_width * rect[1] + j   ] = (uint8_t)v_R ;
        p_data_out[img_width * rect[1] + j+1 ] = (uint8_t)v_G ;
        p_data_out[img_width * rect[1] + j+2 ] = (uint8_t)v_B ;
      }

      /*double lines*/
      for (j = rect[1]; j < tmp_top; j++ ) //left 
      {
        p_data_out[img_width * j + MIN(rect[0] + 3 , img_width - 3)] = (uint8_t)v_R ;
        p_data_out[img_width * j + MIN(rect[0] + 4 , img_width - 3)] = (uint8_t)v_G ;
        p_data_out[img_width * j + MIN(rect[0] + 5 , img_width - 3)] = (uint8_t)v_B ;
      }
      for (j = rect[1]; j < tmp_top; j++ ) //right
      {
        p_data_out[img_width * j + MAX(0 , tmp_right - 3)] = (uint8_t)v_R ;
        p_data_out[img_width * j + MAX(0 , tmp_right - 2)] = (uint8_t)v_G ;
        p_data_out[img_width * j + MAX(0 , tmp_right - 1)] = (uint8_t)v_B ;
      }
      for (j = rect[0]; j < tmp_right; j+=3 ) //top
      {
        p_data_out[img_width * MIN(img_height - 1 , tmp_top + 1) + j    ] = (uint8_t)v_R ;
        p_data_out[img_width * MIN(img_height - 1 , tmp_top + 1) + j + 1] = (uint8_t)v_G ;
        p_data_out[img_width * MIN(img_height - 1 , tmp_top + 1) + j + 2] = (uint8_t)v_B ;
      }
      for (j = rect[0]; j < tmp_right; j+=3 ) //bottom
      {
        p_data_out[img_width * MAX(0 , rect[1] - 1) + j    ] = (uint8_t)v_R ;
        p_data_out[img_width * MAX(0 , rect[1] - 1) + j + 1] = (uint8_t)v_G ;
        p_data_out[img_width * MAX(0 , rect[1] - 1) + j + 2] = (uint8_t)v_B ;
      }
    }
  }
  *p_frame_in_out = *pFrame_temp;
  SAFEDELETE(pFrame_temp);
  ROK(); 
}
/***************************************************
*删除多余的轨迹
*dubing 
****************************************************/
void CPersonDetect::DeleteAdditionalLine()
{
  int tempx = 0;
  int tempy = 0;

  for ( int i = 0 ; i < m_TrackNum && i < m_nTrackObjectMaxNum; i++)
  {
    tempx = TrackObject[i].m_nTrack_pt[0][TrackObject[i].track_pot_count];
    tempy = TrackObject[i].m_nTrack_pt[1][TrackObject[i].track_pot_count];

    for(int j = 0;j < m_TrackNum &&j < m_nTrackObjectMaxNum;j++)
    {
      if((tempx == TrackObject[j].m_nTrack_pt[0][TrackObject[j].track_pot_count])
        && (tempy == TrackObject[j].m_nTrack_pt[1][TrackObject[j].track_pot_count-1])
        && (i !=j))
      {
        memcpy(&TrackObject[j], &TrackObject[m_TrackNum-1],sizeof(LabelObjStatus));
        memset(&TrackObject[m_TrackNum-1],0,sizeof(LabelObjStatus));
        m_TrackNum --;
      }
    }
  }
}
int CPersonDetect::ReadIni(char *FilePath,char* pSectionName,char *pKeyName) 
{
  char strBuf[10];
  int retCode;
  //读取ini文件中相应字段的内容
  GetPrivateProfileString(pSectionName,pKeyName,NULL,strBuf,10,FilePath);
  retCode = atoi(strBuf);
  return retCode;
}

void CPersonDetect::GetIpAddress(sockaddr_in &sa) 
{
  WSADATA wsaData;
  char name[155];
  PHOSTENT hostinfo;
  if ( WSAStartup( MAKEWORD(2,0), &wsaData ) == 0 ) 
  { 
    if( gethostname ( name, sizeof(name)) == 0) 
    { 
      if((hostinfo = gethostbyname(name)) != NULL) 
      { 
        for (int i=0; hostinfo->h_addr_list[i]; i++) 
        { 
          memcpy (&sa.sin_addr.s_addr, hostinfo->h_addr_list[i],hostinfo->h_length); 
        }
      }
    }
  } 
  WSACleanup();
}

//********************************************************
//估算正常人的高度进行，进行人或者狗的识别
//********************************************************
float CPersonDetect::ComputeObjHeigth(int ObjCoordinate,float slope1,
                       float pitch1,
                       float slope2,
                       float pitch2)
{
  float referLength = 3.0; //参照物的长度
  float factLength = 1.8f; //人的实际长度

  float objLength = (factLength / referLength) 
    *((slope1 - slope2)*ObjCoordinate
    +(pitch1 -pitch1));

  return objLength;
}


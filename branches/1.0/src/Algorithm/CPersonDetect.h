/*! 
*************************************************************************************
* \file                  CPersonDetect.h
*    
* \brief
*    M<<???
* \date
*    2008_09_08
* \author
*    - Yang SONG                      <>
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/

#if !defined  (_CPERSONDETECT_H__261F00FA_6095_4a4f_81E1_61D01B095E1A)
#define _CPERSONDETECT_H__261F00FA_6095_4a4f_81E1_61D01B095E1A
#include <afx.h>

#include "matlabFunc.h"


#include <math.h>

#include "PersonStruct.h"


class CPersonDetect
{
#define  SAMPLING_INTERVAL    5             //  采样间隔（每两帧处理一帧）
#define  BKUPDATE_INTERVAL    10             //  采样间隔（每两帧处理一帧）
#define  MAXTHRESHOLD         70            //  第一次差分大阈值(室外)
#define  MINTHRESHOLD         45            //  第二次差分小阈值(室外)
#define  SMOOTHSTARTLINE      80            //  平滑起始行数
#define  BKFRAMEINTERVAL      40            //  背景采样融合间隔
#define  BKFRAMEBUFSZ         7             //  背景BUFFER长度
#define  WHITERATIO           90            //  白点率>40%时，背景重新生成
#define  MAXFRAMENUM          4000000000    //  (帧数计数器)nInputFrameNum>= MAXFRAMENUM时，重新计数
#define  PARABOLAWIDTHNEAR    3 //40  70   //抛物外接矩形长宽最大值  近景    
#define  PARABOLAHEIGHTNEAR   10//40  70                                                 
#define  PARABOLAWIDTHFAR     3//40         //抛物外接矩形长宽最大值  远景   
#define  PARABOLAHEIGHTFAR    3//40
#define  DEFAULTCHANNELMODE   9             // 默认通道号为9，用于单路调试，此数值与多线程的m_nChannel_ID不能相等
#define  ABS(a,b) ((a) >= (b) ? (a) - (b) : (b) - (a));

public:
  //===== constructor, destructor =====
  CPersonDetect(uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in,const uint8_t nChannel = DEFAULTCHANNELMODE);
  ~CPersonDetect(); 
  //====================================

public:/*求背景*/
  uint32_t       m_nRefreshFrameNum;         //<! the number of accumulated incoming frames. Range: [0, m_nGraspFrameInterval)
private:
  uint32_t       m_nGraspFrameInterval;       //<! the frame interval between grasping one frame. Range: [0, ...), '0' for continuous frame grasping.
  uint32_t       m_nGraspFramesBufferSize;    //<! the size of m_GraspFramesBuffer
  CFramesBuffer* m_GraspFramesBuffer;         //<! store the grasped frames to generate a Bkgnd
  uint32_t       CurFrameNum;                 //<! uint32_t [0 , 2^32-1(4294967296-1)];

public:
  /*融合背景初始化*/
  ErrVal initBkdet( uint16_t const  nYWidth_in, 
                    uint16_t const  nYHeight_in, 
                    YUVTYPE const  YuvType_in); // init function

  ErrVal clearBkdet();
  /*融合背景*/
  ErrVal generateRgbBkgnd_fromRgbFrames( CFrameContainer* frame_bkDetected,
                                        const CFrameContainer* const pFrame_curr_in,
                                        const CFrameContainer* const binBkgnd);  

/*新背景算法*/
  static const int m_nbufsz = 4;
  uint8_t*  m_nGetBKBuffer[m_nbufsz];
  /*更新背景初始化*/
  ErrVal initUpdateBk( uint16_t const  nYWidth_in, 
                       uint16_t const  nYHeight_in, 
                       YUVTYPE const  YuvType_in); // init function

  ErrVal clearUpdateBk();
  /*更新背景*/
  ErrVal UpdateBk(CFrameContainer* pFrame_BK_in_out,
                  const CFrameContainer* const pFrame_in,
                  const uint16_t bin_threshold,
                  const uint16_t dot_rate,
                  const uint16_t dot_rate_connection,
                  const uint32_t framenum);
    
private:/*处理*/

  CFrameContainer*   pFrame_RgbSmoothed ;
  CFrameContainer*   pFrame_bkgndDetected ;
  CFrameContainer*   pFrame_binremovebody ;
  CMatlabFunc*       pMatlabFunc;
  CFrameContainer*   pFrame_previous;
  uint8_t            m_nChannel_ID;

public:
  CDList< CObjLabeled*, CPointerDNode >*  ObjectLabeledDList;
  CDList< CObjLabeled*, CPointerDNode >*  ObjectLabeledDList1;

public:
  ErrVal  init_detect(  uint16_t const  nYWidth_in, 
                        uint16_t const  nYHeight_in, 
                        YUVTYPE const  YuvType_in ,
                        const uint8_t nChannel = 0);  // init function

  ErrVal  clear_detect();
  /*机场，人爬围栏初始化*/
  ErrVal  init_pudong (uint16_t const  nYWidth_in, 
                       uint16_t const  nYHeight_in, 
                       YUVTYPE const  YuvType_in);
  ErrVal  clear_pudong();
/*平滑*/
  ErrVal averagesmoothRgb( CFrameContainer* pFrame_RgbSmoothed_inout , \
                            const CFrameContainer*  const pFrame_decoded_in , \
                            const uint16_t NoSmoothLineNum = 1 ) const;//均值平滑

  //整帧图像大阈值差分
  ErrVal binarizeY_fromRgbBkgnd( CFrameContainer* pFrame_RgbtoYBinarized_inout, \
                                const CFrameContainer* const pFrame_curr_in, \
                                const CFrameContainer* const pFrame_bkgnd_in,
                                uint16_t RgbThreshold) const;
  //整帧图像小阈值差分
  ErrVal  binarizeRgbtoY( CFrameContainer* pFrame_RgbtoYBinarized_inout, const CFrameContainer* const  pFrame_curr_in, \
                          const CFrameContainer* const pFrame_bkgnd_in, \
                          const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                          uint16_t Large_RgbThreshold ,
                          uint16_t Small_RgbThreshold);

   /*在小分辨率差分并标定后，用binRgbtoY_LowtoHigh在大分辨率图像中做差分
   *
   *    标定后，得到分块区域外接矩形后，求二值图像。
   *    根据矩形框的上下位置不同，区分远景和近景，设定不同阈值，远景阈值低，近景阈值高。
   *    将矩形框的范围放大，进行小阈值差分。
   *    demarcation_line:图像远近景分界线
   */
  ErrVal  binRgbtoY_LowtoHigh( CFrameContainer* pFrame_RgbtoYBinarized_inout , 
                              const CFrameContainer* const pFrame_curr_in, //  
                              const CFrameContainer*  const pFrame_bkgnd_in, //
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_inout,//
                              uint16_t Large_RgbThreshold ,
                              uint16_t Small_RgbThreshold,
                              const uint16_t  demarcation_line);
/*提取两幅二值化图像中公共部分*/
  ErrVal  extract_public_part(CFrameContainer* pFrame_bin_data_out,
                              const CFrameContainer* const pFrame_bin_bk,
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_bin_bk,
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_bin_pre);

 /*腐蚀,普通3*3腐蚀*/
  ErrVal erodeY( const CFrameContainer* const pFrame_inout,  //
                uint32_t nErodeTimesPerPixel_in ,   //腐蚀次数
                uint16_t StartLineNum = 1) const; // 腐蚀起始行
   /*
   *  腐蚀，对亮度为255的点，如果周围255点的个数>nVotedOne_in，该点置255，否则置0。
   *  nMatrixWidth*nMatrixWidth的全1矩阵模板；
   *  nErodeStep：腐蚀次数。
   */
  void erodeY( CFrameContainer*  const pYuvframe, 
                const uint8_t nMatrixWidth, 
                const uint8_t nVotedOne_in, 
                uint16_t StartLineNum = 1,
                const uint8_t nErodeStep = 1)const;

/*膨胀*/
  ErrVal dilateY( const CFrameContainer* const pFrame_in, 
                uint32_t nDilateTimesPerPixel_in,
                uint16_t NoDilateLineNum  = 1) const;

  /*只显示彩色运动物体*/
  void  RGBmulY(CFrameContainer*   pFrame_input , 
                const CFrameContainer* const pFrame_matlabFunced );

  /*保存的抛物接口，去掉人员部分，减少人员对抛物的干扰*/
  void  binremovebody_list(CFrameContainer* pFrame_matlabFunced , 
                          const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout);
  /*去掉小目标
  *    demarcation_line:图像远近景分界线
  */
  void  deletminobj(CDList< CObjLabeled*, CPointerDNode >* objDList_inout,
                    const uint16_t demarcation_line);

  /*计算指定矩形范围内，目标的RGB均值*/
  void Calculate_BGR_Mean( uint8_t& b_mean,uint8_t& g_mean,uint8_t& r_mean,//
                          const CFrameContainer* const pFrame_curr_in,
                          const CFrameContainer* const pFrame_bin_in,
                          uint16_t top_x,uint16_t top_y,uint16_t bottom_x,uint16_t bottom_y);         

  enum ALARMTYPE   //!< 越线报警方式
  {
    LEFTTORIGNT,   //!< 从左到右      [- , +]
    RIGHTTOLEFT,  //!< 从右到左       [+ , -]
    DOUBLE_DIRECTION, //!< 双向越线   [+ , -] || [- , +]
  }; 

public:/*处理主函数*/

    ErrVal  PersenDetect_Process(CFrameContainer* pFrame_matlabFunced,  
                                 CFrameContainer* pRgbhumaninfo,
                                 CFrameContainer*    pFrame_input,  
                                 const uint8_t* const pRGB_template,  
                                 const ALARMTYPE alarm_type,
                                 uint16_t demarcation_line,
                                 const uint32_t framenum = 0); 


  /*************************************
  *        目标匹配、跟踪与预测
  *  
  *
  **************************************/
private:

  static const uint16_t m_nTrackObjectMaxNum = 200;  //! 最大跟踪目标数
  static const uint8_t m_nTrackObjShowNum = 10;      //! 显示跟踪轨迹点数
  static const uint8_t m_nMaxCameraNum = 4;          //! 同时处理通道数

public:
  struct LabelObjStatus 
  {
    bool     flag ;                   //continue track flag 
    uint16_t FindNum ;                //find number
    uint16_t m_nObjRect[4];           //top_x,top_y,width,height 
    uint16_t m_nForecastCenter[2];    //center_x,center_y of every label area
    uint16_t m_nPreCenter[2];         // tracked object center ,current and previous frame 
    uint16_t m_nCurCenter[2];
    uint8_t  m_nTrackNum;              // object tracked frame number
    uint8_t  m_nLostFrameNum;          // to indicate how many frames have lost
    uint32_t WhiteSpotNum;      
    uint16_t xDiff;                   //average migration length of every frame
    uint16_t yDiff;                   // migration length of every frame
    uint8_t  m_nUpDownCheck ;
    uint8_t  m_nDirection;             // if m_nDirection = 0,right to left;
    uint16_t m_size_Plus[2];              // 目标外接矩形的长宽信息  [sum_width,sum_height];
    uint16_t m_nTrack_pt[2][m_nTrackObjShowNum];  //保存跟踪轨迹点，用目标外接矩形右下角坐标填充。若目标跟丢，用m_nForecastCenter转换出右下角坐标填充。
    uint8_t track_pot_count;
    //dubing add 20091217												  
    bool  b_Warning;            //一级报警信号，告警区告警信号
    bool  b_Alarm;        //二级报警信号，预警区告警信号
    //add end
  };

  LabelObjStatus TrackObject[m_nTrackObjectMaxNum];

  struct ObjLabelInfoStruct
  {
    bool flag                 ;
    uint16_t ObjLabelRect[4]  ;          // top_x,top_y,width,height
    uint16_t ObjLabelCenter[2];          // Center
    uint32_t WhiteSpotNum     ;          // WHITE SPOT number of every label area
    uint8_t ObjBGRmeanValue[3]   ;       //<! [BMV, GMV, RMV]  B.G.R 均值
  };

  ObjLabelInfoStruct ObjLabelInfo[m_nTrackObjectMaxNum];
  ObjLabelInfoStruct PreLabelInfo[m_nTrackObjectMaxNum];

  bool     m_tracked_obj_flg     ;           //找到预测目标标志
  uint16_t m_curr_frm_num        ;           //当前帧得到目标数
  uint16_t m_pre_frm_num         ;           //前一帧的目标数
  uint16_t m_TrackNum            ;           //跟踪目标数目

  static const ErrVal m_nLost     = 0;
  static const ErrVal m_nTracked  = 1;
  static const ErrVal m_nRemove   = 2;

  struct  Cordon_Par                //! 预警线参数结构体
  {
    float         m_slope;          //! 斜率
    float         m_pitch;          //! 截距
    uint16_t      m_distancetotop;    //! 到图像顶部的距离（像素）
    uint16_t      m_distancetobottom;
  };
  Cordon_Par  Warning_Line[m_nMaxCameraNum];
  Cordon_Par  Alarm_Line[m_nMaxCameraNum];
  Cordon_Par  Border_line[m_nMaxCameraNum];


  //只对每条轨迹报警，这里不需要了dubing
  bool  b_First_Alarm;    //一级报警信号
  bool  b_Second_Alarm;    //二级报警信号

  uint32_t delay_counter;

  CFrameContainer*   m_pFrame_RgbSmoothed_low;
  CFrameContainer*   m_pFrame_bkgndDetected_low;
  CFrameContainer*   m_pFrame_matlabFunced_low;

public:

    void   setDelay_counter()                              { delay_counter = 0; }

 /*检测移动目标  指定区域画框*/
 ErrVal ImgMoveObjectDetect(CFrameContainer* p_frame_in_out);

 /*目标匹配和跟踪初始化*/
  ErrVal init_match_track();
  /*统计目标信息*/
  ErrVal Statistics_obj_curinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                                  const  CFrameContainer* const pFrame_curr_in,
                                  const CFrameContainer* const pFrame_bin_in);//输入二值化图像

  /* 目标匹配与跟踪部分 */
  ErrVal ForecastObjectDetect (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout ,
                              CFrameContainer* pFrame_in,
                              const CFrameContainer* const pFrame_bin_in,
                              Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                              Cordon_Par* Alarm_Line_Pra,
                              const ALARMTYPE alarm_type);       //! 报警线参数

  ErrVal DetectedTrackedObject(LabelObjStatus* pLabelObjStatus,
                              const CFrameContainer*  const  pFrame_curr_in,
                              Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                              Cordon_Par* Alarm_Line_Pra ,
                              const ALARMTYPE alarm_type) ;      //! 报警线参数

  ErrVal ChangeObjectValue(ObjLabelInfoStruct* pLabelObjStatus, 
                            LabelObjStatus* pTrackObjInfo, 
                            bool FindObjectFlag,
                            const CFrameContainer*  const  pFrame_curr_in,
                            Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                            Cordon_Par* Alarm_Line_Pra ,
                            const ALARMTYPE alarm_type);       //! 报警线参数

  ErrVal FindDetectedObject(ObjLabelInfoStruct* pCurrObjInfo,
                          const CFrameContainer*  const  pFrame_curr_in,
                          Cordon_Par* Warning_Line_Pra,       //! 预警线参数
                          Cordon_Par* Alarm_Line_Pra );       //! 报警线参数

  /*判断是否越界，三种越界方式*/
  bool Judge_Slop_Over_Line(LabelObjStatus* pTrackObjInfo,
                            Cordon_Par* Slop_Over_Line_Pra,
                            ALARMTYPE alarm_type);

  /*区分人车，缺少算法，待完善*/
  int Differentiate_From_Car_Man(LabelObjStatus* pTrackObjInfo);

  /*画跟踪轨迹*/
  void Drawtrack(CFrameContainer*  pFrame_curr_in);

  /*输入两点坐标，输出两点确定直线的斜率和节距*/
  void Calculate_Line_Parameter(float &slppe,
                                float &pitch,
                                float pt1_x,
                                float pt1_y,
                                float pt2_x,
                                float pt2_y);
  /*画预警线、报警线*/
  void Draw_Warning_Line(Cordon_Par* Slop_Over_Line_Pra,
                         CFrameContainer* pFrame_in);

  /*pFrame_high隔行采样到pFrame_low*/
  ErrVal Interlaced_Scanning (CFrameContainer* pFrame_low,
                              const CFrameContainer* const pFrame_high,
                              uint8_t SampleIntervallines = 2);

  /*用pRGB_template做模板，将pFrame_in_out遮罩*/
  ErrVal  Shadow_Mask (CFrameContainer* pFrame_in_out,
                       const uint8_t* const pRGB_template);
  ErrVal  Shadow_Mask (CFrameContainer* pFrame_in,CFrameContainer* pFrame_out);

  void DeleteAdditionalLine();
  int ReadIni(char *FilePath,char* pSectionName,char *pKeyName) ;
  void GetIpAddress(sockaddr_in &sa);
  float ComputeObjHeigth(int ObjCoordinate,float slope1,
                          float pitch1,float slope2,float pitch2);

  void initTempImage(int iWidth, int iHeight, YUVTYPE YuvType_in);
};


#endif

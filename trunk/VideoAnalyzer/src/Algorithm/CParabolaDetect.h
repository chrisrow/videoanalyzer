/*! 
*************************************************************************************
* \file 
*    
* \brief
*    M<<???
* \date
*    2009_09_09
* \author
*    - Qi LIU                         <>
*    - Tianxiao YE                    <ytxjonathan@gmail.com>
*************************************************************************************
*/

#if !defined  (_CPARABOLADETECT_H__7A136E48_CC78_49e2_8C68_3BBA10A4701F)
#define _CPARABOLADETECT_H__7A136E48_CC78_49e2_8C68_3BBA10A4701F

//#include "stdafx.h"
// #include "..\..\CommonLibStatic\src\teamDebug.h"
#include "matlabFunc.h"

#include <math.h>
/*#include <string.h>*/

struct LineSet
{
  int bFlag ;
  int BeginPointX ;
  int BeginPointY ;
  int EndPointX;
  int EndPointY;
};

struct JudgeSet
{
  int FirstValue  ;
  int SecondValue ;
  int ThirdValue  ;
  int FouthValue  ;
};
struct JudgeSetFlt
{
  float FirstValue  ;
  float SecondValue ;
  float ThirdValue  ;
  float FouthValue  ;
};
struct ParamStruct
{
	int     iStyleChange               ;                  //0_natural 1_Curve 2_Above 3_Tree
	int     bSensitiveFlag            ;                  //灵敏度标志，3帧报警
	int     bTransLensImage           ;                  //镜像图像标志
	//  int     bLittleRegionFlag         ;                  //是否启用小区域的标志
	//  int     bTreeLittleFlag           ;                  //树一侧是否启用小区域的标志
	// float    fLineFirstLocation[4]     ;                  //目标穿越的两条直线参数 0：线1_A   1：线1_B   2：线2左       3：线2又
	//  float    fLineSecondLocation[4]    ;                  //拟合曲线需要的高度参数 0：线 A    1: 线B     2：线的最左端  3：线的最右端  
	int     iBinarizeSubThreshold     ;                  //二值化阈值  
	int     iNightSubThreshold        ;                  //夜间二值化阈值 
	int     iImfilterSingleThreshold  ;                  //去小点阈值
	int     iImdilateThreshold        ;                  //膨胀阈值
	int     iWhiteSpotNumMax          ;                  //最大白点数
	int     iWhiteSpotNumMin          ;                  //最小白点数
	int     iXTrackContinueThreshold  ;                  //判断目标改变方向的阈值
	int     iXTrackOffsetValue        ;                  //判断条件保证x方向有一定偏差
	int     iTrackMaxFrameNum         ;                  //跟踪最大帧数 
	int      iLittleRegionTop         ;                  //小区域中Y的最高点 
	int      iNightRangeVal            ;                 //判读是否到夜间的阈值
	int     iCurveLeftVal              ;                  //拟合曲线线的最左端
	int     iCurveRightVal             ;                  //拟合曲线线的最右端
	int     iPersonFlag               ;                  //人员检测标志
	int     iPersonWhitePotNum        ;                  //人员白点数目

	LineSet  tRectLittleRegion         ;                  //小区域范围   flag:是否启用小区域的标志 1 启用 0 不用
	LineSet  tRectTreeLittleRegion     ;                  //树一侧的小区域   flag: 树一侧是否启用小区域的标志 1 启用 0 不用   
	LineSet  tLineCurverRange          ;                  //拟合曲线范围 
	LineSet  tLineStraightFirst        ;                  //两条轨迹经过区域线中的第一条线 
	LineSet  tLineStraightSecond       ;                  //两条轨迹经过区域线中的第二条线 
	LineSet  tLineBlackLeft            ;                  //加入的黑色直线中的左边为黑 
	LineSet  tLineBlackRight           ;                 //加入的黑色直线中的右边为黑 

	LineSet  tRectBlackBlock[5]        ;                  //加入的黑色块范围 
	LineSet  tNightRange[2]            ;                 //夜间的区域值 用2块区域判断
	//float   iPersonRange[5][2]       ;                   //人员检测范围参数(梯形) 0-0高点 0-1未用 1左线高点 2左线低点 3右线高点 4右线低点  
	LineSet  iPersonRange[3]           ;                // 人员检测范围参数(梯形)左线 右线
//	int      iPersonTopVal             ;                // 人员检测范围参数高度
} ;

struct ParamDistinguish
{
  JudgeSet TrackNumMax ;    //TrackAlarmObject函数中判断跟踪次数的大值
  JudgeSet TrackNumMin ;   //TrackAlarmObject函数中判断跟踪次数的小值
  JudgeSet LostNumVal ;   //TrackAlarmObject函数中判断的丢帧数
  JudgeSet MigrationDiff;   //TrackAlarmObject函数中判断X每帧偏移量
  JudgeSet OriginCurrDis ;   //TrackAlarmObject函数中判断原始点到当前点的X距离
  JudgeSet YHeightVal  ;    //TrackAlarmObject函数中判断Y方向的最大距离  
  JudgeSet XContValue;     //TrackAlarmObject函数中判断X方向的同一方向运动数  
  JudgeSet XMatchValue;   //TrackAlarmObject函数中判断X方向的与预测的匹配个数
  JudgeSet YMatchValue;     // TrackAlarmObject函数中判断Y方向的与预测的匹配个数 
  JudgeSet FindObjValue;   //TrackAlarmObject函数中判断找到目标个数
  JudgeSet RiseNumVal;   //TrackAlarmObject函数中判断上升帧数
  JudgeSet DownNumVal;   //TrackAlarmObject函数中判断下降帧数
  JudgeSet FindObjValMax;   //TrackAlarmObject函数中判断找到目标目标的大值
  JudgeSet FindObjValMin;   //TrackAlarmObject函数中判断找到目标目标的小值
  JudgeSet WhiteSpotVal;   //TrackAlarmObject函数中判断白点数3个区段
  JudgeSet TopPointVal;   //TrackAlarmObject函数中判断Y方向的最大值


  //与JudgeValue类型一样 float型
  JudgeSetFlt fFindObjInverse;  //TrackAlarmObject函数中判断找到目标和跟踪数的比率 
  JudgeSetFlt fXMatchInverse;  //TrackAlarmObject函数中判断 X匹配数与跟踪数的比率  
  JudgeSetFlt fYMatchInverse;  //TrackAlarmObject函数中判断 Y匹配数与跟踪数的比率  
  JudgeSetFlt fXContInverse;  //TrackAlarmObject函数中判断 X同一方向数与跟踪数的比率 

} ;

class CParabolaDetect
{
public:
  CParabolaDetect(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in );  //constructor
  virtual ~CParabolaDetect();                              //--destructor

  unsigned long   m_WholeFrameNum ;                      //-- 全局读取文件时的帧数
  bool            m_AlarmFlg       ;                      //-- 报警标志

protected:

  static const ErrVal m_nTracked = 1;
  static const unsigned int  m_iTrackObjectMaxNum = 200 ;  //-- 最大的跟踪数目 

  struct LabelObjStatus 
  {
    float    fXOffset               ;                        //-- x offset distance 
    bool     bTrackAlarmFlag       ;                        //-- track alarm flag
    bool     bContinueTrackFlag    ;                        //-- continue track flag 
    bool     bObjDistanceFlg       ;                        //-- object distance to the camera
    bool     bLineRangeFlag[2]     ;                        //-- line left and right flag  . 0:left  1:right 
    int32_t  iWhiteSpotNum         ;                        //-- white spot number
    uint8_t  iTrackFrameNum        ;                        //-- object tracked frame number
    uint8_t  iRiseFrameNum[2]      ;                        //-- object ascend frame number  0:rise 1:descend 
    uint8_t  iTrackDirection[2]     ;                        //-- 0:x 1:y  if(t_track_direction[0] == 0) right; else left;  if(t_track_direction[1] == 0) Up; else Down;
    uint8_t  iLostFrameNum         ;                        //-- to indicate how many frames have lost
    uint16_t iFindObjNumber        ;                        //-- find object number
    uint16_t iXContinueNum[2]          ;                        //-- change number  0:x 1:y 
    uint16_t iForecastFrmCenter[2] ;                        //-- center_x,center_y of every label area
    uint16_t iPreFrameCenter[2]    ;                        //-- tracked object center ,current and previous frame 
    uint16_t iCurFrameCenter[2]    ;                        //-- cur frame center coordinate 0:x 1:y
    uint16_t iOriginFrameCenter[2] ;                        //-- origin frame center coordinate 0:x 1:y            
    uint16_t iMigrationDiff[2]      ;                        //-- average migration length of every frame 0:x 1:y 
    uint16_t iXFrameLocation[40]   ;                        //-- object upturn number
    uint16_t iYFrameLocation[40]   ;                        //-- object upturn number
    int16_t  iMatchNum[2]           ;                        //-- match number 0:x 1:y 
    uint16_t iAllLostFrameNum     ;                        //-- all lost frame  
    int16_t  iTrackTopPoint[3]     ;                        //-- track top point    0:x 1:y  2:frame number 
    int16_t  iTrackBottomPoint[3]  ;                        //-- track bottom point  0:x 1:y  2:frame number 
    int16_t  iLittleRegionNum   ;                        //-- little region number
    int16_t  iTreeLittleRegionNum ;                      //-- 数一侧的第二区域数目
  };

  LabelObjStatus TrackObject[m_iTrackObjectMaxNum];

  struct ObjLabelInfoStruct
  {
    bool t_flag                       ;
    uint16_t iObjLabelRect[4]      ;                        //-- top_x,top_y,width,height
    uint16_t iObjLabelCenter[2]    ;                        //-- Center
    uint32_t iObjWhiteSpotNum     ;                        //-- WHITE SPOT number of every label area
  };

  ObjLabelInfoStruct ObjLabelInfo[m_iTrackObjectMaxNum];
  ObjLabelInfoStruct PreLabelInfo[m_iTrackObjectMaxNum];
  ObjLabelInfoStruct AreaInfo[m_iTrackObjectMaxNum];
  ObjLabelInfoStruct AreaInfoMax;



  bool     m_bTrackedObjFlag          ;              //找到预测目标标志
  uint16_t m_iCurrFrmNum              ;              //当前帧得到目标数
  uint16_t m_iPreFrmNum               ;              //前一帧的目标数
  uint16_t m_iTrackNum                 ;              //跟踪目标数目
  uint16_t m_iFirFrmNum                   ;              //采集帧数
  uint16_t m_iFrmDifference            ;              //帧数差
  uint16_t m_iAreaFrmNum              ;              //区域目标数
  uint8_t* m_pContinueImage[5]         ;             //开辟的内存
  uint8_t* m_pShowImage[5]             ;             //开辟的内存
  uint8_t* m_pParaDetectImage[2]      ;
  uint8_t* m_pBlackBlockImage         ;              //开辟的内存
  uint16_t m_iFrameWidth               ;
  uint16_t m_iFrameHeight              ;
  int      m_iAlarmDelay               ;              //报警延时
  int      m_iPersonTimeDelay         ; 
  int      m_iPersonChangeBkTime     ;
  bool     m_bPersonDetectFlag        ;              //检测到人的标志
  int      m_iPersonFrmNum            ;              //采集帧数
  int16_t  m_iCircleFrmNum            ;
  bool     m_bCircleFrmFirst          ;
  int      m_ImfilterSingleThreshold  ;
  int      m_BinarizeSubThreshold     ;
  int      m_NightNumber              ;
  int      m_NightFlag                ;
  float    m_fLineFirstLocation[4]      ;                  //目标穿越的两条直线参数 0：线1_A   1：线1_B   2：线2左       3：线2又
  float    m_fLineSecondLocation[2]     ;                   //拟合曲线需要的高度参数 0：线 A    1: 线B

  /* methods */
public:
  ErrVal ParaDetectTwo(const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out);
  ErrVal ImgMoveObjectDetect(const CFrameContainer* const p_frame_in,CFrameContainer* const p_frame_out);

protected:
  ErrVal DetectedTrackedObject(const CFrameContainer* const pFrame_in, CFrameContainer* const pFrame_out, LabelObjStatus* pLabelObjStatus);
  void   ImfilterSingle(CFrameContainer* const pFrame_in, int SingleThreshold, uint16_t* const pRectangle = NULL);
  ErrVal BinarizeSub(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out,  int16_t v_threshold, uint16_t* pRectangle  = NULL);
  ErrVal GlobalLabelObject(CFrameContainer* const pFrame_binaried);
  ErrVal ForecastObjectDetect(const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out);
  void   RemoveBorder(CFrameContainer* const pFrame_in) ;
  ErrVal FindDetectedObject(CFrameContainer* const pFrame_in, ObjLabelInfoStruct* pLabelObjStatus);
  ErrVal Imerode(CFrameContainer* const pFrame_binaried, uint16_t* pRect = NULL,uint16_t* elementstr = NULL) const;
  void   Imdilate( CFrameContainer* const pFrame_in, uint32_t nDilateTimesPerPixel_in = 1, uint16_t* pRectangle = NULL);
  ErrVal AddBlackBlock(uint8_t* pFrame_in, int left, int bottom, int right, int top );
  BOOL   ThresholdChaseObject(CFrameContainer* const pFrame_in, CFrameContainer* const pFrame_out, LabelObjStatus* pTrackAreaObj );
  ErrVal ClearObjectBuffer();
  ErrVal averageSmoothRgb( const CFrameContainer *const pFrame_decoded_in,uint16_t nFilterSelect = 0,uint16_t StartSmoothLine = 0 ) const;//均值平滑
  ErrVal ChangeObjectValue( const CFrameContainer* const pFrame_in, ObjLabelInfoStruct* pLabelObjStatus, LabelObjStatus* pTrackAreaObj, bool FindObjectFlag );
  DWORD  OnRegRead(const char*  address , const char* str);
  void   InitParaVal( );
  ErrVal AddLeftBlackLine(uint8_t* pFrame_in, int x1, int y1, int x2, int y2 );
  ErrVal AddRightBlackLine(uint8_t* pFrame_in, int x1, int y1, int x2, int y2  );
  void   AddStraightLine1( int x1, int y1, int x2, int y2 );
  void   AddStraightLine2( int x1, int y1, int x2, int y2 );
  void   AddStraightLineCurver( int x1, int y1, int x2, int y2 );
  void   InverseImage(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out);
  ErrVal  StatisticsObjCurinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
    const  CFrameContainer* const pFrame_curr_in, const CFrameContainer* const pFrame_bin_in);
  bool NightMedol(unsigned char  *pSrc, LineSet *pRect1, LineSet *pRect2);

  int   InverseMatrix(double *matrix,const int &row);
  void  swap(double &a,double &b);
  bool  CurveFitting(double *matrix_x,double *matrix_y,double *matrix_A);

  virtual bool TrackAlarmObject(uint16_t i) = 0 ;
  virtual bool CurveContrast( LabelObjStatus* pTrackCurveInfo) =0 ;
  

  ErrVal AreaLabelObj(uint8_t*  pFrame_inout,uint16_t* const pRect);
  bool  PersonBinarizeSub(const CFrameContainer* const pFrame_in,  unsigned char * pBkImage, unsigned char* pSavImage ,int Threshold);
  bool  PersonShapeDetect( );
  void  AreaTargetCombinate( );
  long  PersonDetectWhiteSpot( uint8_t *p_image_src );
  void  PersonAddBlack( );
  void  PersonImfilterSingle(uint8_t* p_frame, uint16_t* const pRectangle = NULL ) ;
  void  PersonImdilate( uint8_t *p_frame,uint32_t nDilateTimesPerPixel_in,  uint16_t* pRectangle = NULL ) ;
  void  PersonCreateBK( uint8_t *p_frame ,int ScaleVal );
};

class CParabolaNatural : public CParabolaDetect
{
public:
  CParabolaNatural(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in):CParabolaDetect(nYWidth_in,  nYHeight_in){}
protected:
  bool TrackAlarmObject(uint16_t i);
  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);
};



class CParabolaCurve :public CParabolaDetect
{
public:
  CParabolaCurve(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in):CParabolaDetect(nYWidth_in,  nYHeight_in){}
protected: 
  bool TrackAlarmObject(uint16_t i);
  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);

};

class CParabolaAbove :public CParabolaDetect
{
public:
  CParabolaAbove(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in):CParabolaDetect(nYWidth_in,  nYHeight_in){}
protected:  
  bool TrackAlarmObject(uint16_t i);
  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);
};

class CParabolaTree :public CParabolaDetect
{
public:
  CParabolaTree(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in):CParabolaDetect(nYWidth_in,  nYHeight_in){}
protected:  
  bool TrackAlarmObject(uint16_t i);
  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);
};




#endif //#if !defined  (_CPERSONDETECT_H__261F00FA_6095_4a4f_81E1_61D01B095E1A)
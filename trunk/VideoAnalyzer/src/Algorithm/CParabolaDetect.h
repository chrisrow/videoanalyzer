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
	int     bSensitiveFlag            ;                  //�����ȱ�־��3֡����
	int     bTransLensImage           ;                  //����ͼ���־
	//  int     bLittleRegionFlag         ;                  //�Ƿ�����С����ı�־
	//  int     bTreeLittleFlag           ;                  //��һ���Ƿ�����С����ı�־
	// float    fLineFirstLocation[4]     ;                  //Ŀ�괩Խ������ֱ�߲��� 0����1_A   1����1_B   2����2��       3����2��
	//  float    fLineSecondLocation[4]    ;                  //���������Ҫ�ĸ߶Ȳ��� 0���� A    1: ��B     2���ߵ������  3���ߵ����Ҷ�  
	int     iBinarizeSubThreshold     ;                  //��ֵ����ֵ  
	int     iNightSubThreshold        ;                  //ҹ���ֵ����ֵ 
	int     iImfilterSingleThreshold  ;                  //ȥС����ֵ
	int     iImdilateThreshold        ;                  //������ֵ
	int     iWhiteSpotNumMax          ;                  //���׵���
	int     iWhiteSpotNumMin          ;                  //��С�׵���
	int     iXTrackContinueThreshold  ;                  //�ж�Ŀ��ı䷽�����ֵ
	int     iXTrackOffsetValue        ;                  //�ж�������֤x������һ��ƫ��
	int     iTrackMaxFrameNum         ;                  //�������֡�� 
	int      iLittleRegionTop         ;                  //С������Y����ߵ� 
	int      iNightRangeVal            ;                 //�ж��Ƿ�ҹ�����ֵ
	int     iCurveLeftVal              ;                  //��������ߵ������
	int     iCurveRightVal             ;                  //��������ߵ����Ҷ�
	int     iPersonFlag               ;                  //��Ա����־
	int     iPersonWhitePotNum        ;                  //��Ա�׵���Ŀ

	LineSet  tRectLittleRegion         ;                  //С����Χ   flag:�Ƿ�����С����ı�־ 1 ���� 0 ����
	LineSet  tRectTreeLittleRegion     ;                  //��һ���С����   flag: ��һ���Ƿ�����С����ı�־ 1 ���� 0 ����   
	LineSet  tLineCurverRange          ;                  //������߷�Χ 
	LineSet  tLineStraightFirst        ;                  //�����켣�����������еĵ�һ���� 
	LineSet  tLineStraightSecond       ;                  //�����켣�����������еĵڶ����� 
	LineSet  tLineBlackLeft            ;                  //����ĺ�ɫֱ���е����Ϊ�� 
	LineSet  tLineBlackRight           ;                 //����ĺ�ɫֱ���е��ұ�Ϊ�� 

	LineSet  tRectBlackBlock[5]        ;                  //����ĺ�ɫ�鷶Χ 
	LineSet  tNightRange[2]            ;                 //ҹ�������ֵ ��2�������ж�
	//float   iPersonRange[5][2]       ;                   //��Ա��ⷶΧ����(����) 0-0�ߵ� 0-1δ�� 1���߸ߵ� 2���ߵ͵� 3���߸ߵ� 4���ߵ͵�  
	LineSet  iPersonRange[3]           ;                // ��Ա��ⷶΧ����(����)���� ����
//	int      iPersonTopVal             ;                // ��Ա��ⷶΧ�����߶�
} ;

struct ParamDistinguish
{
  JudgeSet TrackNumMax ;    //TrackAlarmObject�������жϸ��ٴ����Ĵ�ֵ
  JudgeSet TrackNumMin ;   //TrackAlarmObject�������жϸ��ٴ�����Сֵ
  JudgeSet LostNumVal ;   //TrackAlarmObject�������жϵĶ�֡��
  JudgeSet MigrationDiff;   //TrackAlarmObject�������ж�Xÿ֡ƫ����
  JudgeSet OriginCurrDis ;   //TrackAlarmObject�������ж�ԭʼ�㵽��ǰ���X����
  JudgeSet YHeightVal  ;    //TrackAlarmObject�������ж�Y�����������  
  JudgeSet XContValue;     //TrackAlarmObject�������ж�X�����ͬһ�����˶���  
  JudgeSet XMatchValue;   //TrackAlarmObject�������ж�X�������Ԥ���ƥ�����
  JudgeSet YMatchValue;     // TrackAlarmObject�������ж�Y�������Ԥ���ƥ����� 
  JudgeSet FindObjValue;   //TrackAlarmObject�������ж��ҵ�Ŀ�����
  JudgeSet RiseNumVal;   //TrackAlarmObject�������ж�����֡��
  JudgeSet DownNumVal;   //TrackAlarmObject�������ж��½�֡��
  JudgeSet FindObjValMax;   //TrackAlarmObject�������ж��ҵ�Ŀ��Ŀ��Ĵ�ֵ
  JudgeSet FindObjValMin;   //TrackAlarmObject�������ж��ҵ�Ŀ��Ŀ���Сֵ
  JudgeSet WhiteSpotVal;   //TrackAlarmObject�������жϰ׵���3������
  JudgeSet TopPointVal;   //TrackAlarmObject�������ж�Y��������ֵ


  //��JudgeValue����һ�� float��
  JudgeSetFlt fFindObjInverse;  //TrackAlarmObject�������ж��ҵ�Ŀ��͸������ı��� 
  JudgeSetFlt fXMatchInverse;  //TrackAlarmObject�������ж� Xƥ������������ı���  
  JudgeSetFlt fYMatchInverse;  //TrackAlarmObject�������ж� Yƥ������������ı���  
  JudgeSetFlt fXContInverse;  //TrackAlarmObject�������ж� Xͬһ��������������ı��� 

} ;

class CParabolaDetect
{
public:
  CParabolaDetect(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in );  //constructor
  virtual ~CParabolaDetect();                              //--destructor

  unsigned long   m_WholeFrameNum ;                      //-- ȫ�ֶ�ȡ�ļ�ʱ��֡��
  bool            m_AlarmFlg       ;                      //-- ������־

protected:

  static const ErrVal m_nTracked = 1;
  static const unsigned int  m_iTrackObjectMaxNum = 200 ;  //-- ���ĸ�����Ŀ 

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
    int16_t  iTreeLittleRegionNum ;                      //-- ��һ��ĵڶ�������Ŀ
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



  bool     m_bTrackedObjFlag          ;              //�ҵ�Ԥ��Ŀ���־
  uint16_t m_iCurrFrmNum              ;              //��ǰ֡�õ�Ŀ����
  uint16_t m_iPreFrmNum               ;              //ǰһ֡��Ŀ����
  uint16_t m_iTrackNum                 ;              //����Ŀ����Ŀ
  uint16_t m_iFirFrmNum                   ;              //�ɼ�֡��
  uint16_t m_iFrmDifference            ;              //֡����
  uint16_t m_iAreaFrmNum              ;              //����Ŀ����
  uint8_t* m_pContinueImage[5]         ;             //���ٵ��ڴ�
  uint8_t* m_pShowImage[5]             ;             //���ٵ��ڴ�
  uint8_t* m_pParaDetectImage[2]      ;
  uint8_t* m_pBlackBlockImage         ;              //���ٵ��ڴ�
  uint16_t m_iFrameWidth               ;
  uint16_t m_iFrameHeight              ;
  int      m_iAlarmDelay               ;              //������ʱ
  int      m_iPersonTimeDelay         ; 
  int      m_iPersonChangeBkTime     ;
  bool     m_bPersonDetectFlag        ;              //��⵽�˵ı�־
  int      m_iPersonFrmNum            ;              //�ɼ�֡��
  int16_t  m_iCircleFrmNum            ;
  bool     m_bCircleFrmFirst          ;
  int      m_ImfilterSingleThreshold  ;
  int      m_BinarizeSubThreshold     ;
  int      m_NightNumber              ;
  int      m_NightFlag                ;
  float    m_fLineFirstLocation[4]      ;                  //Ŀ�괩Խ������ֱ�߲��� 0����1_A   1����1_B   2����2��       3����2��
  float    m_fLineSecondLocation[2]     ;                   //���������Ҫ�ĸ߶Ȳ��� 0���� A    1: ��B

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
  ErrVal averageSmoothRgb( const CFrameContainer *const pFrame_decoded_in,uint16_t nFilterSelect = 0,uint16_t StartSmoothLine = 0 ) const;//��ֵƽ��
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
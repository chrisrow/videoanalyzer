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


// class CParabolaDetect : public IFrameReceiver, public CSubject<IAlerter*>
class CParabolaDetect
{
public:
  CParabolaDetect(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in, unsigned int const  nChannel);  //constructor
  virtual ~CParabolaDetect();                              //--destructor

//   // jiangqi //////////////////////////////////////////////////////////
// private:
//   CFrameContainer *m_pFrameContainer, *m_pOutFrameContainer, *m_pFrame_matlabFunced;
// 
// public:
//   virtual void updateFrame(const IplImage *pFrame) ;
//   // jiangqi //////////////////////////////////////////////////////////

  unsigned long   m_whole_frame_num ;                      //-- 全局读取文件时的帧数
  bool            m_alarm_flg       ;                      //-- 报警标志

protected:

  static const ErrVal m_nTracked = 1;
  static const unsigned int  m_nTrackObjectMaxNum = 200 ;  //-- 最大的跟踪数目 

  struct LabelObjStatus 
  {
    float    t_offset_x               ;                        //-- x offset distance 
  	bool     t_track_alarm_flag       ;                        //-- track alarm flag
    bool     t_continue_track_flag    ;                        //-- continue track flag 
    bool     t_obj_distance_flg       ;                        //-- object distance to the camera
    bool     t_line_range_flag[2]     ;                        //-- line left and right flag  . 0:left  1:right 
    int32_t  t_white_spot_num         ;                        //-- white spot number
    uint8_t  t_track_frame_num        ;                        //-- object tracked frame number
    uint8_t  t_rise_frame_num[2]      ;                        //-- object ascend frame number  0:rise 1:descend 
    uint8_t  t_track_direction[2]     ;                        //-- 0:x 1:y  if(t_track_direction[0] == 0) right; else left;  if(t_track_direction[1] == 0) Up; else Down;
    uint8_t  t_lost_frame_num         ;                        //-- to indicate how many frames have lost
    uint16_t t_find_obj_number        ;                        //-- find object number
    uint16_t t_x_continue_num[2]          ;                        //-- change number  0:x 1:y 
    uint16_t t_forecast_frm_center[2] ;                        //-- center_x,center_y of every label area
    uint16_t t_pre_frame_center[2]    ;                        //-- tracked object center ,current and previous frame 
    uint16_t t_cur_frame_center[2]    ;                        //-- cur frame center coordinate 0:x 1:y
    uint16_t t_origin_frame_center[2] ;                        //-- origin frame center coordinate 0:x 1:y            
    uint16_t t_migration_diff[2]      ;                        //-- average migration length of every frame 0:x 1:y 
    uint16_t t_x_frame_location[40]   ;                        //-- object upturn number
    uint16_t t_y_frame_location[40]   ;                        //-- object upturn number
    int16_t  t_match_num[2]           ;                        //-- match number 0:x 1:y 
    uint16_t t_all_lost_frame_num     ;                        //-- all lost frame  
	  int16_t  t_track_top_point[3]     ;                        //-- track top point    0:x 1:y  2:frame number 
	  int16_t  t_track_bottom_point[3]  ;                        //-- track bottom point  0:x 1:y  2:frame number 
    int16_t  t_region_little_number   ;                        //-- little region number
  };

  LabelObjStatus TrackObject[m_nTrackObjectMaxNum];

  struct ObjLabelInfoStruct
  {
    bool t_flag                       ;
    uint16_t t_obj_label_rect[4]      ;                        //-- top_x,top_y,width,height
    uint16_t t_obj_label_center[2]    ;                        //-- Center
    uint32_t t_obj_white_spot_num     ;                        //-- WHITE SPOT number of every label area
  };

  ObjLabelInfoStruct ObjLabelInfo[m_nTrackObjectMaxNum];
  ObjLabelInfoStruct PreLabelInfo[m_nTrackObjectMaxNum];
  ObjLabelInfoStruct AreaInfo[m_nTrackObjectMaxNum];
  ObjLabelInfoStruct AreaInfoMax;

//   struct ParamStruct
//   {
//     bool     b_little_region_flag      ;                  //是否启用小区域的标志
//     float    t_line_first_location[4]  ;                  //目标穿越的两条直线参数 0：线1_A   1：线1_B   2：线2左       3：线2又
//     float    t_line_second_location[4] ;                  //拟合曲线需要的高度参数 0：线 A    1: 线B     2：线的最左端  3：线的最右端    
//     int8_t   binarize_sub_threshold    ;                  //二值化阈值  
//     uint8_t  imfilter_single_threshold ;                  //去小点阈值
//     uint8_t  imdilate_threshold        ;                  //膨胀阈值
//     uint16_t white_spot_num_max        ;                  //最大白点数
//     uint16_t white_spot_num_min        ;                  //最小白点数
//     uint16_t track_x_continue_threshold;                  //判断目标改变方向的阈值
//     uint16_t track_x_offset_value      ;                  //判断条件保证x方向有一定偏差
//     uint16_t track_max_frame_num       ;                  //跟踪最大帧数 
//     int      little_region_range[4]    ;                  //0：left_x 1：LEFT_Y 2:RIGHT_X 3:RIGHT_Y 
//     float    little_region_value       ;                  //小区域中Y的最高点
//     float    person_range[5][2]        ;                  //人员检测范围参数
//   } ParamSet;

  struct ParamStruct
  {
      bool     b_little_region_flag      ;                  //是否启用小区域的标志
      float    t_line_first_location[4]  ;                  //目标穿越的两条直线参数 0：线1_A   1：线1_B   2：线2左       3：线2又
      float    t_line_second_location[4] ;                  //拟合曲线需要的高度参数 0：线 A    1: 线B     2：线的最左端  3：线的最右端    
      int   binarize_sub_threshold    ;                  //二值化阈值  
      int  imfilter_single_threshold ;                  //去小点阈值
      int  imdilate_threshold        ;                  //膨胀阈值
      int white_spot_num_max        ;                  //最大白点数
      int white_spot_num_min        ;                  //最小白点数
      int track_x_continue_threshold;                  //判断目标改变方向的阈值
      int track_x_offset_value      ;                  //判断条件保证x方向有一定偏差
      int track_max_frame_num       ;                  //跟踪最大帧数 
      int      little_region_range[4]    ;                  //0：left_x 1：LEFT_Y 2:RIGHT_X 3:RIGHT_Y 
      float    little_region_value       ;                  //小区域中Y的最高点
      float    person_range[5][2]        ;                  //人员检测范围参数
  } ParamSet;

  bool     m_tracked_obj_flag          ;              //找到预测目标标志
  uint8_t  m_fir_frm_num               ;              //记录最开始的几帧，用于背景叠加
  uint16_t m_curr_frm_num              ;              //当前帧得到目标数
  uint16_t m_pre_frm_num               ;              //前一帧的目标数
  uint16_t m_track_num                 ;              //跟踪目标数目
  uint16_t m_frm_num                   ;              //采集帧数
  uint16_t m_frm_difference            ;              //帧数差
  uint16_t m_area_frm_num              ;              //区域目标数
  uint8_t* m_continue_image[5]         ;             //开辟的内存
  uint8_t* m_show_image[5]             ;             //开辟的内存
  uint8_t* m_para_detect_image[2]      ;
  uint8_t* m_black_block_image         ;              //开辟的内存
  uint16_t m_frame_width               ;
  uint16_t m_frame_height              ;
  int      m_alarm_delay               ;              //报警延时
  int      m_person_time_delay         ; 
  int      m_person_change_bk_time     ;
  bool     m_person_change_bk_flag     ;
  bool     m_person_detect_flag        ;
  int      m_person_frm_num            ;              //采集帧数
  int16_t  m_circle_frm_num            ;
  bool     m_circle_frm_first          ;
  int16_t  m_smooth_rgb_number         ;

   /* methods */
public:
  ErrVal ParaDetectTwo(const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out);
  ErrVal ImgMoveObjectDetect(const CFrameContainer* const p_frame_in,CFrameContainer* const p_frame_out);

protected:
  ErrVal DetectedTrackedObject(const CFrameContainer* const pFrame_in, CFrameContainer* const pFrame_out, LabelObjStatus* pLabelObjStatus);
  void   ImfilterSingle(CFrameContainer* const pFrame_in,uint16_t* const pRectangle = NULL);
  ErrVal BinarizeSub(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out,  int16_t v_threshold, uint16_t* pRectangle  = NULL);
  ErrVal GlobalLabelObject(CFrameContainer* const pFrame_binaried);
  ErrVal ForecastObjectDetect(const CFrameContainer* const pFrame_in,CFrameContainer* const pFrame_out);
  void   RemoveBorder(CFrameContainer* const pFrame_in) ;
  ErrVal FindDetectedObject(CFrameContainer* const pFrame_in, ObjLabelInfoStruct* pLabelObjStatus);
  ErrVal Imerode(CFrameContainer* const pFrame_binaried, uint16_t* pRect = NULL,uint16_t* elementstr = NULL) const;
  void   Imdilate( CFrameContainer* const pFrame_in, uint32_t nDilateTimesPerPixel_in = 1, uint16_t* pRectangle = NULL);
  ErrVal AddBlackBlock(uint8_t* pFrame_in, uint16_t left, uint16_t bottom, uint16_t right, uint16_t top );
  BOOL   ThresholdChaseObject(CFrameContainer* const pFrame_in, CFrameContainer* const pFrame_out, LabelObjStatus* pTrackAreaObj );
  ErrVal ClearObjectBuffer();
  ErrVal averageSmoothRgb( const CFrameContainer *const pFrame_decoded_in,uint16_t nFilterSelect = 0,uint16_t StartSmoothLine = 0 ) const;//均值平滑
  ErrVal ChangeObjectValue( const CFrameContainer* const pFrame_in, ObjLabelInfoStruct* pLabelObjStatus, LabelObjStatus* pTrackAreaObj, bool FindObjectFlag );
  DWORD  OnRegRead(const char*  address , const char* str);
  void   InitParaVal( const char* address );
  void   InitParaRead( const int ChannelNum );
  ErrVal AddBlackLine(uint8_t* pFrame_in, float x1, float y1, float x2, float y2  );
  void   AddStraightLine1( float x1, float y1, float x2, float y2 );
  void   AddStraightLine2( float x1, float y1, float x2, float y2 );
  void   AddStraightLineCurver( float x1, float y1, float x2, float y2 );
  ErrVal  Statistics_obj_curinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
    const  CFrameContainer* const pFrame_curr_in, const CFrameContainer* const pFrame_bin_in);

  int   InverseMatrix(double *matrix,const int &row);
  void  swap(double &a,double &b);
  bool  CurveFitting(double *matrix_x,double *matrix_y,double *matrix_A);

  virtual bool TrackAlarmObject(uint16_t i) = 0 ;
  virtual bool CurveContrast( LabelObjStatus* pTrackCurveInfo) =0 ;

  ErrVal AreaLabelObj(uint8_t*  pFrame_inout,uint16_t* const pRect);
  bool  PersonBinarizeSub(const CFrameContainer* const pFrame_in,  CFrameContainer* const pFrame_out,  int16_t Threshold, uint16_t* pRectangle  = NULL);
  bool  PersonShapeDetect( );
  void  AreaTargetCombinate( );
  long  PersonDetectWhiteSpot( uint8_t *p_image_src );
  void  PersonAddBlack( );
  void  PersonImfilterSingle(uint8_t* p_frame, uint16_t* const pRectangle = NULL ) ;
  void  PersonImdilate( uint8_t *p_frame,uint32_t nDilateTimesPerPixel_in,  uint16_t* pRectangle = NULL ) ;
};

class CParabolaNatural : public CParabolaDetect
{
public:
  CParabolaNatural(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in, unsigned int const  nChannel):CParabolaDetect(nYWidth_in,  nYHeight_in, nChannel){}
protected:
  bool TrackAlarmObject(uint16_t i);
  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);
};



class CParabolaCurve :public CParabolaDetect
{
public:
  CParabolaCurve(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in, unsigned int const  nChannel):CParabolaDetect(nYWidth_in,  nYHeight_in, nChannel){}
protected: 
  bool TrackAlarmObject(uint16_t i);
  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);

};

//class CParabolaTree :public CParabolaDetect
//{
//public:
//  CParabolaTree(unsigned int const  nYWidth_in, unsigned int const  nYHeight_in, unsigned int const  nChannel):CParabolaDetect(nYWidth_in,  nYHeight_in, nChannel){}
//protected:  
//  bool TrackAlarmObject(uint16_t i);
//  bool CurveContrast( LabelObjStatus* pTrackCurveInfo);
//};



#endif //#if !defined  (_CPERSONDETECT_H__261F00FA_6095_4a4f_81E1_61D01B095E1A)
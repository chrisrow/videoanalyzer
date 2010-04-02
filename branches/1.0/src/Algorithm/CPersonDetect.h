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
#define  SAMPLING_INTERVAL    5             //  ���������ÿ��֡����һ֡��
#define  BKUPDATE_INTERVAL    10             //  ���������ÿ��֡����һ֡��
#define  MAXTHRESHOLD         70            //  ��һ�β�ִ���ֵ(����)
#define  MINTHRESHOLD         45            //  �ڶ��β��С��ֵ(����)
#define  SMOOTHSTARTLINE      80            //  ƽ����ʼ����
#define  BKFRAMEINTERVAL      40            //  ���������ںϼ��
#define  BKFRAMEBUFSZ         7             //  ����BUFFER����
#define  WHITERATIO           90            //  �׵���>40%ʱ��������������
#define  MAXFRAMENUM          4000000000    //  (֡��������)nInputFrameNum>= MAXFRAMENUMʱ�����¼���
#define  PARABOLAWIDTHNEAR    3 //40  70   //������Ӿ��γ������ֵ  ����    
#define  PARABOLAHEIGHTNEAR   10//40  70                                                 
#define  PARABOLAWIDTHFAR     3//40         //������Ӿ��γ������ֵ  Զ��   
#define  PARABOLAHEIGHTFAR    3//40
#define  DEFAULTCHANNELMODE   9             // Ĭ��ͨ����Ϊ9�����ڵ�·���ԣ�����ֵ����̵߳�m_nChannel_ID�������
#define  ABS(a,b) ((a) >= (b) ? (a) - (b) : (b) - (a));

public:
  //===== constructor, destructor =====
  CPersonDetect(uint16_t const  nYWidth_in, uint16_t const  nYHeight_in, YUVTYPE const  YuvType_in,const uint8_t nChannel = DEFAULTCHANNELMODE);
  ~CPersonDetect(); 
  //====================================

public:/*�󱳾�*/
  uint32_t       m_nRefreshFrameNum;         //<! the number of accumulated incoming frames. Range: [0, m_nGraspFrameInterval)
private:
  uint32_t       m_nGraspFrameInterval;       //<! the frame interval between grasping one frame. Range: [0, ...), '0' for continuous frame grasping.
  uint32_t       m_nGraspFramesBufferSize;    //<! the size of m_GraspFramesBuffer
  CFramesBuffer* m_GraspFramesBuffer;         //<! store the grasped frames to generate a Bkgnd
  uint32_t       CurFrameNum;                 //<! uint32_t [0 , 2^32-1(4294967296-1)];

public:
  /*�ںϱ�����ʼ��*/
  ErrVal initBkdet( uint16_t const  nYWidth_in, 
                    uint16_t const  nYHeight_in, 
                    YUVTYPE const  YuvType_in); // init function

  ErrVal clearBkdet();
  /*�ںϱ���*/
  ErrVal generateRgbBkgnd_fromRgbFrames( CFrameContainer* frame_bkDetected,
                                        const CFrameContainer* const pFrame_curr_in,
                                        const CFrameContainer* const binBkgnd);  

/*�±����㷨*/
  static const int m_nbufsz = 4;
  uint8_t*  m_nGetBKBuffer[m_nbufsz];
  /*���±�����ʼ��*/
  ErrVal initUpdateBk( uint16_t const  nYWidth_in, 
                       uint16_t const  nYHeight_in, 
                       YUVTYPE const  YuvType_in); // init function

  ErrVal clearUpdateBk();
  /*���±���*/
  ErrVal UpdateBk(CFrameContainer* pFrame_BK_in_out,
                  const CFrameContainer* const pFrame_in,
                  const uint16_t bin_threshold,
                  const uint16_t dot_rate,
                  const uint16_t dot_rate_connection,
                  const uint32_t framenum);
    
private:/*����*/

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
  /*����������Χ����ʼ��*/
  ErrVal  init_pudong (uint16_t const  nYWidth_in, 
                       uint16_t const  nYHeight_in, 
                       YUVTYPE const  YuvType_in);
  ErrVal  clear_pudong();
/*ƽ��*/
  ErrVal averagesmoothRgb( CFrameContainer* pFrame_RgbSmoothed_inout , \
                            const CFrameContainer*  const pFrame_decoded_in , \
                            const uint16_t NoSmoothLineNum = 1 ) const;//��ֵƽ��

  //��֡ͼ�����ֵ���
  ErrVal binarizeY_fromRgbBkgnd( CFrameContainer* pFrame_RgbtoYBinarized_inout, \
                                const CFrameContainer* const pFrame_curr_in, \
                                const CFrameContainer* const pFrame_bkgnd_in,
                                uint16_t RgbThreshold) const;
  //��֡ͼ��С��ֵ���
  ErrVal  binarizeRgbtoY( CFrameContainer* pFrame_RgbtoYBinarized_inout, const CFrameContainer* const  pFrame_curr_in, \
                          const CFrameContainer* const pFrame_bkgnd_in, \
                          const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                          uint16_t Large_RgbThreshold ,
                          uint16_t Small_RgbThreshold);

   /*��С�ֱ��ʲ�ֲ��궨����binRgbtoY_LowtoHigh�ڴ�ֱ���ͼ���������
   *
   *    �궨�󣬵õ��ֿ�������Ӿ��κ����ֵͼ��
   *    ���ݾ��ο������λ�ò�ͬ������Զ���ͽ������趨��ͬ��ֵ��Զ����ֵ�ͣ�������ֵ�ߡ�
   *    �����ο�ķ�Χ�Ŵ󣬽���С��ֵ��֡�
   *    demarcation_line:ͼ��Զ�����ֽ���
   */
  ErrVal  binRgbtoY_LowtoHigh( CFrameContainer* pFrame_RgbtoYBinarized_inout , 
                              const CFrameContainer* const pFrame_curr_in, //  
                              const CFrameContainer*  const pFrame_bkgnd_in, //
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_inout,//
                              uint16_t Large_RgbThreshold ,
                              uint16_t Small_RgbThreshold,
                              const uint16_t  demarcation_line);
/*��ȡ������ֵ��ͼ���й�������*/
  ErrVal  extract_public_part(CFrameContainer* pFrame_bin_data_out,
                              const CFrameContainer* const pFrame_bin_bk,
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_bin_bk,
                              const CDList< CObjLabeled*, CPointerDNode >* objDList_bin_pre);

 /*��ʴ,��ͨ3*3��ʴ*/
  ErrVal erodeY( const CFrameContainer* const pFrame_inout,  //
                uint32_t nErodeTimesPerPixel_in ,   //��ʴ����
                uint16_t StartLineNum = 1) const; // ��ʴ��ʼ��
   /*
   *  ��ʴ��������Ϊ255�ĵ㣬�����Χ255��ĸ���>nVotedOne_in���õ���255��������0��
   *  nMatrixWidth*nMatrixWidth��ȫ1����ģ�壻
   *  nErodeStep����ʴ������
   */
  void erodeY( CFrameContainer*  const pYuvframe, 
                const uint8_t nMatrixWidth, 
                const uint8_t nVotedOne_in, 
                uint16_t StartLineNum = 1,
                const uint8_t nErodeStep = 1)const;

/*����*/
  ErrVal dilateY( const CFrameContainer* const pFrame_in, 
                uint32_t nDilateTimesPerPixel_in,
                uint16_t NoDilateLineNum  = 1) const;

  /*ֻ��ʾ��ɫ�˶�����*/
  void  RGBmulY(CFrameContainer*   pFrame_input , 
                const CFrameContainer* const pFrame_matlabFunced );

  /*���������ӿڣ�ȥ����Ա���֣�������Ա������ĸ���*/
  void  binremovebody_list(CFrameContainer* pFrame_matlabFunced , 
                          const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout);
  /*ȥ��СĿ��
  *    demarcation_line:ͼ��Զ�����ֽ���
  */
  void  deletminobj(CDList< CObjLabeled*, CPointerDNode >* objDList_inout,
                    const uint16_t demarcation_line);

  /*����ָ�����η�Χ�ڣ�Ŀ���RGB��ֵ*/
  void Calculate_BGR_Mean( uint8_t& b_mean,uint8_t& g_mean,uint8_t& r_mean,//
                          const CFrameContainer* const pFrame_curr_in,
                          const CFrameContainer* const pFrame_bin_in,
                          uint16_t top_x,uint16_t top_y,uint16_t bottom_x,uint16_t bottom_y);         

  enum ALARMTYPE   //!< Խ�߱�����ʽ
  {
    LEFTTORIGNT,   //!< ������      [- , +]
    RIGHTTOLEFT,  //!< ���ҵ���       [+ , -]
    DOUBLE_DIRECTION, //!< ˫��Խ��   [+ , -] || [- , +]
  }; 

public:/*����������*/

    ErrVal  PersenDetect_Process(CFrameContainer* pFrame_matlabFunced,  
                                 CFrameContainer* pRgbhumaninfo,
                                 CFrameContainer*    pFrame_input,  
                                 const uint8_t* const pRGB_template,  
                                 const ALARMTYPE alarm_type,
                                 uint16_t demarcation_line,
                                 const uint32_t framenum = 0); 


  /*************************************
  *        Ŀ��ƥ�䡢������Ԥ��
  *  
  *
  **************************************/
private:

  static const uint16_t m_nTrackObjectMaxNum = 200;  //! ������Ŀ����
  static const uint8_t m_nTrackObjShowNum = 10;      //! ��ʾ���ٹ켣����
  static const uint8_t m_nMaxCameraNum = 4;          //! ͬʱ����ͨ����

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
    uint16_t m_size_Plus[2];              // Ŀ����Ӿ��εĳ�����Ϣ  [sum_width,sum_height];
    uint16_t m_nTrack_pt[2][m_nTrackObjShowNum];  //������ٹ켣�㣬��Ŀ����Ӿ������½�������䡣��Ŀ���������m_nForecastCenterת�������½�������䡣
    uint8_t track_pot_count;
    //dubing add 20091217												  
    bool  b_Warning;            //һ�������źţ��澯���澯�ź�
    bool  b_Alarm;        //���������źţ�Ԥ�����澯�ź�
    //add end
  };

  LabelObjStatus TrackObject[m_nTrackObjectMaxNum];

  struct ObjLabelInfoStruct
  {
    bool flag                 ;
    uint16_t ObjLabelRect[4]  ;          // top_x,top_y,width,height
    uint16_t ObjLabelCenter[2];          // Center
    uint32_t WhiteSpotNum     ;          // WHITE SPOT number of every label area
    uint8_t ObjBGRmeanValue[3]   ;       //<! [BMV, GMV, RMV]  B.G.R ��ֵ
  };

  ObjLabelInfoStruct ObjLabelInfo[m_nTrackObjectMaxNum];
  ObjLabelInfoStruct PreLabelInfo[m_nTrackObjectMaxNum];

  bool     m_tracked_obj_flg     ;           //�ҵ�Ԥ��Ŀ���־
  uint16_t m_curr_frm_num        ;           //��ǰ֡�õ�Ŀ����
  uint16_t m_pre_frm_num         ;           //ǰһ֡��Ŀ����
  uint16_t m_TrackNum            ;           //����Ŀ����Ŀ

  static const ErrVal m_nLost     = 0;
  static const ErrVal m_nTracked  = 1;
  static const ErrVal m_nRemove   = 2;

  struct  Cordon_Par                //! Ԥ���߲����ṹ��
  {
    float         m_slope;          //! б��
    float         m_pitch;          //! �ؾ�
    uint16_t      m_distancetotop;    //! ��ͼ�񶥲��ľ��루���أ�
    uint16_t      m_distancetobottom;
  };
  Cordon_Par  Warning_Line[m_nMaxCameraNum];
  Cordon_Par  Alarm_Line[m_nMaxCameraNum];
  Cordon_Par  Border_line[m_nMaxCameraNum];


  //ֻ��ÿ���켣���������ﲻ��Ҫ��dubing
  bool  b_First_Alarm;    //һ�������ź�
  bool  b_Second_Alarm;    //���������ź�

  uint32_t delay_counter;

  CFrameContainer*   m_pFrame_RgbSmoothed_low;
  CFrameContainer*   m_pFrame_bkgndDetected_low;
  CFrameContainer*   m_pFrame_matlabFunced_low;

public:

    void   setDelay_counter()                              { delay_counter = 0; }

 /*����ƶ�Ŀ��  ָ�����򻭿�*/
 ErrVal ImgMoveObjectDetect(CFrameContainer* p_frame_in_out);

 /*Ŀ��ƥ��͸��ٳ�ʼ��*/
  ErrVal init_match_track();
  /*ͳ��Ŀ����Ϣ*/
  ErrVal Statistics_obj_curinfo (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout,
                                  const  CFrameContainer* const pFrame_curr_in,
                                  const CFrameContainer* const pFrame_bin_in);//�����ֵ��ͼ��

  /* Ŀ��ƥ������ٲ��� */
  ErrVal ForecastObjectDetect (const CDList< CObjLabeled*, CPointerDNode >* const objDList_inout ,
                              CFrameContainer* pFrame_in,
                              const CFrameContainer* const pFrame_bin_in,
                              Cordon_Par* Warning_Line_Pra,       //! Ԥ���߲���
                              Cordon_Par* Alarm_Line_Pra,
                              const ALARMTYPE alarm_type);       //! �����߲���

  ErrVal DetectedTrackedObject(LabelObjStatus* pLabelObjStatus,
                              const CFrameContainer*  const  pFrame_curr_in,
                              Cordon_Par* Warning_Line_Pra,       //! Ԥ���߲���
                              Cordon_Par* Alarm_Line_Pra ,
                              const ALARMTYPE alarm_type) ;      //! �����߲���

  ErrVal ChangeObjectValue(ObjLabelInfoStruct* pLabelObjStatus, 
                            LabelObjStatus* pTrackObjInfo, 
                            bool FindObjectFlag,
                            const CFrameContainer*  const  pFrame_curr_in,
                            Cordon_Par* Warning_Line_Pra,       //! Ԥ���߲���
                            Cordon_Par* Alarm_Line_Pra ,
                            const ALARMTYPE alarm_type);       //! �����߲���

  ErrVal FindDetectedObject(ObjLabelInfoStruct* pCurrObjInfo,
                          const CFrameContainer*  const  pFrame_curr_in,
                          Cordon_Par* Warning_Line_Pra,       //! Ԥ���߲���
                          Cordon_Par* Alarm_Line_Pra );       //! �����߲���

  /*�ж��Ƿ�Խ�磬����Խ�緽ʽ*/
  bool Judge_Slop_Over_Line(LabelObjStatus* pTrackObjInfo,
                            Cordon_Par* Slop_Over_Line_Pra,
                            ALARMTYPE alarm_type);

  /*�����˳���ȱ���㷨��������*/
  int Differentiate_From_Car_Man(LabelObjStatus* pTrackObjInfo);

  /*�����ٹ켣*/
  void Drawtrack(CFrameContainer*  pFrame_curr_in);

  /*�����������꣬�������ȷ��ֱ�ߵ�б�ʺͽھ�*/
  void Calculate_Line_Parameter(float &slppe,
                                float &pitch,
                                float pt1_x,
                                float pt1_y,
                                float pt2_x,
                                float pt2_y);
  /*��Ԥ���ߡ�������*/
  void Draw_Warning_Line(Cordon_Par* Slop_Over_Line_Pra,
                         CFrameContainer* pFrame_in);

  /*pFrame_high���в�����pFrame_low*/
  ErrVal Interlaced_Scanning (CFrameContainer* pFrame_low,
                              const CFrameContainer* const pFrame_high,
                              uint8_t SampleIntervallines = 2);

  /*��pRGB_template��ģ�壬��pFrame_in_out����*/
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

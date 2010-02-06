#pragma once

//抛物检测配置参数的类型定义

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
    int     bSensitiveFlag            ;                  //灵敏度标志，3帧报警
    int     bTransLensImage           ;                  //镜像图像标志
    int     bLittleRegionFlag         ;                  //是否启用小区域的标志
    int     bTreeLittleFlag           ;                  //树一侧是否启用小区域的标志
    float    fLineFirstLocation[4]     ;                  //目标穿越的两条直线参数 0：线1_A   1：线1_B   2：线2左       3：线2又
    float    fLineSecondLocation[4]    ;                  //拟合曲线需要的高度参数 0：线 A    1: 线B     2：线的最左端  3：线的最右端    
    int   bBinarizeSubThreshold     ;                  //二值化阈值  
    int   bNightSubThreshold        ;                  //夜间二值化阈值 
    int  iImfilterSingleThreshold  ;                  //去小点阈值
    int  iImdilateThreshold        ;                  //膨胀阈值
    int iWhiteSpotNumMax          ;                  //最大白点数
    int iWhiteSpotNumMin          ;                  //最小白点数
    int iXTrackContinueThreshold  ;                  //判断目标改变方向的阈值
    int iXTrackOffsetValue        ;                  //判断条件保证x方向有一定偏差
    int iTrackMaxFrameNum         ;                  //跟踪最大帧数 
    int      iLittleRegionValue        ;                  //小区域中Y的最高点
    LineSet  tRectLittleRegion         ;                  //小区域范围
    LineSet  tRectTreeLittleRegion     ;                  //树一侧的小区域
    LineSet    fPersonRange[2]        ;                  //人员检测范围参数(梯形) 0-0高点 0-1未用 1左线高点 2左线低点 3右线高点 4右线低点
    LineSet  tLineCurverRange[2]       ;                  //拟合曲线范围 
    LineSet  tLineStraightFirst[2]     ;                  //两条轨迹经过区域线中的第一条线 
    LineSet  tLineStraightSecond[2]    ;                  //两条轨迹经过区域线中的第二条线 
    LineSet  tRectBlackBlock[5]        ;                  //加入的黑色块范围 
    LineSet  tLineBlackLeft[2]         ;                  //加入的黑色直线中的左边为黑 
    LineSet  tLineBlackRight[2]        ;                 //加入的黑色直线中的右边为黑 
    LineSet  tNightRange[3]            ;                 //夜间的区域值
    int      iNightRangeVal            ;                 //判读是否到夜间的阈值
};
  
struct ParamDistinguish
{
    JudgeSet TrackNumMax ;
    JudgeSet TrackNumMin ;
    JudgeSet LostNumVal ;
    JudgeSet MigrationDiff;
    JudgeSet OriginCurrDis ;
    JudgeSet YHeightVal  ;   
    JudgeSet XContValue;    
    JudgeSet XMatchValue;
    JudgeSet YMatchValue;    
    JudgeSet FindObjValue;
    JudgeSet RiseNumVal;
    JudgeSet DownNumVal;
    JudgeSet FindObjValMax;
    JudgeSet FindObjValMin;
    JudgeSet WhiteSpotVal;   //白点数3个区段
    JudgeSet TopPointVal;

    JudgeSetFlt fFindObjInverse;   //与JudgeValue类型一样 float型
    JudgeSetFlt fXMatchInverse;
    JudgeSetFlt fYMatchInverse;
    JudgeSetFlt fXContInverse;

};

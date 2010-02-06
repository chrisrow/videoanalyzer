#pragma once

//���������ò��������Ͷ���

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
    int     bSensitiveFlag            ;                  //�����ȱ�־��3֡����
    int     bTransLensImage           ;                  //����ͼ���־
    int     bLittleRegionFlag         ;                  //�Ƿ�����С����ı�־
    int     bTreeLittleFlag           ;                  //��һ���Ƿ�����С����ı�־
    float    fLineFirstLocation[4]     ;                  //Ŀ�괩Խ������ֱ�߲��� 0����1_A   1����1_B   2����2��       3����2��
    float    fLineSecondLocation[4]    ;                  //���������Ҫ�ĸ߶Ȳ��� 0���� A    1: ��B     2���ߵ������  3���ߵ����Ҷ�    
    int   bBinarizeSubThreshold     ;                  //��ֵ����ֵ  
    int   bNightSubThreshold        ;                  //ҹ���ֵ����ֵ 
    int  iImfilterSingleThreshold  ;                  //ȥС����ֵ
    int  iImdilateThreshold        ;                  //������ֵ
    int iWhiteSpotNumMax          ;                  //���׵���
    int iWhiteSpotNumMin          ;                  //��С�׵���
    int iXTrackContinueThreshold  ;                  //�ж�Ŀ��ı䷽�����ֵ
    int iXTrackOffsetValue        ;                  //�ж�������֤x������һ��ƫ��
    int iTrackMaxFrameNum         ;                  //�������֡�� 
    int      iLittleRegionValue        ;                  //С������Y����ߵ�
    LineSet  tRectLittleRegion         ;                  //С����Χ
    LineSet  tRectTreeLittleRegion     ;                  //��һ���С����
    LineSet    fPersonRange[2]        ;                  //��Ա��ⷶΧ����(����) 0-0�ߵ� 0-1δ�� 1���߸ߵ� 2���ߵ͵� 3���߸ߵ� 4���ߵ͵�
    LineSet  tLineCurverRange[2]       ;                  //������߷�Χ 
    LineSet  tLineStraightFirst[2]     ;                  //�����켣�����������еĵ�һ���� 
    LineSet  tLineStraightSecond[2]    ;                  //�����켣�����������еĵڶ����� 
    LineSet  tRectBlackBlock[5]        ;                  //����ĺ�ɫ�鷶Χ 
    LineSet  tLineBlackLeft[2]         ;                  //����ĺ�ɫֱ���е����Ϊ�� 
    LineSet  tLineBlackRight[2]        ;                 //����ĺ�ɫֱ���е��ұ�Ϊ�� 
    LineSet  tNightRange[3]            ;                 //ҹ�������ֵ
    int      iNightRangeVal            ;                 //�ж��Ƿ�ҹ�����ֵ
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
    JudgeSet WhiteSpotVal;   //�׵���3������
    JudgeSet TopPointVal;

    JudgeSetFlt fFindObjInverse;   //��JudgeValue����һ�� float��
    JudgeSetFlt fXMatchInverse;
    JudgeSetFlt fYMatchInverse;
    JudgeSetFlt fXContInverse;

};

#include "stdafx.h"
#include "RoadWarpper.h"
#include "Macro.h"

#include <vector>

extern int g_debug;

// const int ANGLE_NUM = 25;
// double g_angle_l[ANGLE_NUM];
// double g_angle_r[ANGLE_NUM];

CRoadWarpper::CRoadWarpper()
: m_pFrameOut(NULL)
, m_pFrameTmp(NULL)
, m_pFrameLayer(NULL)
, m_storage(NULL)
{
}

CRoadWarpper::~CRoadWarpper()
{
    reset();
}

void CRoadWarpper::reset()
{
#define RELEASE_IMAGE(a)       \
    do                         \
    {                          \
        if (a)                 \
        {                      \
            cvReleaseImage(&a);\
        }                      \
    } while (0)

    if (m_storage)
    {
        cvReleaseMemStorage(&m_storage);
    }
    m_storage = cvCreateMemStorage(0);

    RELEASE_IMAGE(m_pFrameOut);
    RELEASE_IMAGE(m_pFrameTmp);
    RELEASE_IMAGE(m_pFrameLayer);

    cvInitFont(&m_font, CV_FONT_HERSHEY_PLAIN, 1.0, 0.5);
}

const IplImage* CRoadWarpper::analysis(const IplImage *pFrame)
{
    if (!m_pFrameOut
        || pFrame->width != m_pFrameOut->width 
        || pFrame->height != m_pFrameOut->height)
    {
        reset();
        
        m_pFrameOut = cvCreateImage(cvSize(pFrame->width, pFrame->height), pFrame->depth, 3);
        m_pFrameTmp = cvCreateImage(cvSize(pFrame->width, pFrame->height), pFrame->depth, 1);
        m_pFrameLayer = cvCreateImage(cvSize(pFrame->width, pFrame->height), pFrame->depth, 1);
    }

    cvCopyImage(pFrame, m_pFrameOut);

    cvCvtColor(pFrame, m_pFrameTmp, CV_RGB2GRAY);  //��ͨ���Ҷ�ͼ
    cvSmooth(m_pFrameTmp, m_pFrameTmp);            //��˹ƽ��

    //cvThreshold(m_pFrameTmp, m_pFrameLayer, 120, 255, CV_THRESH_BINARY);
//     cvAdaptiveThreshold(m_pFrameTmp, m_pFrameLayer, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 41, 10);
//     cvEqualizeHist(m_pFrameTmp, m_pFrameLayer);
    SHOW_IMAGE("cvAdaptiveThreshold", m_pFrameTmp);

    cvCanny(m_pFrameTmp, m_pFrameTmp, 150, 30, 3);  //��Ե���

    SHOW_IMAGE("temp", m_pFrameTmp);

    int half_width = pFrame->width/2;
    int half_height = pFrame->height/2;

    int rect_top    = pFrame->height/2 - pFrame->height/30; //10;
    int rect_bottom = pFrame->height/2 + pFrame->height/30; //10;
    int rect_left   = pFrame->width/2  - pFrame->width/20;  //18;
    int rect_right  = pFrame->width/2  + pFrame->width/20;  //18;

    double k = 0;  // б��
    double b = 0;  // �ؾ�
    CvSeq* lines = 0;
    cvClearMemStorage(m_storage);

    int threshold = 20;  // 20 �߶��ϵ�ĸ�����ֵԽ�󣬱�ʾ����߶�����ĵ�Խ��
    double param1 = 10;  // 10 �߶ε���С����
    double param2 = 5 ;  //  5 �����߶β�������һ��ֱ�ߵķָ�����룬ֵԽС��Խ��������ֱ��
    lines = cvHoughLines2( m_pFrameTmp, m_storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, threshold, param1, param2 );
    for( int i = 0; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);

        // ȷ������б��
        if (line[0].x == line[1].x)
        {
            if (line[0].x < half_width)
            {
                (line[0].y < line[1].y) ? (++line[1].x) : (++line[0].x);
            } 
            else
            {
                (line[0].y < line[1].y) ? (++line[0].x) : (++line[1].x);
            }
        }

        // б�ʺͽؾ�
        k = (1.0 * (line[0].y - line[1].y))/(line[0].x - line[1].x);
        b = 1.0 * (line[0].x * line[1].y - line[1].x * line[0].y) / (line[0].x - line[1].x);

        ////////////////////////////////////////////////////////////////////////////
        // б�ʲ���̫С || ��ߵ��ߵ�б�ʱ���С���� || �ұߵ��ߵ�б�ʱ��������
        double max_k = 0.3;
        if ( (k <= max_k && k >= -1 * max_k)
            || ( (line[0].x < half_width) && (line[1].x < half_width) && (k > 0) )
            || ( (line[0].x > half_width) && (line[1].x > half_width) && (k < 0) )
            )
        {
            continue;
        }


        ////////////////////////////////////////////////////////////////////////////
        // �Ƿ���м��С��������������н���
        bool flag = false;
        int x = 0, y = 0;

        y = rect_top;
        x = int((y - b) / k);
        if (x > rect_left && x < rect_right)
        {
            flag = true;
        }

        y = rect_bottom;
        x = int((y - b) / k);
        if (x > rect_left && x < rect_right)
        {
            flag = true;
        }

        if (false == flag)
        {
            continue;
        }

        ////////////////////////////////////////////////////////////////////////////
        // ���ο��µױ����ϵ��߶�����˵�
        int max_y = MAX(line[1].y, line[0].y);
        if (max_y < rect_bottom)
        {
            continue;
        }

        ////////////////////////////////////////////////////////////////////////////
        // ���۶���
        static double k_pre = 0;
        static double b_pre = 0;

        double x_top = 0;
        double y_top = 0;
        
        if (k_pre == k)
        {
            k_pre++;
        }

        x_top = (b - b_pre) / (k_pre - k);
        y_top = k * (b - b_pre) / (k_pre - k) + b;
        
        if (x_top > rect_left && x_top < rect_right && y_top > rect_top && y_top < rect_bottom)
        {
            cvCircle(m_pFrameOut, cvPoint(int(x_top), int(y_top)), 1, cvScalar(0,0,255), 1, CV_AA, 0 );
        }

        k_pre = k;
        b_pre = b;
        ////////////////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////////////////////

        cvLine( m_pFrameOut, line[0], line[1], CV_RGB(255,0,0), 1, CV_AA, 0 );
    }

    // TODO: ����������л�۶���ŵ������У���ֹ�˲������ֵ

    cvRectangle(m_pFrameOut, 
        cvPoint(rect_left,  rect_top), 
        cvPoint(rect_right, rect_bottom), 
        cvScalar(0, 255, 0, 0));

    return m_pFrameOut;
}

ALERTTYPE CRoadWarpper::haveAlert()
{
    return AT_NONE;
}

void CRoadWarpper::updateFrame(const IplImage *pFrame)
{
    const IplImage* p = analysis(pFrame) ;

    doReceiver(p);

    if ( this->haveAlert() != AT_NONE)
    {
        doAlerter(pFrame);
    }
}

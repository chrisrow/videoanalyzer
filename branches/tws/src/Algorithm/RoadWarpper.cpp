#include "stdafx.h"
#include "RoadWarpper.h"
#include "Macro.h"

extern int g_debug;

CRoadWarpper::CRoadWarpper()
: m_pFrameOut(NULL)
, m_pFrameTmp(NULL)
, m_storage(NULL)
{

}

CRoadWarpper::~CRoadWarpper()
{
    reset();
}

void CRoadWarpper::reset()
{
    if (m_storage)
    {
        cvReleaseMemStorage(&m_storage);
    }
    m_storage = cvCreateMemStorage(0);

    if (m_pFrameOut)
    {
        cvReleaseImage(&m_pFrameOut);
    }

    if (m_pFrameTmp)
    {
        cvReleaseImage(&m_pFrameTmp);
    }

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
    }

    cvCopyImage(pFrame, m_pFrameOut);

    cvCvtColor(pFrame, m_pFrameTmp, CV_RGB2GRAY);  //��ͨ���Ҷ�ͼ
    cvSmooth(m_pFrameTmp, m_pFrameTmp);            //��˹ƽ��
//     cvAdaptiveThreshold(m_pFrameTmp, m_pFrameTmp, 255);
//     SHOW_IMAGE("temp1", m_pFrameTmp);

    cvCanny(m_pFrameTmp, m_pFrameTmp, 150, 30, 3);  //��Ե���

    SHOW_IMAGE("temp", m_pFrameTmp);

    int half_width = pFrame->width/2;
    int half_height = pFrame->height/2;

    int rect_top    = pFrame->height/2 - 10;
    int rect_bottom = pFrame->height/2 + 10;
    int rect_left   = pFrame->width/2  - 20;
    int rect_right  = pFrame->width/2  + 20;

    double k = 0;  // б��
    double b = 0;  // �ؾ�
    CvSeq* lines = 0;
    cvClearMemStorage(m_storage);

    int threshold = 20;  //�߶��ϵ�ĸ�����ֵԽ�󣬱�ʾ����߶�����ĵ�Խ��
    double param1 = 10;  //�߶ε���С����
    double param2 = 5;   //�����߶β�������һ��ֱ�ߵķָ�����룬ֵԽС��Խ��������ֱ��
    lines = cvHoughLines2( m_pFrameTmp, m_storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, threshold, param1, param2 );
    for( int i = 0; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
        //б�ʴ���1
        if (line[0].x == line[1].x)
        {
            line[0].x += 1;
        }

        k = (1.0 * (line[0].y - line[1].y))/(line[0].x - line[1].x);

        ////////////////////////////////////////////////////////////////////////////
        // б�ʲ���̫С || ��ߵ��ߵ�б�ʱ���С���� || �ұߵ��ߵ�б�ʱ��������
        if ( (k <= 0.3 && k >= -0.3)
            || ( (line[0].x < half_width) && (line[1].x < half_width) && (k > 0) )
            || ( (line[0].x > half_width) && (line[1].x > half_width) && (k < 0) )
            )
        {
            continue;
        }

        b = 1.0 * (line[0].x * line[1].y - line[1].x * line[0].y) / (line[0].x - line[1].x);

        ////////////////////////////////////////////////////////////////////////////
        // �Ƿ���м��С��������������н���
        bool flag = false;
        int x = 0, y = 0;
        char szText[64] = {0};

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

        cvLine( m_pFrameOut, line[0], line[1], CV_RGB(255,0,0), 1, CV_AA, 0 );
    }

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

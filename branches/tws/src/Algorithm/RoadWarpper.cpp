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

    cvCvtColor(pFrame, m_pFrameTmp, CV_RGB2GRAY);  //单通道灰度图
    cvSmooth(m_pFrameTmp, m_pFrameTmp);            //高斯平滑
//     cvAdaptiveThreshold(m_pFrameTmp, m_pFrameTmp, 255);
//     SHOW_IMAGE("temp1", m_pFrameTmp);

    cvCanny(m_pFrameTmp, m_pFrameTmp, 150, 30, 3);  //边缘检测

    SHOW_IMAGE("temp", m_pFrameTmp);

    int half_width = pFrame->width/2;
    int half_height = pFrame->height/2;
    double k = 0;  // 斜率
    double b = 0;  // 截距
    CvSeq* lines = 0;
    cvClearMemStorage(m_storage);
    lines = cvHoughLines2( m_pFrameTmp, m_storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 20, 20, 30 );
    for( int i = 0; i < lines->total; i++ )
    {
        CvPoint* line = (CvPoint*)cvGetSeqElem(lines,i);
        //斜率大于1
        if (line[0].x == line[1].x)
        {
            line[0].x += 1;
        }

        k = (1.0 * (line[0].y - line[1].y))/(line[0].x - line[1].x);

        // 斜率不能太小 || 左边的线的斜率必须小于零 || 右边的线的斜率必须大于零
        if ( (k <= 0.3 && k >= -0.3)
            || ( (line[0].x < half_width) && (line[1].x < half_width) && (k > 0) )
            || ( (line[0].x > half_width) && (line[1].x > half_width) && (k < 0) )
            )
        {
            continue;
        }

        b = 1.0 * (line[0].x * line[1].y - line[1].x * line[0].y) / (line[0].x - line[1].x);
        // 是否跟中间的小框的上下两个边有交点
        bool flag = false;
        int x = 0, y = 0;
        char szText[64] = {0};

        y = half_height-10;
        x = int((y - b) / k);
        if (x > half_width-15 && x < half_width+15)
        {
            flag = true;
        }

        y = half_height+10;
        x = int((y - b) / k);
        if (x > half_width-15 && x < half_width+15)
        {
            flag = true;
        }

        if (false == flag)
        {
            cvLine( m_pFrameOut, line[0], line[1], CV_RGB(0,0,255), 1, CV_AA, 0 );
            sprintf(szText, "%d,%.2f,%.2f", x,b,k);
            cvPutText(m_pFrameOut, szText, line[0], &m_font, cvScalar(0,0,0,0));
            continue;
        }

        cvLine( m_pFrameOut, line[0], line[1], CV_RGB(255,0,0), 1, CV_AA, 0 );
    }

    cvRectangle(m_pFrameOut, cvPoint(half_width-15, half_height-10), cvPoint(half_width+15, half_height+10), cvScalar(0, 255, 0, 0));

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

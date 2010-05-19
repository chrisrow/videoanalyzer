#include "StdAfx.h"
#include "VideoRecorder.h"

const int CIF_WIDTH  = 352;
const int CIF_HEIGHT = 288;

CVideoRecorder::CVideoRecorder(void)
: m_pWriter(NULL)
, m_iWidth(CIF_WIDTH)
, m_iHeight(CIF_HEIGHT)
{
}

CVideoRecorder::~CVideoRecorder(void)
{
    if (m_pWriter)
    {
        cvReleaseVideoWriter(&m_pWriter);
        m_pWriter = NULL;
    }
}

bool CVideoRecorder::start(const char* szFileName)
{
    int isColor = 1;
    int fps     = 25;

    m_pWriter = cvCreateVideoWriter(szFileName, CV_FOURCC('D', 'I', 'V', 'X'),
        fps, cvSize(m_iWidth, m_iHeight), isColor);

    return (NULL == m_pWriter ? false : true);
}

void CVideoRecorder::stop()
{
    if (m_pWriter)
    {
        cvReleaseVideoWriter(&m_pWriter);
        m_pWriter = NULL;
    }
}

void CVideoRecorder::setResolution(int iWidth, int iHeight)
{
    m_iWidth = iWidth;
    m_iHeight = iHeight;
}

void CVideoRecorder::updateFrame(const IplImage *pFrame)
{
    if (m_pWriter)
    {
        cvWriteFrame(m_pWriter, pFrame);
    }
}

#include "stdafx.h"
#include "VideoAnalyzerDlg.h"
#include "VideoGraber.h"

#include "Algorithm/CParabolaDetect.h"
#include "Algorithm/Macro.h"

extern int g_debug;
extern ParamStruct ParamSet;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CVideoGraber::CVideoGraber(CWnd* pWnd)
: m_pWnd(pWnd)
, m_evtPause(false, true)
, m_eStat(STAT_STOPPED)
{

}

CVideoGraber::~CVideoGraber()
{
    release();
}

VIDEO_STATUS CVideoGraber::getStatus()
{
    return m_eStat;
}

void CVideoGraber::loopGetFrame()
{
    CTaskBase::run();
}

bool CVideoGraber::open(const char* szPath)
{
    if (doOpen(szPath))
    {
        m_evtPause.SetEvent();
        return true;
    }
    else
    {
        return false;
    }
}

bool CVideoGraber::open(int iIndex)
{
    if (doOpen(iIndex))
    {
        m_evtPause.SetEvent();
        return true;
    }
    else
    {
        return false;
    }
}

void CVideoGraber::close()
{
    if (STAT_RUN == m_eStat)
    {
        m_eStat = STAT_TOSTOP;
        m_evtPause.SetEvent();
    }
}

void CVideoGraber::pause()
{
    m_evtPause.ResetEvent();
}

void CVideoGraber::resume()
{
    m_evtPause.SetEvent();
}

void CVideoGraber::afterStop()
{
	doAfterStop();

    Sleep(200);
    if (m_pWnd)
    {
        m_pWnd->PostMessage(WM_VIDEO_END);
    }
}

////////////////////////////////////////////////////////////////////

CCameraWarpper::CCameraWarpper(CWnd* pWnd)
: CVideoGraber(pWnd)
, m_pCamera(NULL)
{
}

CCameraWarpper::~CCameraWarpper()
{
    this->release();
}

bool CCameraWarpper::doOpen(int iIndex)
{
    if (iIndex < 0)
    {
        return false;
    }

    if (!m_pCamera)
    {
        m_pCamera = CCameraDllMgr::getCameraMgr()->createCamera();
    }

    return m_pCamera->open(iIndex);
}

void CCameraWarpper::release()
{
    if (m_pCamera)
    {
        m_pCamera->close();
        CCameraDllMgr::getCameraMgr()->destroyCamera(&m_pCamera);
    }

    CCameraDllMgr::destoryCameraMgr();
}

bool CCameraWarpper::setProperty(int iID, double value)
{
    if (!m_pCamera)
    {
        return false;
    }

    CVideoGraber::pause();

    bool bResult = false;
    switch(iID)
    {
    case PROP_WIDTH:   bResult = m_pCamera->setWidth(static_cast<int>(value)); break;
    case PROP_HEITHT:  bResult = m_pCamera->setHeight(static_cast<int>(value));break;
    default:           bResult = false; break;
    }

    CVideoGraber::resume();

    return true;
}

bool CCameraWarpper::getProperty(int iID, double& value)
{
    if (!m_pCamera)
    {
        return false;
    }

    switch(iID)
    {
    case PROP_WIDTH:    value = m_pCamera->getWidth();  break;
    case PROP_HEITHT:   value = m_pCamera->getHeight(); break;
    default:            return false;
    }

    return true;
}

void CCameraWarpper::doRun()
{
    if (!m_pCamera)
    {
        return;
    }
    
    doLoop();
}

void CCameraWarpper::doLoop()
{
    IplImage *pFrame = NULL;

    m_eStat = STAT_RUN;
    while(STAT_RUN == m_eStat)
    {
        WaitForSingleObject(m_evtPause, INFINITE); 

        pFrame = m_pCamera->retrieveFrame();
        if (!pFrame)
        {
            break;
        }

        FOR_EACH(IFrameReceiver*, updateFrame, pFrame);
        cvWaitKey(1);
    }
    m_eStat = STAT_STOPPED;

    afterStop();
}

void CCameraWarpper::doAfterStop()
{
	DEL_ALL_IMAGE();	
	this->release();
}

//////////////////////////////////////////////////////////////////////////////
COpenCVWarpper::COpenCVWarpper()
: m_pCapture(NULL)
{
}

COpenCVWarpper::~COpenCVWarpper()
{
}

bool COpenCVWarpper::open(int iIndex)
{
    m_pCapture = cvCreateCameraCapture( iIndex );
    return m_pCapture ? true : false;
}

void COpenCVWarpper::close()
{
    if (m_pCapture)
    {
        cvReleaseCapture(&m_pCapture);
    }
}

IplImage* COpenCVWarpper::retrieveFrame()
{
    return cvQueryFrame(m_pCapture);
}

bool COpenCVWarpper::setWidth(int iWidth)
{
    return cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_WIDTH, iWidth) ? true : false;
}

bool COpenCVWarpper::setHeight(int iHeight)
{
    return cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_HEIGHT, iHeight) ? true : false;
}

int  COpenCVWarpper::getWidth()
{
    return static_cast<int>(cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_WIDTH));
}

int  COpenCVWarpper::getHeight()
{
    return static_cast<int>(cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_HEIGHT));
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CVideoFile::CVideoFile(CWnd* pWnd)
: CVideoGraber(pWnd)
, m_pCapture(NULL)
, m_iFPS(25)
, m_iWaitTime(30)
, m_bStep(false)
, m_iStartFrame(0)
, m_uFrame(0)
{

}

CVideoFile::~CVideoFile()
{
    this->release();
}

void CVideoFile::release()
{
    if (m_pCapture)
    {
        cvReleaseCapture(&m_pCapture);
    }
}

bool CVideoFile::doOpen(const char* szPath)
{
    m_uFrame = 0;
    m_pCapture = cvCreateFileCapture( szPath );

    return (m_pCapture ? true : false);
}

void CVideoFile::doRun()
{
    if (!m_pCapture)
    {
        return;
    }

    IplImage *pFrame = NULL;
    m_eStat = STAT_RUN;
    while(STAT_RUN == m_eStat)
    {
        if (m_uFrame >= m_iStartFrame)
        {
            WaitForSingleObject(m_evtPause, INFINITE); 
        }

        if (m_bStep)
        {
            m_evtPause.ResetEvent();
        }

        pFrame = cvQueryFrame(m_pCapture);
        if (!pFrame)
        {
            break;
        }
        m_uFrame++;

        if(ParamSet.bTransLensImage)
        {
            cvFlip(pFrame, NULL, 1);//Ë®Æ½¾µÏñ
        }

        if (m_uFrame >= m_iStartFrame)
        {
            FOR_EACH(IFrameReceiver*, updateFrame, pFrame);

            cvWaitKey(1);
            Sleep(m_iWaitTime);
        }
    }
    m_eStat = STAT_STOPPED;
    afterStop();
}

void CVideoFile::doAfterStop()
{
	DEL_ALL_IMAGE();	
	this->release();
}

bool CVideoFile::setProperty(int iID, double value)
{
    int iResult = 0;
    switch(iID)
    {
    case PROP_FRAMERATE: 
        m_iFPS = (int)value;
        m_iWaitTime = (int)(1000/m_iFPS) - 10;
        if (m_iWaitTime < 0)
        {
            m_iWaitTime = 0;
        }
        iResult = true;
        break;
    case PROP_STEP:
        m_bStep = (value > 0.1 ? true : false);
        iResult = true;
        break;
    case PROP_POS_MSEC:
        iResult = cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_POS_MSEC, value);
        break;
    case PROP_POS_FRAME:
        //iResult = cvSetCaptureProperty(m_pCapture, CV_CAP_PROP_POS_FRAMES, value);
        m_iStartFrame = (unsigned int)value;
        iResult = true;
        break;

    default:
        return false;
    }

    return true;
}

bool CVideoFile::getProperty(int iID, double& value)
{
    switch(iID)
    {
    case PROP_WIDTH: 
        value = cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_WIDTH);
        break;
    case PROP_HEITHT: 
        value = cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_HEIGHT);
        break;
    case PROP_FRAMERATE: 
        value = cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FPS);
        break;
    case PROP_POS_MSEC: 
        value = cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_POS_MSEC);
        break;
    case PROP_POS_FRAME: 
        value = cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_POS_FRAMES);
        break;
    case PROP_FRAME_CNT: 
        value = cvGetCaptureProperty(m_pCapture, CV_CAP_PROP_FRAME_COUNT);
        break;
    default:
        return false;
    }

    return true;
}
////////////////////////////////////////////////////


PGETCAMERAMGR CCameraDllMgr::m_pGetCameraMgr = NULL;
PDESTROYCAMERAMGR CCameraDllMgr::m_pDestroyCameraMgr = NULL;
HINSTANCE CCameraDllMgr::m_hIns;

bool CCameraDllMgr::loadDll()
{
//     m_hIns = LoadLibrary("libVideoInput.dll");
    m_hIns = LoadLibrary("libHikVision.dll");
    if(!m_hIns)
    {
        DWORD errorCode = GetLastError();
        CString str;
        str.Format("'libVideoInput.dll' not supported, ErrorCode = %d", errorCode);
        AfxMessageBox(str);
        return false;
    }

    m_pGetCameraMgr = (PGETCAMERAMGR)GetProcAddress(m_hIns,"getCameraMgr");
    if(!m_pGetCameraMgr)
    {
        DWORD errorCode = GetLastError();
        CString str;
        str.Format("Can't find 'getCameraMgr()' in 'libVideoInput.dll', ErrorCode = %d", errorCode);
        AfxMessageBox(str);
        return false;
    }

    m_pDestroyCameraMgr = (PDESTROYCAMERAMGR)GetProcAddress(m_hIns,"destoryCameraMgr");
    if(!m_pDestroyCameraMgr)
    {
        DWORD errorCode = GetLastError();
        CString str;
        str.Format("Can't find 'getCameraMgr()' in 'libVideoInput.dll', ErrorCode = %d", errorCode);
        AfxMessageBox(str);
        return false;
    }

    return true;
}

void CCameraDllMgr::freeDll()
{
    FreeLibrary(m_hIns);
}

ICameraMgr* CCameraDllMgr::getCameraMgr()
{
    if (!m_pGetCameraMgr)
    {
        return NULL;
    }
    return m_pGetCameraMgr();
}

void CCameraDllMgr::destoryCameraMgr()
{
    if (m_pDestroyCameraMgr)
    {
        m_pDestroyCameraMgr();
    }
}

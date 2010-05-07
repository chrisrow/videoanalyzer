#include "VIWarpper.h"

static CVIMgr* g_VIMgr = NULL;

ICameraMgr* getCameraMgr()
{
    if (!g_VIMgr)
    {
        g_VIMgr = new CVIMgr;
    }

    return g_VIMgr;
}

void destoryCameraMgr()
{
    delete g_VIMgr;
    g_VIMgr = NULL;
}

CVIWarpper::CVIWarpper()
: m_iIndex(-1)
, m_pFrame(NULL)
, m_iWidth(CIF_WIDTH)
, m_iHeight(CIF_HEIGHT)
{
}

CVIWarpper::~CVIWarpper()
{
}

bool CVIWarpper::open(int iIndex)
{
    m_camera.setVerbose(false);
    this->close();
    if(!m_camera.setupDevice(iIndex, m_iWidth, m_iHeight))
    {
        return false;
    }
    if( !m_camera.isDeviceSetup(iIndex) )
    {
        return false;
    }
    m_iIndex = iIndex;
    return true;
}

void CVIWarpper::close()
{
    if( m_iIndex >= 0 )
    {
        m_camera.stopDevice(m_iIndex);
        m_iIndex = -1;

    }

    if (m_pFrame)
    {
        cvReleaseImage(&m_pFrame);
    }
}

IplImage* CVIWarpper::retrieveFrame()
{
    if( !m_pFrame 
        || m_camera.getWidth(m_iIndex)  != m_pFrame->width 
        || m_camera.getHeight(m_iIndex) != m_pFrame->height )
    {
        if (m_pFrame)
        {
            cvReleaseImage( &m_pFrame );
        }
        int w = m_camera.getWidth(m_iIndex);
        int h = m_camera.getHeight(m_iIndex);
        int depth = 8;
        int channel = 3;
        m_pFrame = cvCreateImage( cvSize(w,h), depth, channel );
    }

    m_camera.getPixels( m_iIndex, (unsigned char*)m_pFrame->imageData, false, true );
    return m_pFrame;
}

bool CVIWarpper::setWidth(int iWidth)
{
    if( iWidth != m_camera.getWidth(m_iIndex) )
    {
        m_iWidth = iWidth;
        m_camera.stopDevice(m_iIndex);
        m_camera.setupDevice(m_iIndex, m_iWidth, m_iHeight);
    }
    return m_camera.isDeviceSetup(m_iIndex);
}

bool CVIWarpper::setHeight(int iHeight)
{
    if( iHeight != m_camera.getHeight(m_iIndex) )
    {
        m_iHeight = iHeight;
        m_camera.stopDevice(m_iIndex);
        m_camera.setupDevice(m_iIndex, m_iWidth, m_iHeight);
    }
    return m_camera.isDeviceSetup(m_iIndex);
}

int  CVIWarpper::getWidth()
{
    return m_camera.getWidth(m_iIndex);
}
int  CVIWarpper::getHeight()
{
    return m_camera.getHeight(m_iIndex);
}

/////////////////////////////////////////////////

CVIMgr::CVIMgr()
: m_pCamera(NULL)
{
    memset(m_szName, 0, CAM_NAME_LEN);
}

CVIMgr::~CVIMgr()
{
    delete m_pCamera;
    m_pCamera = NULL;
}

ICamera* CVIMgr::getCamera()
{
    if (!m_pCamera)
    {
        m_pCamera = new CVIWarpper;
    }

    return m_pCamera;
}

void CVIMgr::destroyCamera()
{
    delete m_pCamera;
    m_pCamera = NULL;
}

int CVIMgr::getCount()
{
    return videoInput::listDevices();
}

const char* CVIMgr::getName(int iIndex)
{
    return videoInput::getDeviceName(iIndex);

}

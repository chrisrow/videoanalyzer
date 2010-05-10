#include "stdafx.h"
#include "HikWarpper.h"

static CHikMgr* g_HikMgr = NULL;

ICameraMgr* getCameraMgr()
{
    if (!g_HikMgr)
    {
        g_HikMgr = new CHikMgr;
    }

    return g_HikMgr;
}

void destoryCameraMgr()
{
    delete g_HikMgr;
    g_HikMgr = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CALLBACK VideoRealDataCB(
                                      LONG      lRealHandle,
                                      DWORD     dwDataType,
                                      BYTE      *pBuffer,
                                      DWORD     dwBufSize,
                                      void      *pUser
                                      )
{
    CHikWarpper* pHik = (CHikWarpper*)pUser;

    //TODO: copy data to 
    //BOOL result = PlayM4_GetBMP(m_iPort, PBYTE pBitmap,DWORD nBufSize,DWORD* pBmpSize);
    pHik->m_pFrame;
}

CHikWarpper::CHikWarpper()
: m_iIndex(-1)
, m_pFrame(NULL)
, m_iWidth(CIF_WIDTH)
, m_iHeight(CIF_HEIGHT)
, m_iUseID(-1)
, m_hPlay(-1)
, m_iPort(-1)
{
    NET_DVR_Init();
}

CHikWarpper::~CHikWarpper()
{
    if (m_iUseID != -1)
    {
        NET_DVR_Logout_V30(m_iUseID);
    }
    //释放资源
    NET_DVR_Cleanup();
}

bool CHikWarpper::open(int iIndex)
{
    //登录
    NET_DVR_DEVICEINFO_V30 deviceInfo;
    memset(&deviceInfo, 0, sizeof(NET_DVR_DEVICEINFO_V30));
    m_iUseID = NET_DVR_Login_V30("10.13.14.51", 8000, "admin", "admin", &deviceInfo);
    if (-1 == m_iUseID)
    {
        return false;
    }

    //开始播放
    BOOL bBlocked = FALSE;
    NET_DVR_CLIENTINFO clientInfo;
    //TODO: clientInfo
    clientInfo.lChannel = 1;
    m_hPlay = NET_DVR_RealPlay_V30(m_iUseID, &clientInfo, VideoRealDataCB, this, bBlocked);
    if (-1 == m_hPlay)
    {
        return false;
    }

    m_iPort = NET_DVR_GetRealPlayerIndex(m_hPlay);
    if (-1 == m_iPort)
    {
        return false;
    }


    return true;
}

void CHikWarpper::close()
{
    //停止播放
    NET_DVR_StopRealPlay(m_hPlay);
    //登出
    NET_DVR_Logout_V30(m_iUseID);
}

IplImage* CHikWarpper::retrieveFrame()
{
    if( !m_pFrame )
    {
        int depth = 8;
        int channel = 3;
        m_pFrame = cvCreateImage( cvSize(m_iWidth, m_iHeight), depth, channel );
        cvSetZero(m_pFrame);
    }

    //阻塞，直到m_pFrame更新
//     DWORD result = WaitForSingleObject(hEvent, 1000);
//     if( result != WAIT_OBJECT_0) 
//     {
//         return NULL;
//     }

    return m_pFrame;
}

bool CHikWarpper::setWidth(int iWidth)
{
    m_iWidth = iWidth;
    //TODO: 调用hik sdk

    return true;
}

bool CHikWarpper::setHeight(int iHeight)
{
    m_iHeight = iHeight;
    //TODO: 调用hik sdk

    return true;
}

int  CHikWarpper::getWidth()
{
    //TODO: 调用hik sdk

    return m_iWidth;
}

int  CHikWarpper::getHeight()
{
    //TODO: 调用hik sdk

    return m_iHeight;
}

/////////////////////////////////////////////////

CHikMgr::CHikMgr()
{
    memset(m_szName, 0, CAM_NAME_LEN);
}

CHikMgr::~CHikMgr()
{
}

ICamera* CHikMgr::createCamera()
{
    return new CHikWarpper;
}

void CHikMgr::destroyCamera(ICamera** pCamera)
{
    delete *pCamera;
    *pCamera = NULL;
}

int CHikMgr::getCount()
{
    return 1;
}

const char* CHikMgr::getName(int iIndex)
{
    return "test";

}

#include "stdafx.h"
#include "HikWarpper.h"

#include "highgui.h"

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
IplImage* g_pFrame = NULL;
CCriticalSection g_CSect; //����g_pFrame

class CGuard
{
public:
    CGuard(CCriticalSection& CSect): m_lock(&CSect)
    { 
        m_lock.Lock(); 
    }

    ~CGuard()                 
    { 
        m_lock.Unlock(); 
    }

private:
    CSingleLock m_lock;
};

void CALLBACK DecordCB(long nPort,char * pBuf, long nSize, FRAME_INFO * pFrameInfo, 
                        long nUser,long nReserved2)
{
    CHikWarpper* pHik = (CHikWarpper*)nUser;

    DWORD nResult = PLAYM4_NOERROR;
    CGuard guard(g_CSect);
    if (pFrameInfo->nType = T_YV12)
    {
        if (!g_pFrame
            || pFrameInfo->nWidth != g_pFrame->width 
            || pFrameInfo->nHeight != g_pFrame->height)
        {
            if (!g_pFrame)
            {
                cvReleaseImage(&g_pFrame);
            }

            int iDepth = 8;
            int iChannel = 3;
            g_pFrame = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), iDepth, iChannel);
        }

        yv12_to_rgb24((unsigned char*)pBuf, (unsigned char*)g_pFrame->imageData, pFrameInfo->nWidth, pFrameInfo->nHeight);
        
        SetEvent(pHik->m_hEvent);
    }

}

void CALLBACK VideoRealDataCB(
                                      LONG      lRealHandle,
                                      DWORD     dwDataType,
                                      BYTE      *pBuffer,
                                      DWORD     dwBufSize,
                                      void      *pUser
                                      )
{
    CHikWarpper* pHik = (CHikWarpper*)pUser;

    DWORD nResult = PLAYM4_NOERROR;

    LONG lPort;
    lPort = pHik->m_iPort; //�ص����ݲ���ϵͳͷʱ����Ҫ��ȡ֮ǰ�����������port��

    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //ϵͳͷ

        if (!PlayM4_GetPort(&lPort))  //��ȡ���ſ�δʹ�õ�ͨ����
        {
            nResult = PlayM4_GetLastError(lPort);
            break;
        }
        pHik->m_iPort = lPort;  //�������������ͨ���ű���Ϊȫ�ֵģ��Ա�֮�󲥷Żص�����������

        if (dwBufSize > 0)
        {
            if (!PlayM4_SetStreamOpenMode(lPort, STREAME_REALTIME))  //����ʵʱ������ģʽ
            {
                nResult = PlayM4_GetLastError(lPort);
                break;
            }

            if (!PlayM4_OpenStream(lPort, pBuffer, dwBufSize, 1024*1024)) //�����ӿ�
            {
                nResult = PlayM4_GetLastError(lPort);
                break;
            }

            if (!PlayM4_SetDecCallBackMend(lPort, DecordCB, (long)pHik)) //����ص�����
            {
                nResult = PlayM4_GetLastError(lPort);
                break;
            }

            if (!PlayM4_Play(lPort, NULL)) //���ſ�ʼ
            {
                nResult = PlayM4_GetLastError(lPort);
                break;
            }
        }
    case NET_DVR_STREAMDATA:   //��������
        if (dwBufSize > 0 && lPort != -1)
        {
            if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))  //����������
            {
                nResult = PlayM4_GetLastError(lPort);
                break;
            } 
        }
    }

    if (nResult != PLAYM4_NOERROR)
    {

    }
}

CHikWarpper::CHikWarpper()
: m_iIndex(-1)
, m_pFrame(NULL)
, m_iWidth(CIF_WIDTH)
, m_iHeight(CIF_HEIGHT)
, m_hEvent(false, true)
, m_iUseID(-1)
, m_hPlay(-1)
, m_iPort(-1)
{
    NET_DVR_Init();
}

CHikWarpper::~CHikWarpper()
{
    if (m_pFrame)
    {
        cvReleaseImage(&m_pFrame);
    }

    if (m_iUseID != -1)
    {
        NET_DVR_Logout_V30(m_iUseID);
    }
    //�ͷ���Դ
    NET_DVR_Cleanup();
}

bool CHikWarpper::open(int iIndex)
{
    //��¼
    NET_DVR_DEVICEINFO_V30 deviceInfo;
    memset(&deviceInfo, 0, sizeof(NET_DVR_DEVICEINFO_V30));
    m_iUseID = NET_DVR_Login_V30("10.13.14.51", 8000, "admin", "12345", &deviceInfo);
    if (-1 == m_iUseID)
    {
        return false;
    }

    //��ʼ����
    BOOL bBlocked = FALSE;
    NET_DVR_CLIENTINFO clientInfo;
    //TODO: clientInfo
    clientInfo.lChannel = 1;  // ͨ���ţ���1��ʼ
    clientInfo.lLinkMode = 0;
    clientInfo.hPlayWnd = NULL;
    clientInfo.sMultiCastIP = NULL;
    m_hPlay = NET_DVR_RealPlay_V30(m_iUseID, &clientInfo, VideoRealDataCB, this, bBlocked);
    if (-1 == m_hPlay)
    {
        return false;
    }

    return true;
}

void CHikWarpper::close()
{
    PlayM4_CloseStream(m_iPort);

    PlayM4_FreePort(m_iPort);
    m_iPort = -1;

    //ֹͣ����
    NET_DVR_StopRealPlay(m_hPlay);
    //�ǳ�
    NET_DVR_Logout_V30(m_iUseID);
}

IplImage* CHikWarpper::retrieveFrame()
{
    //������ֱ��m_pFrame����
    DWORD result = WaitForSingleObject(m_hEvent, 1000);
    ResetEvent(m_hEvent);
    if( result != WAIT_OBJECT_0) 
    {
        return NULL;
    }

    CGuard guard(g_CSect);
    if (NULL == g_pFrame)
    {
        return NULL;
    }
    
    if (NULL == m_pFrame) 
    {
        int depth = 8;
        int channel = 3;
        m_pFrame = cvCloneImage(g_pFrame);
        return m_pFrame;
    } 

    if (m_pFrame->width != g_pFrame->width
        || m_pFrame->height != g_pFrame->height)
    {
        cvReleaseImage(&m_pFrame);
        m_pFrame = cvCloneImage(g_pFrame);
        return m_pFrame;
    }

    cvCopyImage(g_pFrame, m_pFrame);
    
    return m_pFrame;
}

bool CHikWarpper::setWidth(int iWidth)
{
    m_iWidth = iWidth;
    //TODO: ����hik sdk

    return true;
}

bool CHikWarpper::setHeight(int iHeight)
{
    m_iHeight = iHeight;
    //TODO: ����hik sdk

    return true;
}

int  CHikWarpper::getWidth()
{
    //TODO: ����hik sdk

    return m_iWidth;
}

int  CHikWarpper::getHeight()
{
    //TODO: ����hik sdk

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

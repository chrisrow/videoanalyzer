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
#define YUV2RGB(y, u, v, r, g, b)\
    r = y + ((v*1436) >>10);\
    g = y - ((u*352 + v*731) >> 10);\
    b = y + ((u*1814) >> 10);\
    r = r < 0 ? 0 : r;\
    g = g < 0 ? 0 : g;\
    b = b < 0 ? 0 : b;\
    r = r > 255 ? 255 : r;\
    g = g > 255 ? 255 : g;\
    b = b > 255 ? 255 : b


static inline void yv12_to_rgb24 (unsigned char *src, unsigned char *dest, int width, int height)
{
    register int i,j;
    register int y0, y1, u, v;
    register int r, g, b;
    register unsigned char *s[3];
    s[0] = src;
    s[1] = s[0] + width*height;
    s[2] = s[1] + width*height/4;
    for (i = 0; i < height; i++) 
    {
        for (j = 0; j < width/2; j++) 
        {
            y0 = *(s[0])++;
            y1 = *(s[0])++;
            if (i % 2 == 0 ) 
            {
                u = *(s[1])++ - 128;
                v = *(s[2])++ - 128;
            }
            YUV2RGB (y0, u, v, r, g, b);
            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
            YUV2RGB (y1, u, v, r, g, b);
            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
        }
    }
}

void CALLBACK DecCBFunc(long nPort,char * pBuf, long nSize, FRAME_INFO * pFrameInfo, 
                        long nReserved1,long nReserved2)
{
//frame type
// #define T_AUDIO16	101
// #define T_AUDIO8	    100
// #define T_UYVY		1
// #define T_YV12		3
// #define T_RGB32		7

    if (pFrameInfo->nType = 3)
    {
        IplImage *pFrame = cvCreateImage(cvSize(pFrameInfo->nWidth, pFrameInfo->nHeight), 8, 3);

        yv12_to_rgb24((unsigned char*)pBuf, (unsigned char*)pFrame->imageData, pFrameInfo->nWidth, pFrameInfo->nHeight);

        cvShowImage("tmp3", pFrame);
        cvWaitKey(10);

        cvReleaseImage(&pFrame);
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

    LONG lPort;
    lPort = pHik->m_iPort; //�ص����ݲ���ϵͳͷʱ����Ҫ��ȡ֮ǰ�����������port��

    switch (dwDataType)
    {
    case NET_DVR_SYSHEAD: //ϵͳͷ

        if (!PlayM4_GetPort(&lPort))  //��ȡ���ſ�δʹ�õ�ͨ����
        {
            break;
        }
        pHik->m_iPort = lPort;  //�������������ͨ���ű���Ϊȫ�ֵģ��Ա�֮�󲥷Żص�����������

        if (dwBufSize > 0)
        {
            if (!PlayM4_SetStreamOpenMode(lPort, STREAME_REALTIME))  //����ʵʱ������ģʽ
            {
                break;
            }

            if (!PlayM4_OpenStream(lPort, pBuffer, dwBufSize, 1024*1024)) //�����ӿ�
            {
                break;
            }

            BOOL  bResult = PlayM4_SetDecCallBack(lPort, DecCBFunc);

            if (!PlayM4_Play(lPort, NULL)) //���ſ�ʼ
            {
                break;
            }
        }
    case NET_DVR_STREAMDATA:   //��������
        if (dwBufSize > 0 && lPort != -1)
        {
            if (!PlayM4_InputData(lPort, pBuffer, dwBufSize))  //����������
            {
                break;
            } 
        }
    }



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
    clientInfo.lChannel = 1;
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
    //ֹͣ����
    NET_DVR_StopRealPlay(m_hPlay);
    //�ǳ�
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

    //������ֱ��m_pFrame����
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

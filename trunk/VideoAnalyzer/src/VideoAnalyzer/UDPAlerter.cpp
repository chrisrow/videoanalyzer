#include "StdAfx.h"
#include "UDPAlerter.h"

CUDPAlerter::CUDPAlerter()
: m_iChannel(-1)
, m_pIplImage(NULL)
{
    memset(m_ucLocalIP, 0, 4);
    memset(m_ucRemoteIP, 0, 4);
}

CUDPAlerter::~CUDPAlerter()
{
}

void CUDPAlerter::alert(const IplImage *pFrame)
{
    m_pIplImage = pFrame;
    CTaskBase::run();
}

void CUDPAlerter::doRun()
{
    IplImage* pImage = cvCloneImage(m_pIplImage);
    const unsigned short REMOTE_PORT = 10116;

    SOCKET sockClient=socket(AF_INET,SOCK_DGRAM,0);
    SOCKADDR_IN addrClient;

    unsigned char szBuf[1024] = {0};
    TVIAlarmHead *pHead = (TVIAlarmHead*)szBuf;
    memset(pHead, 0, sizeof(TVIAlarmHead));

    pHead->ucVIConst = VI_CONST;
    pHead->ucChannelID = m_iChannel;
    pHead->ucAlarmType = m_iAlarmType;

    pHead->ucIPAddr[0] = m_ucLocalIP[3]; //真恶心，还要反过来
    pHead->ucIPAddr[1] = m_ucLocalIP[2];
    pHead->ucIPAddr[2] = m_ucLocalIP[1];
    pHead->ucIPAddr[3] = m_ucLocalIP[0];

    char ucRemoteIP[20] = {0};
    sprintf(ucRemoteIP, "%d.%d.%d.%d", m_ucRemoteIP[0], m_ucRemoteIP[1], m_ucRemoteIP[2], m_ucRemoteIP[3]);
    //addrClient.sin_addr.S_un.S_addr=inet_addr("192.168.1.74");
    addrClient.sin_addr.S_un.S_addr = inet_addr(ucRemoteIP);
    addrClient.sin_family = AF_INET;
    addrClient.sin_port = htons(REMOTE_PORT);

    sendto( sockClient, (const char*)pHead, sizeof(TVIAlarmHead), 0, (SOCKADDR*)&addrClient, sizeof(SOCKADDR) );

    closesocket(sockClient);
    cvReleaseImage(&pImage);
}

bool CUDPAlerter::init(int iAlarmType, int iChannel, unsigned char ucLocalIP[4], unsigned char ucRemoteIP[4])
{
    m_iAlarmType = iAlarmType;
    m_iChannel = iChannel;
    memcpy(m_ucLocalIP, ucLocalIP, sizeof(ucLocalIP));
    memcpy(m_ucRemoteIP, ucRemoteIP, sizeof(ucRemoteIP));

    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD( 2, 2 );

    if ( WSAStartup( wVersionRequested, &wsaData ) != 0 ) 
    {
        return false;
    }

    if( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
    {
        WSACleanup( );
        return false;
    }

    return true;
}

bool CUDPAlerter::destroy()
{
    WSACleanup( );
    return true;
}
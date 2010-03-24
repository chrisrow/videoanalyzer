#include "StdAfx.h"
#include "UDPAlerter.h"

CUDPAlerter::CUDPAlerter()
{
}

CUDPAlerter::~CUDPAlerter()
{
    this->destroy();
}

void CUDPAlerter::alert(const IplImage *pFrame)
{
    CTaskBase::run();
}

void CUDPAlerter::doRun()
{
    sendto( m_sockClient, (const char*)m_szBuf, sizeof(TVIAlarmHead), 0, 
        (SOCKADDR*)&m_addrClient, sizeof(SOCKADDR) );
}

bool CUDPAlerter::init(int iAlarmType, 
                       int iChannel, 
                       unsigned char ucLocalIP[4], 
                       unsigned char ucRemoteIP[4], 
                       int iPort)
{
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

    //初始化socket
    m_sockClient=socket(AF_INET,SOCK_DGRAM,0);
    TVIAlarmHead *pHead = (TVIAlarmHead*)m_szBuf;
    memset(pHead, 0, sizeof(TVIAlarmHead));

    pHead->ucVIConst = VI_CONST;
    pHead->ucChannelID = iChannel;
    pHead->ucAlarmType = iAlarmType;

    pHead->ucIPAddr[0] = ucLocalIP[3]; //真恶心，还要反过来
    pHead->ucIPAddr[1] = ucLocalIP[2];
    pHead->ucIPAddr[2] = ucLocalIP[1];
    pHead->ucIPAddr[3] = ucLocalIP[0];

    char szRemoteIP[20] = {0};
    sprintf(szRemoteIP, "%d.%d.%d.%d", ucRemoteIP[0], ucRemoteIP[1], ucRemoteIP[2], ucRemoteIP[3]);
    m_addrClient.sin_addr.S_un.S_addr = inet_addr(szRemoteIP);
    m_addrClient.sin_family = AF_INET;
    m_addrClient.sin_port = htons(iPort);

    return true;
}

bool CUDPAlerter::destroy()
{
    closesocket(m_sockClient);
    WSACleanup( );
    return true;
}


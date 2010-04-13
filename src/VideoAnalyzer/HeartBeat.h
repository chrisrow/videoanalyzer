#pragma once

#include "TaskBase.h"
#include "Protocol.h"
#include <afxmt.h>

//UDP��������Ϊ�˷�ֹ��Ϊ�����ԭ��������������У����Խ�����Ϊһ���첽����
class CHeartBeat : public CTaskBase
{
public:
    CHeartBeat(void);
    virtual ~CHeartBeat(void);

    bool init(int iChannel, 
              unsigned char ucLocalIP[4], 
              unsigned char ucRemoteIP[4], 
              int iPort,
              int iInterval = 180);
    bool destroy();

protected:
    virtual void doRun();

private:
    int m_iInterval;
    CEvent m_evtStop;

    SOCKET m_sockClient;
    unsigned char m_szBuf[1024];
    SOCKADDR_IN m_addrClient;
};


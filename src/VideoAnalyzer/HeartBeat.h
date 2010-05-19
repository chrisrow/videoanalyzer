#pragma once

#include "TaskBase.h"
#include "Protocol.h"
#include <afxmt.h>

//UDP报警器。为了防止因为网络的原因阻塞程序的运行，所以将其作为一个异步任务
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


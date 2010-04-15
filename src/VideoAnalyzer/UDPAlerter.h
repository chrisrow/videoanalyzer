#pragma once
#include "alerter.h"
#include "TaskBase.h"
#include "Protocol.h"


//UDP报警器。为了防止因为网络的原因阻塞程序的运行，所以将其作为一个异步任务
class CUDPAlerter : public IAlerter, public CTaskBase
{
public:
    CUDPAlerter(void);
    virtual ~CUDPAlerter(void);

    bool init(int iAlarmType, int iChannel, 
        unsigned char ucLocalIP[4], unsigned char ucRemoteIP[4], int iPort);
    virtual void alert(const IplImage *pFrame);
    bool destroy();

protected:
    virtual void doRun();

private:
    SOCKET m_sockClient;
    unsigned char m_szBuf[1024];
    SOCKADDR_IN m_addrClient;
};

//虑警后的报警确认
class CAlarmConfirm: public IAlerter, public CTaskBase
{
public:
    CAlarmConfirm(void);
    virtual ~CAlarmConfirm(void);

    bool init(unsigned uiAlarmID, unsigned char ucRemoteIP[4], int iPort);
    virtual void alert(const IplImage *pFrame);
    bool destroy();

protected:
    virtual void doRun();

private:
    SOCKET m_sockClient;
    unsigned char m_szBuf[1024];
    SOCKADDR_IN m_addrClient;
};


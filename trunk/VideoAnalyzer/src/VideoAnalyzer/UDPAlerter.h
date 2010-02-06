#pragma once
#include "alerter.h"
#include "TaskBase.h"

#pragma pack(push) 
#pragma pack(1)

struct TVIAlarmHead
{
    unsigned char ucHeadConst;
    unsigned short usGolbalCameraID;
    unsigned char ucReserver1;
    unsigned char ucVIConst;
    unsigned char ucAlarmType;
    unsigned char ucChannelID;
    unsigned char ucIPAddr[4];
    unsigned char ucReserver2;
    unsigned char ucReserver3;
};

#pragma   pack(pop)

enum
{
    ALARM_PARABOLA
};

const unsigned char VI_CONST = 50;

//UDP报警器。为了防止因为网络的原因阻塞程序的运行，所以将其作为一个异步任务
class CUDPAlerter : public IAlerter, public CTaskBase
{
public:
    CUDPAlerter(void);
    virtual ~CUDPAlerter(void);

    bool init(int iAlarmType, int iChannel, unsigned char ucLocalIP[4], unsigned char ucRemoteIP[4]);
    virtual void alert(const IplImage *pFrame);
    bool destroy();

protected:
    virtual void doRun();

private:
    int m_iAlarmType;
    int m_iChannel;
    unsigned char m_ucLocalIP[4];
    unsigned char m_ucRemoteIP[4];
    const IplImage* m_pIplImage;
};


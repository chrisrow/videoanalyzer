#pragma once


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
    ALARM_PARABOLA  = 0,
    ALARM_HEARTBEAT = 255
};

const unsigned char VI_CONST = 50;

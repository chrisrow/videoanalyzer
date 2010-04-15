#pragma once


#pragma pack(push) 
#pragma pack(1)

//一般的报警
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

//过滤报警后的处理结果
struct TVIAlarmConfirm
{
    unsigned char ucMessageType;
    unsigned short usMessageLen;
    unsigned int uiAlarmID;
    unsigned short usResult;
};

#pragma   pack(pop)

enum
{
    ALARM_PARABOLA  = 0,
    ALARM_HEARTBEAT = 255
};

const unsigned char VI_CONST = 50;

const unsigned char VI_ALARM_CONFIRM = 53;

enum
{
    ALARM_CONFIRM = 1
};

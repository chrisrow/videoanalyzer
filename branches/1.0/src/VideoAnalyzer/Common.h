#pragma once

struct TCommonParam
{
    unsigned char szLocalAddr[4];
    unsigned char szUDPServerIP[4];
    int iUDPServerPort;
    unsigned char szUDPServerIP_2[4];
    int iUDPServerPort_2;
    char szImagePath[MAX_PATH+1];
    int iHeartBeat;

    TCommonParam();

    void reset();
};

const char* loadCommonParam(const char* szFileName);

#pragma once

struct TCommonParam
{
    char szDriverDll[64+1];
    unsigned char szLocalAddr[4];
    unsigned char szUDPServerIP[4];
    int iUDPServerPort;
    char szImagePath[MAX_PATH+1];
    int iHeartBeat;
    int iDebug;

    TCommonParam();

    void reset();
};

const char* loadCommonParam(const char* szFileName);

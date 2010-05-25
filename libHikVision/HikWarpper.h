#ifndef _HIKWARPPER_H_
#define _HIKWARPPER_H_

#pragma once

#include "DllMain.h"
#include "HikVision/include/DataType.h"
#include "HikVision/include/DecodeCardSdk.h"
#include "HikVision/include/HCNetSDK.h"
#include "HikVision/include/plaympeg4.h"

#pragma comment(lib, "HikVision/DsSdk.lib")
#pragma comment(lib, "HikVision/HCNetSDK.lib")
#pragma comment(lib, "HikVision/PlayCtrl.lib")

class CHikWarpper: public ICamera
{
    friend void CALLBACK VideoRealDataCB(
        LONG      lRealHandle,
        DWORD     dwDataType,
        BYTE      *pBuffer,
        DWORD     dwBufSize,
        void      *pUser
        );

    friend void CALLBACK DecCBFunc(
        long nPort,
        char *pBuf, 
        long nSize, 
        FRAME_INFO * pFrameInfo, 
        long nReserved1,
        long nReserved2);


public:
    CHikWarpper();
    virtual ~CHikWarpper();

    virtual bool open(int iIndex);
    virtual void close();

    virtual bool setWidth(int iWidth);
    virtual bool setHeight(int iHeight);

    virtual int  getWidth();
    virtual int  getHeight();

    virtual IplImage* retrieveFrame();
    virtual bool isCallBackMode() { return true; }

protected:
//     videoInput m_camera;

    int m_iIndex;
    IplImage* m_pFrame;
    int m_iWidth;
    int m_iHeight;

    //hik
    LONG m_iUseID;
    LONG m_hPlay;
    LONG m_iPort;
};


class CHikMgr: public ICameraMgr
{
public:
    CHikMgr();
    virtual ~CHikMgr();

    virtual ICamera* createCamera();
    virtual void destroyCamera(ICamera** pCamera);
    virtual int getCount();
    virtual const char* getName(int iIndex);

private:
    char m_szName[CAM_NAME_LEN];
};

#endif

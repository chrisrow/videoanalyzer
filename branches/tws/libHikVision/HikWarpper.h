#ifndef _HIKWARPPER_H_
#define _HIKWARPPER_H_

#pragma once

#include "DllMain.h"
#include "HikVision/include/DataType.h"
#include "HikVision/include/DecodeCardSdk.h"
#include "HikVision/include/HCNetSDK.h"
#include "HikVision/include/plaympeg4.h"

#include "afxmt.h" //CEvent

#pragma comment(lib, "HikVision/DsSdk.lib")
#pragma comment(lib, "HikVision/HCNetSDK.lib")
#pragma comment(lib, "HikVision/PlayCtrl.lib")

//////////////////////////////////////////////////////
#define YUV2RGB(y, u, v, r, g, b)\
    r = y + ((v*1436) >>10);\
    g = y - ((u*352 + v*731) >> 10);\
    b = y + ((u*1814) >> 10);\
    r = r < 0 ? 0 : r;\
    g = g < 0 ? 0 : g;\
    b = b < 0 ? 0 : b;\
    r = r > 255 ? 255 : r;\
    g = g > 255 ? 255 : g;\
    b = b > 255 ? 255 : b


inline void yv12_to_rgb24 (unsigned char *src, unsigned char *dest, int width, int height)
{
    register int i,j;
    register int y0, y1, u, v;
    register int r, g, b;
    register unsigned char *s[3];
    s[0] = src;
    s[1] = s[0] + width*height;
    s[2] = s[1] + width*height/4;
    for (i = 0; i < height; i++) 
    {
        for (j = 0; j < width/2; j++) 
        {
            y0 = *(s[0])++;
            y1 = *(s[0])++;
            if (i % 2 == 0 ) 
            {
                u = *(s[1])++ - 128;
                v = *(s[2])++ - 128;
            }
            YUV2RGB (y0, u, v, r, g, b);
            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
            YUV2RGB (y1, u, v, r, g, b);
            *dest++ = r;
            *dest++ = g;
            *dest++ = b;
        }
    }
}
////////////////////////////////////////////////

class CHikWarpper: public ICamera
{
    friend void CALLBACK VideoRealDataCB(
        LONG      lRealHandle,
        DWORD     dwDataType,
        BYTE      *pBuffer,
        DWORD     dwBufSize,
        void      *pUser
        );

    friend void CALLBACK DecordCB(
        long nPort,
        char *pBuf, 
        long nSize, 
        FRAME_INFO * pFrameInfo, 
        long nUser,
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

    //通知是否解码出新的一帧
    CEvent m_hEvent;

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

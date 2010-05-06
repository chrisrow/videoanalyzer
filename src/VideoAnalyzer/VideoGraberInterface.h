#pragma once

const int CAM_NAME_LEN = 256;
const int CIF_WIDTH  = 352;
const int CIF_HEIGHT = 288;

class ICamera
{
public:
    virtual ~ICamera() {}

    virtual bool open(int iIndex) = 0;
    virtual void close() = 0;

    virtual bool setWidth(int iWidth) = 0;
    virtual bool setHeight(int iHeight) = 0;

    virtual int  getWidth() = 0;
    virtual int  getHeight() = 0;

    virtual IplImage* retrieveFrame() = 0;
};

class ICameraMgr
{
public:
    virtual ~ICameraMgr() {}

    virtual ICamera* getCamera() = 0;
    virtual int getCount() = 0;
    virtual const char* getName(int iIndex) = 0;
};



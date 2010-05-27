#pragma once

#ifdef _DEBUG
#pragma comment(lib,"cv200d.lib")
#pragma comment(lib,"cvaux200d.lib")
#pragma comment(lib,"cxcore200d.lib")
#pragma comment(lib,"cxts200d.lib")
#pragma comment(lib,"highgui200d.lib")
#pragma comment(lib,"ml200d.lib")
#else
#pragma comment(lib,"cv200.lib")
#pragma comment(lib,"cvaux200.lib")
#pragma comment(lib,"cxcore200.lib")
#pragma comment(lib,"cxts200.lib")
#pragma comment(lib,"highgui200.lib")
#pragma comment(lib,"ml200.lib")
#endif

// #include "Subject.h"
// #include "FrameReceiver.h"
#include "cv.h"
#include <vector>

class IFrameReceiver;

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

    //主动获取每一帧图像（阻塞式）
    virtual IplImage* retrieveFrame() = 0;
};

class ICameraMgr
{
public:
    virtual ~ICameraMgr() {}

    virtual ICamera* createCamera() = 0;
    virtual void destroyCamera(ICamera** pCamera) = 0;
    virtual int getCount() = 0;
    virtual const char* getName(int iIndex) = 0;
};



#ifndef _VIWARPPER_H_
#define _VIWARPPER_H_

#include "DllMain.h"
#include "videoInput/include/videoInput.h"

#pragma comment(lib, "videoInput/videoInput.lib")

class CVIWarpper: public ICamera
{
public:
    CVIWarpper();
    virtual ~CVIWarpper();

    virtual bool open(int iIndex);
    virtual void close();

    virtual bool setWidth(int iWidth);
    virtual bool setHeight(int iHeight);

    virtual int  getWidth();
    virtual int  getHeight();

    virtual IplImage* retrieveFrame();

protected:
    videoInput m_camera;

    int m_iIndex;
    IplImage* m_pFrame;
    int m_iWidth;
    int m_iHeight;
};


class CVIMgr: public ICameraMgr
{
public:
    CVIMgr();
    virtual ~CVIMgr();

    virtual ICamera* createCamera();
    virtual void destroyCamera(ICamera** pCamera);
    virtual int getCount();
    virtual const char* getName(int iIndex);

private:
    char m_szName[CAM_NAME_LEN];
    ICamera* m_pCamera;
};

#endif

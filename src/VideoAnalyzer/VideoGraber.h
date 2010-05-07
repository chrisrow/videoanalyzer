#pragma once

#include "stdafx.h"
#include <vector>
#include <afxmt.h>

#include "cv.h"
#include "highgui.h"
#include "FrameReceiver.h"
#include "VideoGraberInterface.h"
#include "Subject.h"
#include "TaskBase.h"
// #include "3rdParty/videoInput/include/videoInput.h"

#define WM_VIDEO_END (WM_USER + 1)

enum
{
    //支持文件和摄像头
    PROP_WIDTH,     //宽
    PROP_HEITHT,    //高
    PROP_FRAMERATE, //帧率(每秒的帧数)

    //仅支持文件
    PROP_STEP,         //是否单帧播放，0:是; 1:不是
    PROP_POS_MSEC,     //从第xx毫秒开始播放
    PROP_POS_FRAME,    //从第xx帧开始播放
    PROP_FRAME_CNT     //视频文件的总帧数
};

enum VIDEO_STATUS
{
    STAT_RUN,
    STAT_TOSTOP,
    STAT_STOPPED
};

//帧获取器基类
class CVideoGraber: public CTaskBase, public CSubject<IFrameReceiver*>
{
public:
    CVideoGraber(CWnd* pWnd);
    virtual ~CVideoGraber();

    virtual bool open(const char* szPath);
    virtual bool open(int iIndex);
    virtual void close();

    virtual void pause();
    virtual void resume();

    virtual bool setProperty(int iID, double value)  { return false; }
    virtual bool getProperty(int iID, double& value) { return false; }

    void loopGetFrame();
    VIDEO_STATUS getStatus();

protected:
    virtual bool doOpen(const char* szPath)   { return false; }
    virtual bool doOpen(int iIndex)           { return false; }
    virtual void afterStop();
	virtual void doAfterStop()                {  }
    virtual void release() {}

protected:
    CWnd* m_pWnd;
    CEvent m_evtPause;
    VIDEO_STATUS m_eStat ;
};

////////////////////////////////////////////////////
// class ICamera;

class CCameraWarpper: public CVideoGraber
{
public:
    CCameraWarpper(CWnd* pWnd);
    ~CCameraWarpper();

    virtual void release();
    virtual bool setProperty(int iID, double value);
    virtual bool getProperty(int iID, double& value);

protected:
    virtual bool doOpen(int iIndex);
    virtual void doRun();
	virtual void doAfterStop();

protected:
    ICamera* m_pCamera;
};

////////////////////////////////////////////////////

class COpenCVWarpper: public ICamera
{
public:
    COpenCVWarpper();
    virtual ~COpenCVWarpper();

    virtual bool open(int iIndex);
    virtual void close();

    virtual bool setWidth(int iWidth);
    virtual bool setHeight(int iHeight);

    virtual int  getWidth();
    virtual int  getHeight();

    virtual IplImage* retrieveFrame();

protected:
    CvCapture* m_pCapture;
};

//==================================================================
////////////////////////////////////////////////////
//视频文件操作类
class CVideoFile: public CVideoGraber
{
public:
    CVideoFile(CWnd* pWnd);
    virtual ~CVideoFile();
    virtual void release();

    virtual bool setProperty(int iID, double value);
    virtual bool getProperty(int iID, double& value);

protected:
    virtual bool doOpen(const char* szPath);
    virtual void doRun();
	virtual void doAfterStop();

private:
    CvCapture* m_pCapture;
    int m_iFPS;
    int m_iWaitTime;
    bool m_bStep;//单帧播放
    unsigned int m_iStartFrame;
    unsigned int m_uFrame;//帧计数器
};


////////////////////////////////////////////////////
typedef ICameraMgr* (*PGETCAMERAMGR)();//getCameraMgr
typedef void (*PDESTROYCAMERAMGR)();//destoryCameraMgr

class CCameraDllMgr
{

public:
    static bool loadDll();
    static void freeDll();
    static ICameraMgr* getCameraMgr();
    static void destoryCameraMgr();

private:
    static PGETCAMERAMGR m_pGetCameraMgr;
    static PDESTROYCAMERAMGR m_pDestroyCameraMgr;
    static HINSTANCE m_hIns;
};


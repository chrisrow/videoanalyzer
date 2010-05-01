#pragma once

#include "FrameReceiver.h"
#include "cv.h"
#include "highgui.h"

class CVideoRecorder: public IFrameReceiver
{
public:
    CVideoRecorder(void);
    ~CVideoRecorder(void);

    virtual void updateFrame(const IplImage *pFrame);

    void setResolution(int iWidth, int iHeight);
    bool start(const char* szFileName);
    void stop();

private:
    CvVideoWriter *m_pWriter;
    int m_iWidth, m_iHeight;
};

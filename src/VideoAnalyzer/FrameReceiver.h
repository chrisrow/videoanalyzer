
#pragma once

#include "cv.h"

//图像帧接收器基类
class IFrameReceiver
{
public:
    virtual ~IFrameReceiver() {}
    virtual void updateFrame(const IplImage *pFrame) = 0;
};

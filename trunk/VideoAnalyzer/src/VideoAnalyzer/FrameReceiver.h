
#pragma once

#include "cv.h"

//ͼ��֡����������
class IFrameReceiver
{
public:
    virtual ~IFrameReceiver() {}
    virtual void updateFrame(const IplImage *pFrame) = 0;
};

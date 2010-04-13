
#pragma once

#include "cv.h"

//±¨¾¯Æ÷»ùÀà
class IAlerter
{
public:
    virtual ~IAlerter() {}
    virtual void alert(const IplImage *pFrame) = 0;
};

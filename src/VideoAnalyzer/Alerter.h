
#pragma once

#include "cv.h"

//����������
class IAlerter
{
public:
    virtual ~IAlerter() {}
    virtual void alert(const IplImage *pFrame) = 0;
};

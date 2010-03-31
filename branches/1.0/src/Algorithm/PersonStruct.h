#pragma once

#include "cv.h"

//��Ա�����ز���
struct TPersonDetect
{
    int warn_pt1_x;
    int warn_pt1_y;
    int warn_pt2_x;
    int warn_pt2_y;
    IplImage* mask;

    TPersonDetect(): warn_pt1_x(0), warn_pt1_y(0), warn_pt2_x(0), warn_pt2_y(0), mask(NULL) {}
};

#pragma once

#include "cv.h"
#include <vector>

typedef std::vector<CPoint> PolyLine; //折线
typedef PolyLine Line;     //直线

typedef std::vector<PolyLine> PolyLineArray;

//人员检测相关参数
struct TPersonDetect
{
    Line warnLine;
    PolyLineArray maskLine;
    IplImage* mask;

    TPersonDetect(): mask(NULL) {}
    void reset()
    {
        warnLine.clear();
        maskLine.clear();
        if (mask)
        {
            cvReleaseImage(&mask);
            mask = NULL;
        }
    }
};

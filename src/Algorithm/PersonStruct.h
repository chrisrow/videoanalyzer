#pragma once

#include "cv.h"
#include <vector>

typedef std::vector<CPoint> PolyLine; //����
typedef PolyLine Line;     //ֱ��

typedef std::vector<PolyLine> PolyLineArray;

//��Ա�����ز���
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

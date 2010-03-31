#pragma once

#include "cv.h"
#include <vector>

typedef std::vector<CPoint> PolyLine; //����
typedef PolyLine Line;     //ֱ��

typedef std::vector<PolyLine> PolyLineArray;

//��Ա�����ز���
struct TPersonDetect
{
    Line warnLing;
    PolyLineArray maskLine;
    IplImage* mask;

    TPersonDetect(): mask(NULL) {}
};

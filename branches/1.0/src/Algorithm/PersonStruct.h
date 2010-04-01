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
    virtual ~TPersonDetect() { reset(); }

    inline void reset();
};

void TPersonDetect::reset()
{
    warnLine.clear();
    maskLine.clear();
    if (mask)
    {
        cvReleaseImage(&mask);
        mask = NULL;
    }
    return;
}

inline void makeMask(PolyLineArray& maskLine, IplImage* mask)
{
    cvSet(mask, cvScalar(1, 1, 1, 1));

    CvScalar color = cvScalar(0, 0, 0, 0);
    for (unsigned i = 0; i < maskLine.size(); i++)
    {
        PolyLine& line = maskLine[i];

        if (line.size() < 2)
        {
            continue;
        }

        for (unsigned j = 0; j < line.size() - 1; j++)
        {
            cvLine(mask, cvPoint(line[j].x, line[j].y), cvPoint(line[j+1].x, line[j+1].y), 
                color, 1, CV_AA, 0 );
        }

        int arr[1];
        arr[0] = line.size();
        CvPoint ** pt = new CvPoint*[1];
        pt[0] = new CvPoint[line.size()];
        for (unsigned j = 0; j < line.size(); j++)
        {
            pt[0][j] = cvPoint(line[j].x, line[j].y);
        }
        cvFillPoly(mask, pt, arr, 1, color);

        delete []pt[0];
        pt[0] = NULL;
        delete []pt;
    }

    return;
}

#pragma once

#include "Analyzer.h"
#include "Common/frames.h"


//抛物检测类的包装类，以屏蔽上层对其调用的差异性
class CRoadWarpper: public CAnalyzer
{
public:
    CRoadWarpper();
    virtual ~CRoadWarpper();

    void reset();

    virtual const IplImage* analysis(const IplImage *pFrame);
    virtual ALERTTYPE haveAlert();

    virtual void updateFrame(const IplImage *pFrame) ;

private:
    IplImage *m_pFrameOut, *m_pFrameTmp;
    CvMemStorage* m_storage;
    CvFont m_font;
};



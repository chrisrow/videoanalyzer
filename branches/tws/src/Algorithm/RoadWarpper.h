#pragma once

#include "Analyzer.h"
#include "Common/frames.h"


//��������İ�װ�࣬�������ϲ������õĲ�����
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



#pragma once

#include "Analyzer.h"
#include "Common/frames.h"

class CPersonDetect;

//抛物检测类的包装类，以屏蔽上层对其调用的差异性
class CPersonWarpper: public CAnalyzer
{
public:
    CPersonWarpper();
    virtual ~CPersonWarpper();

    void reset();

    virtual const IplImage* analysis(const IplImage *pFrame);
    virtual ALERTTYPE haveAlert();

    virtual void updateFrame(const IplImage *pFrame) ;

private:
    CFrameContainer *m_pFrameContainer, *m_pOutFrameContainer, *m_pFrame_matlabFunced;
    CPersonDetect* m_pDetector;
};



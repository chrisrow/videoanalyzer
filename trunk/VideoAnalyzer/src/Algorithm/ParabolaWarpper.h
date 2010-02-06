#pragma once

#include "CParabolaDetect.h"
#include "Analyzer.h"

//抛物检测类的包装类，以屏蔽上层对其调用的差异性
class CParabolaWarpper: public CAnalyzer
{
public:
    CParabolaWarpper();
    virtual ~CParabolaWarpper();

    void reset();

    virtual const IplImage* analysis(const IplImage *pFrame);
    virtual ALERTTYPE haveAlert();

    virtual void updateFrame(const IplImage *pFrame) ;

private:
    CFrameContainer *m_pFrameContainer, *m_pOutFrameContainer, *m_pFrame_matlabFunced;
    CParabolaDetect* m_pDetector;
};



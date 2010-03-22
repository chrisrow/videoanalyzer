#pragma once

#include "CPersonDetect.h"
#include "Analyzer.h"

//��������İ�װ�࣬�������ϲ������õĲ�����
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



#pragma once

#include "Analyzer.h"
#include "Common/frames.h"

class CPersonDetect;

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
    unsigned char* m_RGB_template;
    CFrameContainer *m_pFrameContainer, *m_pOutFrameContainer, *m_pFrame_matlabFunced;
    CPersonDetect* m_pDetector;
};



#include "stdafx.h"
#include "ParabolaWarpper.h"


CParabolaWarpper::CParabolaWarpper()
: m_pFrameContainer(NULL)
, m_pOutFrameContainer(NULL)
, m_pFrame_matlabFunced(NULL)
, m_pDetector(NULL)
{

}

CParabolaWarpper::~CParabolaWarpper()
{
    reset();
}

void CParabolaWarpper::reset()
{
    delete m_pFrameContainer;
    delete m_pOutFrameContainer;
    delete m_pFrame_matlabFunced;
    delete m_pDetector;

    m_pFrameContainer = NULL;
    m_pOutFrameContainer = NULL;
    m_pFrame_matlabFunced = NULL;
    m_pDetector = NULL;
}

const IplImage* CParabolaWarpper::analysis(const IplImage *pFrame)
{
    if (NULL == m_pDetector
        || pFrame->width != m_pFrameContainer->getWidth() 
        || pFrame->height != m_pFrameContainer->getHeight())
    {
        reset();

        m_pDetector  = new CParabolaNatural(pFrame->width, pFrame->height, 0);
        m_pFrameContainer = new CFrameContainer(pFrame->width, pFrame->height);
        m_pOutFrameContainer = new CFrameContainer(pFrame->width, pFrame->height);
        m_pFrame_matlabFunced = new CFrameContainer(pFrame->width, pFrame->height);
    }

    memcpy(m_pFrameContainer->m_BmpBuffer, pFrame->imageData, pFrame->imageSize);
    m_pDetector->ParaDetectTwo(m_pFrameContainer, m_pOutFrameContainer);

    return m_pOutFrameContainer->getImage();
}

ALERTTYPE CParabolaWarpper::haveAlert()
{
    return (m_pDetector->m_alarm_flg == 0) ? AT_NONE : AT_WARNING;
}

void CParabolaWarpper::updateFrame(const IplImage *pFrame)
{
    const IplImage* p = this->analysis(pFrame) ;
//     if ( this->haveAlert() != AT_NONE)
    if (m_pDetector->m_alarm_flg)
    {
        m_pDetector->ImgMoveObjectDetect(m_pFrameContainer, m_pFrame_matlabFunced );

        //±¨¾¯
        FOR_EACH(IAlerter*, alert, m_pFrame_matlabFunced->getImage());
    }
}

#include "stdafx.h"
#include "PersonWarpper.h"
#include "CPersonDetect.h"
#include "Macro.h"

extern int g_debug;

CPersonWarpper::CPersonWarpper()
: m_pFrameContainer(NULL)
, m_pOutFrameContainer(NULL)
, m_pFrame_matlabFunced(NULL)
, m_pDetector(NULL)
{

}

CPersonWarpper::~CPersonWarpper()
{
    reset();
}

void CPersonWarpper::reset()
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

const IplImage* CPersonWarpper::analysis(const IplImage *pFrame)
{
	if (NULL == m_pDetector
		|| pFrame->width != m_pFrameContainer->getWidth() 
		|| pFrame->height != m_pFrameContainer->getHeight())
	{
		reset();

        m_pDetector = new CPersonDetect(pFrame->width, pFrame->height, YUVTYPE_444);

		m_pFrameContainer = new CFrameContainer(pFrame->width, pFrame->height);
		m_pOutFrameContainer = new CFrameContainer(pFrame->width, pFrame->height);
		m_pFrame_matlabFunced = new CFrameContainer(pFrame->width, pFrame->height);
    }

	memcpy(m_pFrameContainer->m_BmpBuffer, pFrame->imageData, pFrame->imageSize);

        m_pDetector->PersenDetect_Process(m_pFrame_matlabFunced, m_pOutFrameContainer,
        m_pFrameContainer, NULL, m_pDetector->LEFTTORIGNT, 60/*m_nInputFrameNum*/); 

	return m_pOutFrameContainer->getImage();
}

ALERTTYPE CPersonWarpper::haveAlert()
{
    ALERTTYPE result = m_pDetector->b_First_Alarm ? AT_WARNING : AT_NONE;
    m_pDetector->b_First_Alarm = false;
    return result;
}

void CPersonWarpper::updateFrame(const IplImage *pFrame)
{
    const IplImage* p = analysis(pFrame) ;

    if ( this->haveAlert() != AT_NONE)
    {
        //生成报警截图
        m_pDetector->ImgMoveObjectDetect(m_pFrameContainer);
        SHOW_IMAGE("alarm", m_pFrameContainer->getImage());

        //报警
        FOR_EACH(IAlerter*, alert, m_pFrameContainer->getImage());
    }
}

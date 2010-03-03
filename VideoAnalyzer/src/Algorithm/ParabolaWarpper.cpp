#include "stdafx.h"
#include "ParabolaWarpper.h"

extern struct ParamStruct ParamSet ;

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

		switch ( ParamSet.iStyleChange )
		{
		case 0: 
			m_pDetector  = new CParabolaLineOneSide(pFrame->width, pFrame->height);
			break;
		case 1:
			m_pDetector  = new CParabolaLineTwoSide(pFrame->width, pFrame->height);
			break;
		case 2:
			m_pDetector  = new CParabolaCurveOneSide(pFrame->width, pFrame->height);
			break;
		case 3:
			m_pDetector  = new CParabolaCurveTwoSide(pFrame->width, pFrame->height);
			break;
		case 4:
			m_pDetector  = new CParabolaTree(pFrame->width, pFrame->height);
			break;
		case 5:
			m_pDetector  = new CParabolaTree(pFrame->width, pFrame->height);
			break;
		default : 
			m_pDetector  = new CParabolaLineTwoSide(pFrame->width, pFrame->height);
			break;
		}


		m_pFrameContainer = new CFrameContainer(pFrame->width, pFrame->height);
		m_pOutFrameContainer = new CFrameContainer(pFrame->width, pFrame->height);
		m_pFrame_matlabFunced = new CFrameContainer(pFrame->width, pFrame->height);
	}

	memcpy(m_pFrameContainer->m_BmpBuffer, pFrame->imageData, pFrame->imageSize);
	m_pDetector->ParaDetectTwo(m_pFrameContainer, m_pOutFrameContainer); //jojo

	return m_pOutFrameContainer->getImage();
}

ALERTTYPE CParabolaWarpper::haveAlert()
{
    return (m_pDetector->m_AlarmFlg == 0) ? AT_NONE : AT_WARNING;
}

void CParabolaWarpper::updateFrame(const IplImage *pFrame)
{
    const IplImage* p = this->analysis(pFrame) ;
//     if ( this->haveAlert() != AT_NONE)
    if (m_pDetector->m_AlarmFlg)
    {
        m_pDetector->ImgMoveObjectDetect(m_pFrameContainer, m_pFrame_matlabFunced );

        //±¨¾¯
        FOR_EACH(IAlerter*, alert, m_pFrame_matlabFunced->getImage());
    }
}

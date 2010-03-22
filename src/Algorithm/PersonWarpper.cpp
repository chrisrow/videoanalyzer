#include "stdafx.h"
#include "PersonWarpper.h"

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

    // TODO: 人员检测总入口
// 	m_pDetector->ParaDetectTwo(m_pFrameContainer, m_pOutFrameContainer); //jojo

	return m_pOutFrameContainer->getImage();
}

ALERTTYPE CPersonWarpper::haveAlert()
{
    // TODO: 判断是否需要报警
//     return (m_pDetector->m_AlarmFlg == 0) ? AT_NONE : AT_WARNING;
    return AT_NONE;
}

void CPersonWarpper::updateFrame(const IplImage *pFrame)
{
    const IplImage* p = this->analysis(pFrame) ;

    if ( this->haveAlert() != AT_NONE)
    {
        //生成报警截图
//         m_pDetector->ImgMoveObjectDetect(m_pFrameContainer, m_pFrame_matlabFunced );

        //报警
        FOR_EACH(IAlerter*, alert, m_pFrame_matlabFunced->getImage());
    }
}

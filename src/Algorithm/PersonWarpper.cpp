#include "stdafx.h"
#include "PersonWarpper.h"
#include "CPersonDetect.h"
#include "VideoAnalyzer/Line.h"

CPersonWarpper::CPersonWarpper()
: m_RGB_template(NULL)
, m_pFrameContainer(NULL)
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
    delete m_RGB_template;
    delete m_pFrameContainer;
    delete m_pOutFrameContainer;
    delete m_pFrame_matlabFunced;
    delete m_pDetector;

    m_RGB_template = NULL;
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

        m_RGB_template = new unsigned char[pFrame->width * pFrame->height];
        Line* getTemplate = new Line(pFrame->width, pFrame->height);
        bool b_get_template_is_OK = getTemplate->GetTemplateParameter(m_RGB_template,
            _T("\\set_TestGUIDLG.set"), pFrame->width, pFrame->height);
        if(!b_get_template_is_OK)
        {
            MessageBoxA( NULL, "模板参数读取失败!", "Line", MB_OK );
        }
        delete getTemplate;
    }

	memcpy(m_pFrameContainer->m_BmpBuffer, pFrame->imageData, pFrame->imageSize);

    m_pDetector->PersenDetect_Process_Channel_3(m_pFrame_matlabFunced, m_pOutFrameContainer,
        m_pFrameContainer, m_RGB_template, m_pDetector->LEFTTORIGNT, 60/*m_nInputFrameNum*/); 

	return m_pOutFrameContainer->getImage();
}

ALERTTYPE CPersonWarpper::haveAlert()
{
    return m_pDetector->b_First_Alarm ? AT_WARNING : AT_NONE;
}

void CPersonWarpper::updateFrame(const IplImage *pFrame)
{
    const IplImage* p = analysis(pFrame) ;

    if ( this->haveAlert() != AT_NONE)
    {
        //生成报警截图
        m_pDetector->ImgMoveObjectDetect(m_pFrameContainer);

        //报警
        FOR_EACH(IAlerter*, alert, m_pFrame_matlabFunced->getImage());
    }
}

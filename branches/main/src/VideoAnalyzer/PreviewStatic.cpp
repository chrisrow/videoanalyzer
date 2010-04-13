#include "StdAfx.h"
#include "PreviewStatic.h"

CPreviewStatic::CPreviewStatic(void)
{
}

CPreviewStatic::~CPreviewStatic(void)
{
    m_cvvImage.Destroy();
}

void CPreviewStatic::showImage(const IplImage *pImage)
{
    if (!pImage)
    {
        return;
    }

    m_cvvImage.CopyOf(const_cast<IplImage*>(pImage));
    DrawItem(NULL);

}

void CPreviewStatic::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
	CDC *pDC = GetDC();
	HDC hDC= pDC->GetSafeHdc(); 
    GetClientRect(&m_rect);

	m_cvvImage.DrawToHDC(hDC, &m_rect);

	ReleaseDC(pDC);
}

const IplImage* CPreviewStatic::getImage()
{
	return m_cvvImage.GetImage();
}

BEGIN_MESSAGE_MAP(CPreviewStatic, CStatic)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CPreviewStatic::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;

    return CStatic::OnEraseBkgnd(pDC);
}


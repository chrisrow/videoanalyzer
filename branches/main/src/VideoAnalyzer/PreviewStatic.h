#pragma once
#include "afxwin.h"

#include "cv.h"
#include "highgui.h"

class CPreviewStatic : public CStatic
{
public:
    CPreviewStatic(void);
    virtual ~CPreviewStatic(void);

    void showImage(const IplImage *pImage);
    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	const IplImage* getImage();

private:
    CvvImage m_cvvImage;
    CRect m_rect;

public:
    DECLARE_MESSAGE_MAP()
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

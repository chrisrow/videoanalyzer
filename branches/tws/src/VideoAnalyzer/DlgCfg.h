#pragma once

#include "cv.h"

class CDlgCfgBase
{
public:
    CDlgCfgBase(): m_pImage(NULL) {}
    virtual ~CDlgCfgBase() {}

    inline void setImage(const IplImage *pImage);

protected:
    IplImage *m_pImage;
};

void CDlgCfgBase::setImage(const IplImage *pImage)
{
    if (pImage)
    {
        m_pImage = cvCloneImage(pImage);
    }
    else
    {
        m_pImage = NULL;
    }
}



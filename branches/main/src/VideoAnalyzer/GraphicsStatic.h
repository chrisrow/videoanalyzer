#pragma once
#include "afxwin.h"

#include <vector>

#include "cv.h"
#include "highgui.h"


enum GraphicsType
{
    GT_Polyline,
    GT_Rectangle
};

typedef std::vector<CPoint> PolyLine;
typedef std::vector<PolyLine> PolyLineArray;
typedef std::vector<CRect> RectArray;

class CGraphicsStatic :
    public CStatic
{
protected:
    IplImage *m_image, *m_imageDraft, *m_imageOrg;
    bool m_bStart;
    CPoint m_ptPre, m_ptCur;

    GraphicsType m_grapType;
    CvScalar m_color;

    PolyLine m_tmpPolyLine;
    PolyLineArray* m_pPolyLineArray;

    CRect m_tmpRect;
    RectArray* m_pRectArray;
public:
    CPoint m_OrgPoint ;
    CPoint m_EndPoint ;
public:
    CGraphicsStatic(void);
    ~CGraphicsStatic(void);

    //��ʾ����ԭͼ��
    bool ShowImage(const IplImage* pFrame);
    bool Refresh();

    //����ͼ�����ͣ����ߡ�����
    inline void SetGraphicsType(GraphicsType type);
    //����ͼ����ɫ
    inline void SetGraphicsColor(COLORREF color);

    //���á������������
    inline void setPolyLineArray(PolyLineArray& array);
    inline void clearPolyLineArray();

    //���á���վ�������
    inline void setRectArray(RectArray& array);
    inline void clearRectArray();

protected:
    bool DramImage(IplImage *img);
    void Reset();

    DECLARE_MESSAGE_MAP()

    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

// inline functions

void CGraphicsStatic::SetGraphicsType(GraphicsType type)
{
    m_grapType = type;
}

void CGraphicsStatic::SetGraphicsColor(COLORREF color)
{
    m_color = cvScalar(GetBValue(color), GetGValue(color), GetRValue(color), 0);
}

void CGraphicsStatic::setPolyLineArray(PolyLineArray& array)
{ 
    m_pPolyLineArray = &array; 
}

void CGraphicsStatic::clearPolyLineArray() 
{ 
	if (m_pPolyLineArray)
	{
		m_pPolyLineArray->clear(); 
	}
}

void CGraphicsStatic::setRectArray(RectArray& array) 
{ 
    m_pRectArray = &array; 
}

void CGraphicsStatic::clearRectArray() 
{ 
	if (m_pRectArray)
	{
		m_pRectArray->clear(); 
	}
}

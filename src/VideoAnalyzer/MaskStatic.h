#pragma once
#include "afxwin.h"

#include <vector>

#include "cv.h"
#include "highgui.h"

enum GraphicsType_2
{
    GT2_Polyline = 0, //����
    GT2_Line,         //ֱ��
    GT2_Rectangle
};

const COLORREF COLOR_LINE = RGB(255, 0, 0); //��ɫ��Ԥ����
const COLORREF COLOR_POLY = RGB(0, 0, 255); //��ɫ����������

typedef std::vector<CPoint> PolyLine; //����
typedef PolyLine Line;     //ֱ��

typedef std::vector<PolyLine> PolyLineArray;
typedef PolyLineArray LineArray;

typedef std::vector<CRect> RectArray;

class CMaskStatic : public CStatic
{
protected:
    IplImage *m_pImageOrg;
    IplImage *m_pImage;
    IplImage *m_pImageDraft;


    bool m_bStart;
    CPoint m_ptPre, m_ptCur;
    bool m_bFill;

    //ͼ����������ɫ
    GraphicsType_2 m_grapType;

    //�����
    PolyLine m_tmpPolyLine;
    PolyLineArray* m_pPolyLineArray;
    CvScalar m_polyLineColor;

    //ֱ��
    Line m_tmpLine;
    LineArray* m_pLineArray;
    CvScalar m_lineColor;

    CRect m_tmpRect;
    RectArray* m_pRectArray;
    CvScalar m_rectColor;

public:
    CPoint m_OrgPoint ;
    CPoint m_EndPoint ;

public:
    CMaskStatic(void);
    ~CMaskStatic(void);

    //��ʾ����ԭͼ��
    bool ShowImage(const IplImage* pFrame);
    bool Refresh();

    //����ͼ�����ͣ����ߡ�����
    inline void SetGraphicsType(GraphicsType_2 type, COLORREF color = RGB(255, 0, 0));

    //���á������������
    inline void setPolyLineArray(PolyLineArray& array);
    inline void clearPolyLineArray();

    //ֱ��
    inline void setLine(LineArray& line);
    inline void clearLine();

    //���á���վ�������
    inline void setRectArray(RectArray& array);
    inline void clearRectArray();

    inline void setFill(bool bFill = true);

protected:
    bool DramImage(IplImage *img);
    void Reset();

    void DrawPolylineArray(IplImage* img, PolyLineArray* array, CvScalar* color);
    void DrawPolyline(IplImage* img, PolyLine* line, CvScalar* color);
    void DrawLine(IplImage* img, Line* line, CvScalar* color);
    void DrawRect(IplImage* img, RectArray* array, CvScalar* color);
    inline CvScalar& getColor(GraphicsType_2 type);

    DECLARE_MESSAGE_MAP()

    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};

// inline functions

void CMaskStatic::SetGraphicsType(GraphicsType_2 type, COLORREF color)
{
    m_grapType = type;

    switch (m_grapType)
    {
    case GT2_Polyline:
        m_polyLineColor = cvScalar(GetBValue(color), GetGValue(color), GetRValue(color), 0);
    	break;
    case GT2_Line:
        m_lineColor = cvScalar(GetBValue(color), GetGValue(color), GetRValue(color), 0);
        break;
    case GT2_Rectangle:
        m_rectColor = cvScalar(GetBValue(color), GetGValue(color), GetRValue(color), 0);
        break;
    }
}

void CMaskStatic::setPolyLineArray(PolyLineArray& array)
{ 
    m_pPolyLineArray = &array; 
}

void CMaskStatic::clearPolyLineArray() 
{ 
	if (m_pPolyLineArray)
	{
		m_pPolyLineArray->clear(); 
	}
}

void CMaskStatic::setLine(LineArray& line)
{ 
    m_pLineArray = &line; 
}

void CMaskStatic::clearLine() 
{ 
    if (m_pLineArray)
    {
        m_pLineArray->clear(); 
    }
}

void CMaskStatic::setRectArray(RectArray& array) 
{ 
    m_pRectArray = &array; 
}

void CMaskStatic::clearRectArray() 
{ 
	if (m_pRectArray)
	{
		m_pRectArray->clear(); 
	}
}

CvScalar& CMaskStatic::getColor(GraphicsType_2 type)
{
    static CvScalar black = cvScalar(0, 0, 0, 0);
    switch (type)
    {
    case GT2_Line :      return m_lineColor;
    case GT2_Polyline :  return m_polyLineColor;
    case GT2_Rectangle : return m_rectColor;
    default: 
        return black;
    }
}

void CMaskStatic::setFill(bool bFill)
{
    m_bFill = bFill;
}


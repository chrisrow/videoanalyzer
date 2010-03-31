#pragma once
#include "afxwin.h"

#include <vector>

#include "cv.h"
#include "highgui.h"

enum GraphicsType_2
{
    GT2_Polyline = 0, //折线
    GT2_Line,         //直线
    GT2_Rectangle
};

const COLORREF COLOR_LINE = RGB(255, 0, 0); //红色：预警线
const COLORREF COLOR_POLY = RGB(0, 0, 255); //蓝色：遮罩区域

typedef std::vector<CPoint> PolyLine; //折线
typedef PolyLine Line;     //直线

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

    //图像类型与颜色
    GraphicsType_2 m_grapType;

    //多边形
    PolyLine m_tmpPolyLine;
    PolyLineArray* m_pPolyLineArray;
    CvScalar m_polyLineColor;

    //直线
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

    //显示、复原图像
    bool ShowImage(const IplImage* pFrame);
    bool Refresh();

    //设置图形类型：折线、矩形
    inline void SetGraphicsType(GraphicsType_2 type, COLORREF color = RGB(255, 0, 0));

    //设置、清空折线数组
    inline void setPolyLineArray(PolyLineArray& array);
    inline void clearPolyLineArray();

    //直线
    inline void setLine(LineArray& line);
    inline void clearLine();

    //设置、清空矩形数组
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


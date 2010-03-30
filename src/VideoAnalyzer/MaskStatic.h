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
typedef std::vector<CPoint> Line;     //直线

typedef std::vector<PolyLine> PolyLineArray;
typedef std::vector<CRect> RectArray;

class CMaskStatic : public CStatic
{
protected:
    IplImage *m_image, *m_imageDraft, *m_imageOrg;


    IplImage *m_pLayer, *m_pLayerDraft; //用于显示线条矩形的图像层
    IplImage *m_pImageOrg;  //原始的视频截图
    IplImage *m_pImage;     //最终显示在界面上的

    bool m_bStart;
    CPoint m_ptPre, m_ptCur;

    //图像类型与颜色
    GraphicsType_2 m_grapType;

    //多边形
    PolyLine m_tmpPolyLine;
    PolyLineArray* m_pPolyLineArray;
    CvScalar m_polyLineColor;

    //直线
    Line m_tmpLine;
    Line* m_pLine;
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
    inline void setLine(Line& line);
    inline void clearLine();

    //设置、清空矩形数组
    inline void setRectArray(RectArray& array);
    inline void clearRectArray();

protected:
    bool DramImage(IplImage *img);
    void Reset();

    void DrawPolyline(IplImage* img, PolyLineArray* array, CvScalar* color);
    void DrawLine(IplImage* img, Line* line, CvScalar* color);
    void DrawRect(IplImage* img, RectArray* array, CvScalar* color);

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

void CMaskStatic::setLine(Line& line)
{ 
    m_pLine = &line; 
}

void CMaskStatic::clearLine() 
{ 
    if (m_pLine)
    {
        m_pLine->clear(); 
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

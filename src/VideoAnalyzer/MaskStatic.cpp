#include "StdAfx.h"
#include "MaskStatic.h"

const int LINE_WIDTH = 2;

CMaskStatic::CMaskStatic(void)
    : m_pImage(NULL)
    , m_pImageOrg(NULL)
    , m_pImageDraft(NULL)
    , m_bStart(false)
    , m_bFill(false)
    , m_grapType(GT2_Polyline)
    , m_pPolyLineArray(NULL)
    , m_pLineArray(NULL)
    , m_pRectArray(NULL)
{
    m_OrgPoint.x = 0 ;
    m_OrgPoint.y = 0 ;

    m_EndPoint.x = 0 ;
    m_EndPoint.y = 0 ;

    m_polyLineColor = cvScalar(0, 0, 255, 0);
    m_lineColor = cvScalar(255, 0, 0, 0);
    m_rectColor = cvScalar(255, 255, 0, 0);
}

CMaskStatic::~CMaskStatic(void)
{
    Reset();
}

BEGIN_MESSAGE_MAP(CMaskStatic, CStatic)
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


bool CMaskStatic::ShowImage(const IplImage* pFrame)
{
    //判断大小是否改变
    if (m_pImage && pFrame
        && (m_pImage->width != pFrame->width || m_pImage->height != pFrame->height))
    {
        this->Reset();
    }

    //创建或者复制视频图像
	if (pFrame)
	{
        if (m_pImage)
        {
            cvCopy(pFrame, m_pImage);
        }
        else
        {
            m_pImage = cvCloneImage(pFrame);
        }
	}
	else
	{
        if (m_pImage)
        {
            cvZero(m_pImage);
        }
        else
        {
            m_pImage = cvCreateImage( cvSize(352, 288), 8, 3);
            cvZero(m_pImage);
        }
	}

    //创建或者复制视频图像
    if (m_pImageOrg)
    {
        cvCopy(m_pImage, m_pImageOrg);
    }
    else
    {
        m_pImageOrg = cvCloneImage(m_pImage);
    }

    if (m_pPolyLineArray)
    {
        DrawPolylineArray(m_pImage, m_pPolyLineArray, &m_polyLineColor);
    }
    if (m_pLineArray)
    {
        DrawPolylineArray(m_pImage, m_pLineArray, &m_lineColor);
    }
    if (m_pRectArray)
    {
        DrawRect(m_pImage, m_pRectArray, &m_rectColor);
    }

    //创建或者复制视频图像
    if (m_pImageDraft)
    {
        cvCopy(m_pImage, m_pImageDraft);
    }
    else
    {
        m_pImageDraft = cvCloneImage(m_pImage);
    }

    SetWindowPos(NULL, -1, -1, m_pImage->width, m_pImage->height, SWP_NOMOVE);
	this->DrawItem(NULL);

    return true;
}

void CMaskStatic::DrawLine(IplImage* img, Line* line, CvScalar* color)
{
    if (line && line->size() >= 2)
    {
        cvLine(img, cvPoint((*line)[0].x, (*line)[0].y), cvPoint((*line)[1].x, (*line)[1].y), 
            *color, LINE_WIDTH, CV_AA, 0 );
    }
}

void CMaskStatic::DrawPolyline(IplImage* img, PolyLine* line, CvScalar* color)
{
    if (line->size() < 2)
    {
        return;
    }

    for (unsigned j = 0; j < line->size() - 1; j++)
    {
        cvLine(img, cvPoint((*line)[j].x, (*line)[j].y), cvPoint((*line)[j+1].x, (*line)[j+1].y), 
            *color, LINE_WIDTH, CV_AA, 0 );
    }
}

void CMaskStatic::DrawPolylineArray(IplImage* img, PolyLineArray* array, CvScalar* color)
{
    if (array)
    {
        for (unsigned i = 0; i < array->size(); i++)
        {
            PolyLine& line = (*array)[i];

            DrawPolyline(img, &line, color);

            if (m_bFill && line.size() > 2)
            {
                int arr[1];
                arr[0] = line.size();
                CvPoint ** pt = new CvPoint*[1];
                pt[0] = new CvPoint[line.size()];
                for (unsigned j = 0; j < line.size(); j++)
                {
                    pt[0][j] = cvPoint(line[j].x, line[j].y);
                }
                cvFillPoly(img, pt, arr, 1, m_polyLineColor);
            }
        }
    }
}

void CMaskStatic::DrawRect(IplImage* img, RectArray* array, CvScalar* color)
{
    if (array)
    {
        for (unsigned i = 0; i < array->size(); i++)
        {
            CRect& rect = (*array)[i];
            cvRectangle(img, cvPoint(rect.left, rect.top), cvPoint(rect.right, rect.bottom), 
                *color, LINE_WIDTH, CV_AA, 0 );
        }
    }
}

bool CMaskStatic::Refresh()
{
    cvCopy(m_pImageOrg, m_pImage);
    cvCopy(m_pImageOrg, m_pImageDraft);

    DramImage(m_pImageOrg);

    return true;
}

void CMaskStatic::Reset()
{
#define RELEASE_IMAGE(a) \
    do \
    {\
        if (a != NULL)\
        {\
            cvReleaseImage(&a);\
            a = NULL;\
        }\
    } while (0)

    RELEASE_IMAGE(m_pImageDraft);
    RELEASE_IMAGE(m_pImageOrg);
    RELEASE_IMAGE(m_pImage);

    m_bStart = false;
    m_grapType = GT2_Polyline;

    m_pPolyLineArray = NULL;
    m_pLineArray = NULL;
    m_pRectArray = NULL;
}

bool CMaskStatic::DramImage(IplImage *img)
{
    CDC *pDC = GetDC(); 
    HDC hDC= pDC->GetSafeHdc(); 
    CRect rect; 
    GetClientRect(&rect); 
    CvvImage cimg; 

    cimg.CopyOf(img,1); 
    cimg.DrawToHDC(hDC,&rect); 

    ReleaseDC(pDC); 
    return true;
}

void CMaskStatic::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
    if(m_pImageDraft)
    {
        DramImage(m_pImageDraft);
    }
}

void CMaskStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
    cvCopy(m_pImageOrg, m_pImage);

    // 第一次画点
	if (GT2_Polyline == m_grapType || GT2_Line == m_grapType)
	{
	   if (!m_bStart)
	   {
		   m_ptPre = point;
		   m_tmpPolyLine.clear();
		   m_tmpPolyLine.push_back(point);
		   m_bStart = true;
	   }
	   else
	   {
		   m_ptPre = point;
		   m_tmpPolyLine.push_back(point);

           if (GT2_Line == m_grapType)
           {
		       m_pLineArray->push_back(m_tmpPolyLine);
               m_tmpPolyLine.clear();
		       m_bStart = false;
           } 
	   }
	} 
	else //矩形
	{
	   if (!m_bStart)
	   {
		   m_ptPre = point;
		   m_tmpRect.left = point.x;
		   m_tmpRect.top = point.y;
		   m_bStart = true;
	   }
	   else
	   {
		   if (m_pRectArray)
		   {
			   m_pRectArray->push_back(CRect(m_ptPre.x, m_ptPre.y, point.x, point.y));
		   }
           m_bStart = false;
	   }
	}

    DrawPolylineArray(m_pImage, m_pLineArray, &getColor(GT2_Line));

    DrawPolylineArray(m_pImage, m_pPolyLineArray, &getColor(GT2_Polyline));
    DrawPolyline(m_pImage, &m_tmpPolyLine, &getColor(GT2_Polyline));

    DrawRect(m_pImage, m_pRectArray, &getColor(GT2_Rectangle));


    m_OrgPoint = point;
    cvCopy(m_pImage, m_pImageDraft);
    DramImage(m_pImage);
}

void CMaskStatic::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bStart)
    {
        return;
    }

    if (GT2_Polyline == m_grapType)
    {
        cvLine(m_pImageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
            getColor(GT2_Polyline), LINE_WIDTH, CV_AA, 0 );
    } 
    else if (GT2_Line == m_grapType)
    {
        cvLine(m_pImageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
            getColor(GT2_Line), LINE_WIDTH, CV_AA, 0 );
    }
    else //矩形
    {
        cvRectangle(m_pImageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
            getColor(GT2_Rectangle), LINE_WIDTH, CV_AA, 0 );
    }

    DramImage(m_pImageDraft);
    cvCopy(m_pImage, m_pImageDraft, NULL);
}

//取消一个点
void CMaskStatic::OnRButtonDown(UINT nFlags, CPoint point)
{
    //完成折线的绘画
    if (m_bStart && GT2_Polyline == m_grapType)
    {
        if (m_tmpPolyLine.size() > 1 && m_pPolyLineArray)
        {
            m_pPolyLineArray->push_back(m_tmpPolyLine);
        }
    }

    //回退
    if (!m_bStart)
    {
        if (GT2_Polyline == m_grapType)
        {
        } 
        else if (GT2_Line == m_grapType)
        {
        }
        else if(GT2_Rectangle == m_grapType)
        {
        }
    }

    m_bStart = false;

    DrawItem(NULL);
}


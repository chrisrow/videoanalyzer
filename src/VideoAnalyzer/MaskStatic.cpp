#include "StdAfx.h"
#include "MaskStatic.h"

const int LINE_WIDTH = 2;

CMaskStatic::CMaskStatic(void)
    : m_image(NULL)
    , m_imageDraft(NULL)
    , m_imageOrg(NULL)
    , m_pLayer(NULL)
    , m_pLayerDraft(NULL)
    , m_pImage(NULL)
    , m_pImageOrg(NULL)
    , m_bStart(false)
    , m_grapType(GT2_Polyline)
    , m_pPolyLineArray(NULL)
    , m_pRectArray(NULL)
{
  m_OrgPoint.x = 0 ;
  m_OrgPoint.y = 0 ;

  m_EndPoint.x = 0 ;
  m_EndPoint.y = 0 ;
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

    DrawPolyline(m_pLayer, m_pPolyLineArray, &m_polyLineColor);
    DrawLine(m_pLayer, m_pLine, &m_lineColor);
    DrawRect(m_pLayer, m_pRectArray, &m_rectColor);

    SetWindowPos(NULL, -1, -1, m_image->width, m_image->height, SWP_NOMOVE);
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

void CMaskStatic::DrawPolyline(IplImage* img, PolyLineArray* array, CvScalar* color)
{
    if (array)
    {
        for (unsigned i = 0; i < array->size(); i++)
        {
            PolyLine& line = (*array)[i];
            if (line.size() < 2)
            {
                continue;
            }

            for (unsigned j = 0; j < line.size() - 1; j++)
            {
                cvLine(img, cvPoint(line[j].x, line[j].y), cvPoint(line[j+1].x, line[j+1].y), 
                    *color, LINE_WIDTH, CV_AA, 0 );
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


// TODO: //////////////////////////////////////////////////////////////////
// TODO: //////////////////////////////////////////////////////////////////
// TODO: //////////////////////////////////////////////////////////////////
// TODO: //////////////////////////////////////////////////////////////////
// TODO: //////////////////////////////////////////////////////////////////

bool CMaskStatic::Refresh()
{
//     m_pPolyLineArray = NULL;
//     m_pRectArray = NULL;

    cvCopy(m_imageOrg, m_imageDraft, NULL);
    cvCopy(m_imageOrg, m_image, NULL);

    DramImage(m_imageOrg);

    return true;
}

void CMaskStatic::Reset()
{
    if (m_image != NULL)
    {
        cvReleaseImage(&m_image);
        m_image = NULL;
    }

    if (m_imageDraft != NULL)
    {
        cvReleaseImage(&m_imageDraft);
        m_imageDraft = NULL;
    }

    if (m_imageOrg != NULL)
    {
        cvReleaseImage(&m_imageOrg);
        m_imageOrg = NULL;
    }

    if (m_pLayer != NULL)
    {
        cvReleaseImage(&m_imageOrg);
        m_pLayer = NULL;
    }

    if (m_pLayerDraft != NULL)
    {
        cvReleaseImage(&m_imageOrg);
        m_pLayerDraft = NULL;
    }

    m_bStart = false;
    m_grapType = GT2_Polyline;
    m_pPolyLineArray = NULL;
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
    if (m_imageDraft != NULL)
    {
        DramImage(m_imageDraft);
    }
}

void CMaskStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
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
		   cvLine(m_image, cvPoint(m_ptPre.x, m_ptPre.y), cvPoint(point.x, point.y), 
			   m_polyLineColor, LINE_WIDTH, CV_AA, 0 );
		   m_ptPre = point;
		   m_tmpPolyLine.push_back(point);
// 		   cvCopy(m_image, m_imageDraft, NULL);
           cvAddWeighted(m_image, 0.5, m_imageDraft, 0.5, 0, m_imageDraft);

           if (GT2_Line == m_grapType)
           {
		       m_pPolyLineArray->push_back(m_tmpPolyLine);
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
		   cvRectangle(m_image, cvPoint(m_ptPre.x, m_ptPre.y), cvPoint(point.x, point.y), 
			   m_rectColor, LINE_WIDTH, CV_AA, 0 );
// 		   m_ptPre = point;
		   if (m_pRectArray)
		   {
			   m_pRectArray->push_back(CRect(m_ptPre.x, m_ptPre.y, point.x, point.y));
		   }
// 		   cvCopy(m_image, m_imageDraft, NULL);
           cvAddWeighted(m_image, 0.5, m_imageDraft, 0.5, 0, m_imageDraft);
		   m_bStart = false;
	   }
	}

    m_OrgPoint = point;
//     m_bStart = true;
    DramImage(m_imageDraft);

    //CStatic::OnLButtonDown(nFlags, point);
}

void CMaskStatic::OnMouseMove(UINT nFlags, CPoint point)
{
    if (GT2_Polyline == m_grapType || GT2_Line == m_grapType)
    {
        if (m_bStart)
        {
            cvLine(m_imageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
                m_polyLineColor, LINE_WIDTH, CV_AA, 0 );
        }
    } 
    else //矩形
    {
        if (m_bStart)
        {
            cvRectangle(m_imageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
                m_rectColor, LINE_WIDTH, CV_AA, 0 );
       }
    }

    DramImage(m_imageDraft);

    //CStatic::OnMouseMove(nFlags, point);
    if (m_bStart)
    {
//       cvCopy(m_image, m_imageDraft, NULL);
      cvAddWeighted(m_image, 0.5, m_imageDraft, 0.5, 0, m_imageDraft);
    }
    
}

//取消一个点
void CMaskStatic::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (GT2_Polyline == m_grapType || GT2_Line == m_grapType)
    {
      if (m_bStart)
      {
          if (m_tmpPolyLine.size() > 1 && m_pPolyLineArray)
          {
              m_pPolyLineArray->push_back(m_tmpPolyLine);
          }
//           cvCopy(m_image, m_imageDraft, NULL);
          cvAddWeighted(m_image, 0.5, m_imageDraft, 0.5, 0, m_imageDraft);

          m_bStart = false;
      }
    }
    else //矩形
    {
      if (m_bStart)
      {
//           cvCopy(m_image, m_imageDraft, NULL);
          cvAddWeighted(m_image, 0.5, m_imageDraft, 0.5, 0, m_imageDraft);

          m_bStart = false;
      }
    }

    DramImage(m_imageDraft);

    //CStatic::OnRButtonDown(nFlags, point);
}


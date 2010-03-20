#include "StdAfx.h"
#include "GraphicsStatic.h"

const int LINE_WIDTH = 2;

CGraphicsStatic::CGraphicsStatic(void)
    :m_image(NULL)
    , m_imageDraft(NULL)
    , m_imageOrg(NULL)
    , m_bStart(false)
    , m_grapType(GT_Polyline)
    , m_pPolyLineArray(NULL)
    , m_pRectArray(NULL)
{
  m_OrgPoint.x = 0 ;
  m_OrgPoint.y = 0 ;

  m_EndPoint.x = 0 ;
  m_EndPoint.y = 0 ;
}

CGraphicsStatic::~CGraphicsStatic(void)
{
    Reset();
}

BEGIN_MESSAGE_MAP(CGraphicsStatic, CStatic)
ON_WM_LBUTTONDOWN()
ON_WM_MOUSEMOVE()
ON_WM_RBUTTONDOWN()
//ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


bool CGraphicsStatic::ShowImage(const IplImage* pFrame)
{
    if (m_image && pFrame
        && (m_image->width != pFrame->width || m_image->height != pFrame->height))
    {
        this->Reset();
    }

	if (pFrame)
	{
        if (m_image)
        {
            cvCopy(pFrame, m_image);
        }
        else
        {
            m_image = cvCloneImage(pFrame);
        }
	}
	else
	{
        if (m_image)
        {
            cvZero(m_image);
        }
        else
        {
            m_image = cvCreateImage( cvSize(352, 288), 8, 3);
            cvZero(m_image);
        }
	}

    if (m_pPolyLineArray)
    {
        for (unsigned i = 0; i < m_pPolyLineArray->size(); i++)
        {
            PolyLine& line = (*m_pPolyLineArray)[i];
            if (line.size() < 2)
            {
                continue;
            }

            for (unsigned j = 0; j < line.size() - 1; j++)
            {
                cvLine(m_image, cvPoint(line[j].x, line[j].y), cvPoint(line[j+1].x, line[j+1].y), 
                    m_color, LINE_WIDTH, CV_AA, 0 );
            }
        }
    }

    if (m_pRectArray)
    {
        for (unsigned i = 0; i < m_pRectArray->size(); i++)
        {
            CRect& rect = (*m_pRectArray)[i];
            cvRectangle(m_image, cvPoint(rect.left, rect.top), cvPoint(rect.right, rect.bottom), 
                m_color, LINE_WIDTH, CV_AA, 0 );
        }
    }

    if (m_imageDraft)
    {
        cvCopy(m_image, m_imageDraft);
    }
    else
    {
        m_imageDraft = cvCloneImage(m_image);
    }

    if (m_imageOrg)
    {
        cvCopy(m_image, m_imageOrg);
    }
    else
    {
        m_imageOrg = cvCloneImage(m_image);
    }

    SetWindowPos(NULL, -1, -1, m_image->width, m_image->height, SWP_NOMOVE);

	this->DrawItem(NULL);

    return true;
}

bool CGraphicsStatic::Refresh()
{
//     m_pPolyLineArray = NULL;
//     m_pRectArray = NULL;

    cvCopy(m_imageOrg, m_imageDraft, NULL);
    cvCopy(m_imageOrg, m_image, NULL);

    DramImage(m_imageOrg);

    return true;
}

void CGraphicsStatic::Reset()
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

    m_bStart = false;
    m_grapType = GT_Polyline;
    m_pPolyLineArray = NULL;
    m_pRectArray = NULL;
}

bool CGraphicsStatic::DramImage(IplImage *img)
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

void CGraphicsStatic::DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/)
{
    if (m_imageDraft != NULL)
    {
        DramImage(m_imageDraft);
    }
}

void CGraphicsStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
    // 第一次画点
	if (GT_Polyline == m_grapType)
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
			   m_color, LINE_WIDTH, CV_AA, 0 );
		   m_ptPre = point;
		   m_tmpPolyLine.push_back(point);
		   cvCopy(m_image, m_imageDraft, NULL);
		   //删除一下两行就可以支持折线
		   m_pPolyLineArray->push_back(m_tmpPolyLine);
		   m_bStart = false;
	   }
	} 
	else
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
			   m_color, LINE_WIDTH, CV_AA, 0 );
// 		   m_ptPre = point;
		   if (m_pRectArray)
		   {
			   m_pRectArray->push_back(CRect(m_ptPre.x, m_ptPre.y, point.x, point.y));
		   }
		   cvCopy(m_image, m_imageDraft, NULL);
		   m_bStart = false;
	   }
	}

    m_OrgPoint = point;
//     m_bStart = true;
    DramImage(m_imageDraft);

    //CStatic::OnLButtonDown(nFlags, point);
}

void CGraphicsStatic::OnMouseMove(UINT nFlags, CPoint point)
{
    if (GT_Polyline == m_grapType)
    {
        if (m_bStart)
        {
            cvLine(m_imageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
                m_color, LINE_WIDTH, CV_AA, 0 );
        }
    } 
    else
    {
        if (m_bStart)
        {
            cvRectangle(m_imageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
                m_color, LINE_WIDTH, CV_AA, 0 );
       }
    }

    DramImage(m_imageDraft);

    //CStatic::OnMouseMove(nFlags, point);
    if (m_bStart)
    {
      cvCopy(m_image, m_imageDraft, NULL);
    }
    
}

//取消一个点
void CGraphicsStatic::OnRButtonDown(UINT nFlags, CPoint point)
{
  if (GT_Polyline == m_grapType)
  {
      if (m_bStart)
      {
          if (m_tmpPolyLine.size() > 1 && m_pPolyLineArray)
          {
              m_pPolyLineArray->push_back(m_tmpPolyLine);
          }
          cvCopy(m_image, m_imageDraft, NULL);

          m_bStart = false;
      }
  }
  else
  {
      if (m_bStart)
      {
          cvCopy(m_image, m_imageDraft, NULL);

          m_bStart = false;
      }
  }

  DramImage(m_imageDraft);

  //CStatic::OnRButtonDown(nFlags, point);
}


void CGraphicsStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
//      if (GT_Polyline == m_grapType)
//      {
//          if (m_bStart)
//          {
//            //  cvCopy(m_image, m_imageDraft, NULL);
//            m_EndPoint.x = point.x ;
//            m_EndPoint.y = point.y ;
//            cvLine(m_imageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
//              m_color, 1, CV_AA, 0 );
//   
//              m_bStart = false;
//          }
//      }
//      else
//      {
//          if (m_bStart)
//          {
//            m_EndPoint.x = point.x ;
//            m_EndPoint.y = point.y ;
//            cvRectangle(m_imageDraft, cvPoint(m_OrgPoint.x, m_OrgPoint.y), cvPoint(point.x, point.y), 
//              m_color, 1, CV_AA, 0 );
//             // cvCopy(m_image, m_imageDraft, NULL);
//   
//              m_bStart = false;
//          }
//      }
//   
//      DramImage(m_imageDraft);
//   CStatic::OnLButtonUp(nFlags, point);
}

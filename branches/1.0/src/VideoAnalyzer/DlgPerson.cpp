// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DlgPerson.h"
#include "Common/frames.h"


// CDlgPerson dialog

IMPLEMENT_DYNAMIC(CDlgPerson, CDialog)

CDlgPerson::CDlgPerson(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPerson::IDD, pParent)
{

}

CDlgPerson::~CDlgPerson()
{
  desrory( );
}

void CDlgPerson::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPerson, CDialog)
  ON_BN_CLICKED(IDC_BTN_REFRESH, &CDlgPerson::OnBnClickedBtnRefresh)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_BN_CLICKED(IDC_BTN_FILL, &CDlgPerson::OnBnClickedBtnFill)
  ON_BN_CLICKED(IDC_BTN_CAR, &CDlgPerson::OnBnClickedBtnCar)
  ON_BN_CLICKED(IDC_BTN_UNDO, &CDlgPerson::OnBnClickedBtnUndo)
  ON_BN_CLICKED(IDC_BTN_OUTPUT, &CDlgPerson::OnBnClickedBtnOutput)
  ON_BN_CLICKED(IDC_BTN_SAVE, &CDlgPerson::OnBnClickedBtnSave)
  ON_WM_PAINT()
  ON_BN_CLICKED(IDC_BTN_INVERSE, &CDlgPerson::OnBnClickedBtnInverse)
END_MESSAGE_MAP()


// CDlgPerson message handlers

BOOL CDlgPerson::OnInitDialog()
{
  CDialog::OnInitDialog();


  CString m_FilePath = line_TestGUI->GetFilePath() ;
  line_TestGUI->flag4 = true;
  CString FileName = _T("\\set_TestGUIDLG.set");
  m_FilePath += FileName;
  line_TestGUI->GetParameterToFile(m_FilePath);


  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPerson::OnPaint()
{
  CPaintDC dc(this); // device context for painting
  // TODO: Add your message handler code here
  // Do not call CDialog::OnPaint() for painting messages

  CClientDC DC(this);
  line_TestGUI->DrawPicture2(DC,0,0,datafromline,line_TestGUI->Pic_sizex,line_TestGUI->Pic_sizey);
  if(line_TestGUI->index>0)
  {
    line_TestGUI->Draw(DC);
  }
}

void CDlgPerson::OnLButtonDown(UINT nFlags, CPoint point)
{
  if ( line_TestGUI->flag4 )
  {
    if(point.x > line_TestGUI->pos_pic.x+line_TestGUI->Pic_sizex
        || point.y > line_TestGUI->pos_pic.y+line_TestGUI->Pic_sizey
        || point.x < line_TestGUI->pos_pic.x
        || point.y < line_TestGUI->pos_pic.y)
    {
      return;
    }
    line_TestGUI->GetCenterPoint_Pic();
    line_TestGUI->GetMinPoint(point);
    line_TestGUI->Pic_Sign[line_TestGUI->index]=line_TestGUI->pt_Pic;
    line_TestGUI->index++;
    OnPaint();
  }

  CDialog::OnLButtonDown(nFlags, point);
  OnPaint();
}

void CDlgPerson::OnRButtonDown(UINT nFlags, CPoint point)
{
  if(point.x>line_TestGUI->pos_pic.x+line_TestGUI->Pic_sizex
      ||point.y>line_TestGUI->pos_pic.y+line_TestGUI->Pic_sizey
      ||point.x<line_TestGUI->pos_pic.x
      ||point.y<line_TestGUI->pos_pic.y)
  {
    return;
  }
  line_TestGUI->DeletPoint(point);	

  CDialog::OnRButtonDown(nFlags, point);
  OnPaint();
}

void CDlgPerson::OnBnClickedBtnRefresh()
{
  line_TestGUI->init_line(line_TestGUI->Pic_sizex,line_TestGUI->Pic_sizey);
  line_TestGUI->flag4 = true;
  OnPaint();
  Invalidate();
}

void CDlgPerson::OnBnClickedBtnFill()
{
  CClientDC dc(this); 
  line_TestGUI->flag2=true;
  line_TestGUI->Draw(dc); 
  line_TestGUI->OutPut(dc);
}

void CDlgPerson::OnBnClickedBtnCar()
{
  if(0 == line_TestGUI->index)
  {
    return;
  }
  line_TestGUI->flag3=true;
  line_TestGUI->GetCutIndex();
}

void CDlgPerson::OnBnClickedBtnUndo()
{
  line_TestGUI->undo();
  OnPaint();
}

void CDlgPerson::OnBnClickedBtnOutput()
{
  CClientDC dc(this);
  line_TestGUI->ShowReginInfo(dc,360,0);
}

void CDlgPerson::OnBnClickedBtnInverse()
{
  line_TestGUI->binverse = !line_TestGUI->binverse;
}

void CDlgPerson::OnBnClickedBtnSave()
{
  CString m_FilePath = line_TestGUI->GetFilePath();
  CString FileName = _T("\\set_TestGUIDLG.set");
  m_FilePath += FileName;
  CClientDC dc(this);
  line_TestGUI->OutPut(dc);
  line_TestGUI->SaveParameterToFile(m_FilePath);
}

void CDlgPerson::desrory( )//==
{
  if (datafromline)
  {
    delete [] datafromline;
    datafromline = NULL;
  }
}

void CDlgPerson::reinit(int imWidthin, int imHeightin )
{
  line_TestGUI = new Line(imWidthin,imHeightin);
  datafromline = new unsigned char[imWidthin*imHeightin*3];

  m_nWidth = imWidthin;
  m_nHeight = imHeightin;
}




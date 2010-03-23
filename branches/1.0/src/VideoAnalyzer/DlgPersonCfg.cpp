// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DlgPersonCfg.h"
#include "Common/frames.h"


// CDlgPersonCfg dialog

IMPLEMENT_DYNAMIC(CDlgPersonCfg, CDialog)

CDlgPersonCfg::CDlgPersonCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPersonCfg::IDD, pParent)
{

}

CDlgPersonCfg::~CDlgPersonCfg()
{
  desrory( );
}

void CDlgPersonCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPersonCfg, CDialog)
  ON_BN_CLICKED(IDC_BTN_REFRESH, &CDlgPersonCfg::OnBnClickedBtnRefresh)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_BN_CLICKED(IDC_BTN_FILL, &CDlgPersonCfg::OnBnClickedBtnFill)
  ON_BN_CLICKED(IDC_BTN_CAR, &CDlgPersonCfg::OnBnClickedBtnCar)
  ON_BN_CLICKED(IDC_BTN_UNDO, &CDlgPersonCfg::OnBnClickedBtnUndo)
  ON_BN_CLICKED(IDC_BTN_OUTPUT, &CDlgPersonCfg::OnBnClickedBtnOutput)
  ON_BN_CLICKED(IDC_BTN_SAVE, &CDlgPersonCfg::OnBnClickedBtnSave)
  ON_WM_PAINT()
  ON_BN_CLICKED(IDC_BTN_INVERSE, &CDlgPersonCfg::OnBnClickedBtnInverse)
END_MESSAGE_MAP()


// CDlgPersonCfg message handlers

BOOL CDlgPersonCfg::OnInitDialog()
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

void CDlgPersonCfg::OnPaint()
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

void CDlgPersonCfg::OnLButtonDown(UINT nFlags, CPoint point)
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

void CDlgPersonCfg::OnRButtonDown(UINT nFlags, CPoint point)
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

void CDlgPersonCfg::OnBnClickedBtnRefresh()
{
  line_TestGUI->init_line(line_TestGUI->Pic_sizex,line_TestGUI->Pic_sizey);
  line_TestGUI->flag4 = true;
  OnPaint();
  Invalidate();
}

void CDlgPersonCfg::OnBnClickedBtnFill()
{
  CClientDC dc(this); 
  line_TestGUI->flag2=true;
  line_TestGUI->Draw(dc); 
  line_TestGUI->OutPut(dc);
}

void CDlgPersonCfg::OnBnClickedBtnCar()
{
  if(0 == line_TestGUI->index)
  {
    return;
  }
  line_TestGUI->flag3=true;
  line_TestGUI->GetCutIndex();
}

void CDlgPersonCfg::OnBnClickedBtnUndo()
{
  line_TestGUI->undo();
  OnPaint();
}

void CDlgPersonCfg::OnBnClickedBtnOutput()
{
  CClientDC dc(this);
  line_TestGUI->ShowReginInfo(dc,360,0);
}

void CDlgPersonCfg::OnBnClickedBtnInverse()
{
  line_TestGUI->binverse = !line_TestGUI->binverse;
}

void CDlgPersonCfg::OnBnClickedBtnSave()
{
  CString m_FilePath = line_TestGUI->GetFilePath();
  CString FileName = _T("\\set_TestGUIDLG.set");
  m_FilePath += FileName;
  CClientDC dc(this);
  line_TestGUI->OutPut(dc);
  line_TestGUI->SaveParameterToFile(m_FilePath);
}

void CDlgPersonCfg::desrory( )//==
{
  if (datafromline)
  {
    delete [] datafromline;
    datafromline = NULL;
  }
}

void CDlgPersonCfg::reinit(int imWidthin, int imHeightin )
{
  line_TestGUI = new Line(imWidthin,imHeightin);
  datafromline = new unsigned char[imWidthin*imHeightin*3];

  m_nWidth = imWidthin;
  m_nHeight = imHeightin;
}




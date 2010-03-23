// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DlgPersonCfg.h"
#include "Common/frames.h"


// CSettingDlg dialog

IMPLEMENT_DYNAMIC(CSettingDlg, CDialog)

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDlg::IDD, pParent)
{

}

CSettingDlg::~CSettingDlg()
{
  desrory( );
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialog)
  ON_BN_CLICKED(IDC_BTN_REFRESH, &CSettingDlg::OnBnClickedBtnRefresh)
  ON_WM_LBUTTONDOWN()
  ON_WM_RBUTTONDOWN()
  ON_BN_CLICKED(IDC_BTN_FILL, &CSettingDlg::OnBnClickedBtnFill)
  ON_BN_CLICKED(IDC_BTN_CAR, &CSettingDlg::OnBnClickedBtnCar)
  ON_BN_CLICKED(IDC_BTN_UNDO, &CSettingDlg::OnBnClickedBtnUndo)
  ON_BN_CLICKED(IDC_BTN_OUTPUT, &CSettingDlg::OnBnClickedBtnOutput)
  ON_BN_CLICKED(IDC_BTN_SAVE, &CSettingDlg::OnBnClickedBtnSave)
  ON_WM_PAINT()
  ON_BN_CLICKED(IDC_BTN_INVERSE, &CSettingDlg::OnBnClickedBtnInverse)
END_MESSAGE_MAP()


// CSettingDlg message handlers

BOOL CSettingDlg::OnInitDialog()
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

void CSettingDlg::OnPaint()
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

void CSettingDlg::OnLButtonDown(UINT nFlags, CPoint point)
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

void CSettingDlg::OnRButtonDown(UINT nFlags, CPoint point)
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
void CSettingDlg::OnBnClickedBtnRefresh()
{
  line_TestGUI->init_line(line_TestGUI->Pic_sizex,line_TestGUI->Pic_sizey);
  line_TestGUI->flag4 = true;
  OnPaint();
  Invalidate();
}

void CSettingDlg::OnBnClickedBtnFill()
{
  CClientDC dc(this); 
  line_TestGUI->flag2=true;
  line_TestGUI->Draw(dc); 
  line_TestGUI->OutPut(dc);
}

void CSettingDlg::OnBnClickedBtnCar()
{
  if(0 == line_TestGUI->index)
  {
    return;
  }
  line_TestGUI->flag3=true;
  line_TestGUI->GetCutIndex();
}

void CSettingDlg::OnBnClickedBtnUndo()
{
  line_TestGUI->undo();
  OnPaint();
}

void CSettingDlg::OnBnClickedBtnOutput()
{
  CClientDC dc(this);
  line_TestGUI->ShowReginInfo(dc,360,0);
}

void CSettingDlg::OnBnClickedBtnInverse()
{
  line_TestGUI->binverse = !line_TestGUI->binverse;
}

void CSettingDlg::OnBnClickedBtnSave()
{
  CString m_FilePath = line_TestGUI->GetFilePath();
  CString FileName = _T("\\set_TestGUIDLG.set");
  m_FilePath += FileName;
  CClientDC dc(this);
  line_TestGUI->OutPut(dc);
  line_TestGUI->SaveParameterToFile(m_FilePath);
}

void CSettingDlg::desrory( )//==
{
  if (datafromline)
  {
    delete [] datafromline;
    datafromline = NULL;
  }
}

void CSettingDlg::reinit(int imWidthin, int imHeightin )
{
  line_TestGUI = new Line(imWidthin,imHeightin);
  datafromline = new unsigned char[imWidthin*imHeightin*3];

  m_nWidth = imWidthin;
  m_nHeight = imHeightin;
}




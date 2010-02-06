// E:\code\VideoAnalyzer\src\VideoAnalyzer\DlgSetting.cpp : implementation file
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "VideoAnalyzer\DlgSetting.h"


// CDlgSetting dialog

IMPLEMENT_DYNAMIC(CDlgSetting, CDialog)

CDlgSetting::CDlgSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetting::IDD, pParent)
{
	m_pImage = NULL;
}

CDlgSetting::~CDlgSetting()
{
}

void CDlgSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMAGE, m_ctrlImage);
}


BEGIN_MESSAGE_MAP(CDlgSetting, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CDlgSetting::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CDlgSetting::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CDlgSetting message handlers

BOOL CDlgSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ctrlImage.ShowImage(m_pImage);
	m_ctrlImage.SetGraphicsColor(RGB(255, 0, 0));

	m_ctrlImage.setPolyLineArray(m_testPolyLineArray);
	m_ctrlImage.setRectArray(m_testRectArray);

// 	m_ctrlImage.SetGraphicsType(GT_Rectangle);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetting::setImage(const IplImage *pImage)
{
	m_pImage = pImage;
}
void CDlgSetting::OnBnClickedButtonClear()
{
	m_ctrlImage.Refresh();
}

void CDlgSetting::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	m_ctrlImage.Refresh() ;
	int a = m_testPolyLineArray.size();
	CPoint b = m_testPolyLineArray[0][0];
	CPoint v = m_testPolyLineArray[0][1];
	m_testPolyLineArray.clear();
	int t = a ;
}

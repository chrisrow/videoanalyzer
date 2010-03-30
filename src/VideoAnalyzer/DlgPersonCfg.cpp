// DlgPersonCfg.cpp : 实现文件
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "DlgPersonCfg.h"


// CDlgPersonCfg 对话框

IMPLEMENT_DYNAMIC(CDlgPersonCfg, CDialog)

CDlgPersonCfg::CDlgPersonCfg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPersonCfg::IDD, pParent)
{

}

CDlgPersonCfg::~CDlgPersonCfg()
{
}

void CDlgPersonCfg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_IMAGE, m_ctrlImage);
}


BEGIN_MESSAGE_MAP(CDlgPersonCfg, CDialog)
END_MESSAGE_MAP()

void CDlgPersonCfg::setImage(const IplImage *pImage)
{
    if (pImage)
    {
        m_pImage = cvCloneImage(pImage);
    }
    else
    {
        m_pImage = NULL;
    }
}

// CDlgPersonCfg 消息处理程序

BOOL CDlgPersonCfg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_ctrlImage.ShowImage(m_pImage);

    m_ctrlImage.setPolyLineArray(m_mask);
    m_ctrlImage.setLine(m_warningLine);
    m_ctrlImage.setRectArray(m_rect);

    m_ctrlImage.SetGraphicsType(GT2_Polyline, RGB(0, 0, 255));
//     m_ctrlImage.SetGraphicsType(GT2_Line, RGB(255, 0, 0));
//     m_ctrlImage.SetGraphicsType(GT2_Rectangle, RGB(255, 0, 255));


    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}

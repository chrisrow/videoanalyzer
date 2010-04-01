// DlgPersonCfg.cpp : 实现文件
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "DlgPersonCfg.h"
#include "Algorithm/PersonStruct.h"

extern TPersonDetect g_personParam;

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
    DDX_Control(pDX, IDC_BUTTON_WARNING_LINE, m_btnWarnLine);
    DDX_Control(pDX, IDC_BUTTON_POLY, m_btnMask);
    DDX_Control(pDX, IDC_BUTTON_FILL, m_btnFill);
}


BEGIN_MESSAGE_MAP(CDlgPersonCfg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_WARNING_LINE, &CDlgPersonCfg::OnBnClickedButtonWarningLine)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_WARNING_LINE, &CDlgPersonCfg::OnBnClickedButtonClearWarningLine)
    ON_BN_CLICKED(IDC_BUTTON_POLY, &CDlgPersonCfg::OnBnClickedButtonPoly)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_POLY, &CDlgPersonCfg::OnBnClickedButtonClearPoly)
    ON_BN_CLICKED(IDC_BUTTON_FILL, &CDlgPersonCfg::OnBnClickedButtonFill)
    ON_BN_CLICKED(IDC_BUTTON_CLEARALL, &CDlgPersonCfg::OnBnClickedButtonClearall)
    ON_BN_CLICKED(IDOK, &CDlgPersonCfg::OnBnClickedOk)
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


    m_ctrlImage.SetGraphicsType(GT2_Polyline, RGB(0, 0, 255));
    m_ctrlImage.setPolyLineArray(m_mask);
    m_btnMask.SetState(FALSE);

    m_ctrlImage.SetGraphicsType(GT2_Line, RGB(255, 0, 0));
    m_ctrlImage.setLine(m_warningLine);
    m_btnWarnLine.SetState(TRUE);

    m_warningLine.clear();
    if (g_personParam.warnLine.size() > 0)
    {
        m_warningLine.push_back(g_personParam.warnLine);
    }

    m_mask.clear();
    m_mask = g_personParam.maskLine;


    m_ctrlImage.ShowImage(m_pImage);

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}

void CDlgPersonCfg::OnBnClickedButtonWarningLine()
{
    m_ctrlImage.setLine(m_warningLine);
    m_ctrlImage.SetGraphicsType(GT2_Line, RGB(255, 0, 0));

    m_btnWarnLine.SetState(TRUE);
    m_btnMask.SetState(FALSE);
}

void CDlgPersonCfg::OnBnClickedButtonClearWarningLine()
{
    if (m_warningLine.size() > 0)
    {
        m_warningLine.pop_back();
        m_ctrlImage.ShowImage(m_pImage);
    }
    OnBnClickedButtonWarningLine();
}

void CDlgPersonCfg::OnBnClickedButtonPoly()
{
    m_ctrlImage.setPolyLineArray(m_mask);
    m_ctrlImage.SetGraphicsType(GT2_Polyline, RGB(0, 0, 255));

    m_btnWarnLine.SetState(FALSE);
    m_btnMask.SetState(TRUE);
}

void CDlgPersonCfg::OnBnClickedButtonClearPoly()
{
    if (m_mask.size() > 0)
    {
        m_mask.pop_back();
        m_ctrlImage.ShowImage(m_pImage);
    }
    OnBnClickedButtonPoly();
}

void CDlgPersonCfg::OnBnClickedButtonFill()
{
    static bool flag = false;
    if (!flag)
    {
        m_ctrlImage.setFill(true);
        m_btnFill.SetState(TRUE);
    }
    else
    {
        m_ctrlImage.setFill(false);
        m_btnFill.SetState(FALSE);
    }
    m_ctrlImage.ShowImage(m_pImage);
    flag = !flag;
}

void CDlgPersonCfg::OnBnClickedButtonClearall()
{
    m_warningLine.clear();
    m_mask.clear();
    m_ctrlImage.ShowImage(m_pImage);

    m_btnWarnLine.SetState(FALSE);
    m_btnMask.SetState(FALSE);
}

void CDlgPersonCfg::OnBnClickedOk()
{

    g_personParam.reset();

    //预警线
    if (m_warningLine.size() == 2)
    {
        g_personParam.warnLine.push_back(CPoint(m_warningLine[0][0].x, m_warningLine[0][0].y));
        g_personParam.warnLine.push_back(CPoint(m_warningLine[0][1].x, m_warningLine[0][1].y));
    }

    //由折线生成遮罩图像
    if (m_mask.size() > 0)
    {
        g_personParam.maskLine = m_mask;
        CRect rect;
        m_ctrlImage.GetWindowRect(&rect);
        g_personParam.mask = cvCreateImage(cvSize(rect.Width(), rect.Height()), 8, 3);

        ::makeMask(m_mask, g_personParam.mask);
    }

    OnOK();
}

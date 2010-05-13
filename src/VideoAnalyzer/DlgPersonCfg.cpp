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
    DDX_Control(pDX, IDC_BUTTON_FILL, m_btnFill);
}


BEGIN_MESSAGE_MAP(CDlgPersonCfg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_WARNING_LINE, &CDlgPersonCfg::OnBnClickedButtonClearWarningLine)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_POLY, &CDlgPersonCfg::OnBnClickedButtonClearPoly)
    ON_BN_CLICKED(IDC_BUTTON_FILL, &CDlgPersonCfg::OnBnClickedButtonFill)
    ON_BN_CLICKED(IDC_BUTTON_CLEARALL, &CDlgPersonCfg::OnBnClickedButtonClearall)
    ON_BN_CLICKED(IDOK, &CDlgPersonCfg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR_NIGTH, &CDlgPersonCfg::OnBnClickedButtonClearNigth)
    ON_BN_CLICKED(IDC_RADIO_WARNING_LINE, &CDlgPersonCfg::OnBnClickedRadioWarningLine)
    ON_BN_CLICKED(IDC_RADIO_MASK, &CDlgPersonCfg::OnBnClickedRadioMask)
    ON_BN_CLICKED(IDC_RADIO_NIGTH, &CDlgPersonCfg::OnBnClickedRadioNigth)
END_MESSAGE_MAP()

// CDlgPersonCfg 消息处理程序

BOOL CDlgPersonCfg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_ctrlImage.ShowImage(m_pImage);

    //遮罩
    m_ctrlImage.SetGraphicsType(GT2_Polyline, RGB(0, 0, 255));
    m_ctrlImage.setPolyLineArray(m_mask);
    m_mask.clear();
    m_mask = g_personParam.maskLine;
	
    //黑夜框
    m_ctrlImage.SetGraphicsType(GT2_Rectangle, RGB(0, 255, 0));
    m_ctrlImage.setRectArray(m_nigthRectArray);
    m_nigthRectArray.clear();
    m_nigthRectArray = g_personParam.nigthRectArray;

    //警戒线。这个要放在最后
    m_ctrlImage.SetGraphicsType(GT2_Line, RGB(255, 0, 0));
    m_ctrlImage.setLine(m_warningLine);
    ((CButton*)GetDlgItem(IDC_RADIO_WARNING_LINE))->SetCheck(1);

    m_warningLine.clear();
    if (g_personParam.warnLine.size() > 0)
    {
        m_warningLine.push_back(g_personParam.warnLine);
    }

    m_ctrlImage.ShowImage(m_pImage);

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}

void CDlgPersonCfg::OnBnClickedRadioWarningLine()
{
    m_ctrlImage.setLine(m_warningLine);
    m_ctrlImage.SetGraphicsType(GT2_Line, RGB(255, 0, 0));

    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgPersonCfg::OnBnClickedRadioMask()
{
    m_ctrlImage.setPolyLineArray(m_mask);
    m_ctrlImage.SetGraphicsType(GT2_Polyline, RGB(0, 0, 255));

    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgPersonCfg::OnBnClickedRadioNigth()
{
    m_ctrlImage.setRectArray(m_nigthRectArray);
    m_ctrlImage.SetGraphicsType(GT2_Rectangle, RGB(0, 255, 0));

    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgPersonCfg::OnBnClickedButtonClearWarningLine()
{
    if (m_warningLine.size() > 0)
    {
        m_warningLine.pop_back();
        m_ctrlImage.ShowImage(m_pImage);
    }
    OnBnClickedRadioWarningLine();
}

void CDlgPersonCfg::OnBnClickedButtonClearPoly()
{
    if (m_mask.size() > 0)
    {
        m_mask.pop_back();
        m_ctrlImage.ShowImage(m_pImage);
    }
    OnBnClickedRadioMask();
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
    flag = !flag;
    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgPersonCfg::OnBnClickedButtonClearNigth()
{
    if (m_nigthRectArray.size() > 0)
    {
        m_nigthRectArray.pop_back();
        m_ctrlImage.ShowImage(m_pImage);
    }
    OnBnClickedRadioNigth();
}

void CDlgPersonCfg::OnBnClickedButtonClearall()
{
    m_warningLine.clear();
    m_mask.clear();
    m_nigthRectArray.clear();

    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgPersonCfg::OnBnClickedOk()
{

    g_personParam.reset();

    //黑夜框
    g_personParam.nigthRectArray = m_nigthRectArray;

    //预警线
    if (m_warningLine.size() > 0 && m_warningLine[0].size() > 1)
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




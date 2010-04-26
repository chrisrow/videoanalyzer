#pragma once

#include "DlgCfg.h"
#include "MaskStatic.h"
#include "cv.h"
#include "highgui.h"
#include "afxwin.h"


// CDlgPersonCfg 对话框

class CDlgPersonCfg : public CDialog, public CDlgCfgBase
{
	DECLARE_DYNAMIC(CDlgPersonCfg)

public:
	CDlgPersonCfg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgPersonCfg();

// 对话框数据
	enum { IDD = IDD_PERSON_CFG };


public:
    virtual BOOL OnInitDialog();

private:
    CMaskStatic m_ctrlImage;

    PolyLineArray m_mask; //遮罩
    LineArray m_warningLine;   //预警线
//    LineArray m_referLine;//
    RectArray m_rect;

    CButton m_btnWarnLine;
    CButton m_btnReferLine;
    CButton m_btnMask;
    CButton m_btnFill;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonWarningLine();
    afx_msg void OnBnClickedButtonClearWarningLine();
    afx_msg void OnBnClickedButtonPoly();
    afx_msg void OnBnClickedButtonClearPoly();
    afx_msg void OnBnClickedButtonFill();
    afx_msg void OnBnClickedButtonClearall();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButtonReferLine();
    afx_msg void OnBnClickedButtonClearReferLine();
};

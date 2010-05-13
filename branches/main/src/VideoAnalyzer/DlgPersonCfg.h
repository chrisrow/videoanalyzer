#pragma once

#include "DlgCfg.h"
#include "MaskStatic.h"
#include "cv.h"
#include "highgui.h"
#include "afxwin.h"


// CDlgPersonCfg �Ի���

class CDlgPersonCfg : public CDialog, public CDlgCfgBase
{
	DECLARE_DYNAMIC(CDlgPersonCfg)

public:
	CDlgPersonCfg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgPersonCfg();

// �Ի�������
	enum { IDD = IDD_PERSON_CFG };


public:
    virtual BOOL OnInitDialog();

private:
    CMaskStatic m_ctrlImage;

    PolyLineArray m_mask; //����
    LineArray m_warningLine;   //Ԥ����
    RectArray m_nigthRectArray;

    CButton m_btnFill;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonClearWarningLine();

    afx_msg void OnBnClickedButtonClearPoly();
    afx_msg void OnBnClickedButtonFill();

    afx_msg void OnBnClickedButtonClearNigth();

    afx_msg void OnBnClickedButtonClearall();

    afx_msg void OnBnClickedOk();

    afx_msg void OnBnClickedRadioWarningLine();
    afx_msg void OnBnClickedRadioMask();
    afx_msg void OnBnClickedRadioNigth();
};

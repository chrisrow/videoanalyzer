#pragma once

#include "cv.h"
#include "highgui.h"
#include "GraphicsStatic.h"

// CDlgSetting dialog

class CDlgSetting : public CDialog
{
	DECLARE_DYNAMIC(CDlgSetting)

public:
	CDlgSetting(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSetting();

// Dialog Data
	enum { IDD = IDD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void setImage(const IplImage *pImage );
private:
	const IplImage *m_pImage;
	CGraphicsStatic m_ctrlImage;

	PolyLineArray m_testPolyLineArray;
	RectArray     m_testRectArray;

	int m_ComboChangeNum;

public:
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnCbnSelchangeComboControl();
	int m_edit_two_value;
	int m_edit_filter_value;
	int m_edit_imdilate_value;
	int m_edit_white_spot_max;
	int m_edit_white_min;
	int m_edit_night_thr;
	int m_edit_max_frame;
	afx_msg void OnBnClickedCheckLens();
};

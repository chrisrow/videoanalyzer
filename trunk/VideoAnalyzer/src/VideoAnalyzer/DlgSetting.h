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
	void setImage(const IplImage *pImage);

private:
	const IplImage *m_pImage;
	CGraphicsStatic m_ctrlImage;

	PolyLineArray m_testPolyLineArray;
	RectArray     m_testRectArray;

public:
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedButtonSave();
};

#pragma once

#include "cv.h"
#include "highgui.h"
#include "GraphicsStatic.h"

#include "ConfigFile.h"

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
	IplImage *m_pImage;
	CGraphicsStatic m_ctrlImage;

    enum {COMBOBOX_ITEM_NUM = 13};
	PolyLineArray m_polyLineArrays[COMBOBOX_ITEM_NUM];
// 	RectArray     m_testRectArray;

	int m_ComboChangeNum;

    void SaveComboboxData();
    void LoadComboboxData();

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
	afx_msg void OnBnClickedRadioNatural();
	afx_msg void OnBnClickedRadioTree();
	afx_msg void OnBnClickedRadioCurver();
	afx_msg void OnBnClickedRadioNatuarlCurver();
	afx_msg void OnBnClickedRadioNatuarlNatuarl();
	afx_msg void OnBnClickedRadioTreeCurver();
	afx_msg void OnBnClickedRadioDepth1();
	afx_msg void OnBnClickedRadioDepth2();
	afx_msg void OnBnClickedRadioDepth3();
	afx_msg void OnBnClickedRadioSensitive1();
	afx_msg void OnBnClickedRadioSensitive2();
	afx_msg void OnBnClickedRadioSensitive3();
	afx_msg void OnEnChangeEditTwoValue();
//	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedCheckPerson();
    afx_msg void OnBnClickedOk();
};

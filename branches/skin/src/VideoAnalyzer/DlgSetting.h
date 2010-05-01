#pragma once

#include "DlgCfg.h"
#include "GraphicsStatic.h"
#include "cv.h"
#include "highgui.h"

#include "afxwin.h"

// CDlgSetting dialog
enum
{
    COMBOBOX_ITEM_RECT_BLACK_BLOCK        = 0,  //矩形
    COMBOBOX_ITEM_LINE_BLACK_LEFT         = 1,
    COMBOBOX_ITEM_LINE_BLACK_RIGHT        = 2,
    COMBOBOX_ITEM_LINE_STRAIGHT_FIRST     = 3,
    COMBOBOX_ITEM_LINE_STRAIGHT_SECOND    = 4,
    COMBOBOX_ITEM_LINE_CURVER_RANGE       = 5,
    COMBOBOX_ITEM_PERSON_RANGE_0          = 6,
    COMBOBOX_ITEM_PERSON_RANGE_1          = 7,
    COMBOBOX_ITEM_PERSON_RANGE_2          = 8,
    COMBOBOX_ITEM_NIGHT_RANGE_0           = 9,   //矩形
    COMBOBOX_ITEM_NIGHT_RANGE_1           = 10,  //矩形
    COMBOBOX_ITEM_RECT_LITTLE_REGION      = 11,  //矩形
    COMBOBOX_ITEM_LITTLE_REGION_Y_LINE    = 12
};

class CDlgSetting : public CDialog, public CDlgCfgBase
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

private:
	CGraphicsStatic m_ctrlImage;

    enum {COMBOBOX_ITEM_NUM = 13};
	PolyLineArray m_polyLineArrays[COMBOBOX_ITEM_NUM];
 	RectArray     m_rectArrays[COMBOBOX_ITEM_NUM];

	int m_ComboChangeNum;
    CComboBox m_cbGraphicsType;

private:
	void SetPaintType();

    void SaveComboboxData();
    void LoadComboboxData();

    int m_edit_two_value;
    int m_edit_filter_value;
    int m_edit_imdilate_value;
    int m_edit_white_spot_max;
    int m_edit_white_min;
    int m_edit_night_thr;
    int m_edit_max_frame;
    int m_edit_alarm_delay;
    int m_edit_alarm;

public:
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnCbnSelchangeComboControl();
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
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedCheckPerson();
    afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeEditWhiteSpotMin();
	afx_msg void OnEnChangeEditFilterValue();
	afx_msg void OnEnChangeEditImdilateValue();
	afx_msg void OnEnChangeEditWhiteSpotMax();
	afx_msg void OnEnChangeEditNightThr();
	afx_msg void OnEnChangeEditAlarm();
	afx_msg void OnEnChangeEditAlarmDelay();
    afx_msg void OnBnClickedButtonNext();
	afx_msg void OnBnClickedCheckNight();
};

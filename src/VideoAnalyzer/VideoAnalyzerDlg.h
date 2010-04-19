// VideoAnalyzerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "resource.h"

#include "FrameReceiver.h"
#include "Alerter.h"

#include "ConfigFile.h"
#include "VideoGraber.h"
#include "VideoRecorder.h"
#include "PreviewStatic.h"
#include "Algorithm/Analyzer.h"
#include "afxcmn.h"
#include "HeartBeat.h"

#include "Common/timer.h"

#include <vector>

enum SOURCETYPE
{
    TYPE_CAMERA,
    TYPE_FILE
};

enum VIDEO_CONTROL
{
    VC_NO,
    VC_OPEN,       //打开
    VC_CLOSE,      //关闭
    VC_RESTART,    //重启
    VC_DESTROY     //关闭窗口
};

enum
{
    COL_ID_TIME,
    COL_ID_STATUS
};

struct TVideoSource
{
    SOURCETYPE eType;
    int iCamID;
    CString strFileName;

    TVideoSource(): eType(TYPE_CAMERA), iCamID(-1) {}
};

class CDlgCfgBase;

struct CAnalyzerMgr
{
    CAnalyzer* pAnalyzer;
    CDialog* pDlgCfg;
    const char* pComment;

    CAnalyzerMgr(): pAnalyzer(NULL), pDlgCfg(NULL), pComment(NULL) {}
    CAnalyzerMgr(CAnalyzer* a, CDialog* d, const char* c)
        : pAnalyzer(a), pDlgCfg(d), pComment(c) {}
};

// CVideoAnalyzerDlg 对话框
class CVideoAnalyzerDlg : public CDialog, IFrameReceiver, IAlerter
{
// 构造
public:
	CVideoAnalyzerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_VIDEOANALYZER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

    virtual void updateFrame(const IplImage *pFrame);
    virtual void alert(const IplImage *pFrame);

    void ExpandDialog (int nResourceID, BOOL bExpand);
    void EnableVisibleChildren();
    void AddRunStatus(const char* szStatus, ...);

    bool openSource(TVideoSource& tSource);
    void closeSource();

    bool startRecord(const char* szFileName);
    void stopRecord();

    bool loadConfig();
    bool saveConfig();

    void ResetAnalyzer();

    bool CreateMultipleDirectory(const CString& szPath);

// 实现
protected:
	HICON m_hIcon;

    CVideoGraber* m_pVideoGraber;
    CAnalyzer* m_pAnalyzer;
    CDialog* m_pDlgCfg;
    IAlerter* m_pUDPAlerter;
    IAlerter* m_pUDPAlerter_2;
    CVideoRecorder* m_pVideoRecoder;
    CHeartBeat* m_pHeartBeat;

    CTimer m_timer;
    CString m_strAppPath;
    VIDEO_CONTROL m_eVideoCtrl;
    TVideoSource m_tSource;
    CCfgParse m_cfgParse;
    int m_iWidth, m_iHeight;
    bool m_bPause;
    unsigned int m_uCurrentFrame;
    unsigned int m_uAlert;
    bool m_bRecord;
    std::vector<CAnalyzerMgr> m_analyzerMgr;

    CButton m_btnRestart;
    CButton m_btnPause;
    CButton m_chkPreview;
    CButton m_chkDebug;
    CComboBox m_cbCamera;
    CComboBox m_cbAnalyzer;
    CComboBox m_cbConfigFile;
    CComboBox m_cbChannel;
    CPreviewStatic m_ctlVideo;
    CEdit m_edtWidth;
    CEdit m_edtHeight;
    CEdit m_edtFrameRate;
    CEdit m_edtStartFrame;
    CEdit m_edtStartTime;
    CEdit m_edtRecPath;
    CStatic m_txtCurrentFrame;
    CStatic m_txtAlert;
    CListCtrl m_lstStatus;
    CMenu m_menu;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnMsgVideoEnd(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
    void setVideoControl(VIDEO_CONTROL c);
    bool autoStart();

    afx_msg void OnBnClickedCheckPreview();
    afx_msg void OnCbnDropdownComboCamera();
    afx_msg void OnBnClickedButtonOpenFile();
    afx_msg void OnBnClickedButtonClose();
    afx_msg void OnCbnSelchangeComboCamera();
    afx_msg void OnBnClickedButtonRestart();
    afx_msg void OnBnClickedButtonPause();
    afx_msg void OnBnClickedButtonStep();
    afx_msg void OnBnClickedButtonApplyRes();
    afx_msg void OnBnClickedButtonApplyFr();
    afx_msg void OnCbnSelchangeComboAyalyzer();
    afx_msg void OnNMRClickListStatus(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnMenuClear();
    afx_msg void OnBnClickedButtonSetup();
    afx_msg void OnBnClickedCheckDebug();
    afx_msg void OnClose();
    afx_msg void OnEnSetfocusEditStartFrame();
    afx_msg void OnEnSetfocusEditStartTime();
    afx_msg void OnBnClickedButtonRecOpen();
    afx_msg void OnBnClickedButtonRecStart();
};

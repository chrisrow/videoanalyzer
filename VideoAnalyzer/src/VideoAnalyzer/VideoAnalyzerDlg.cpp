// VideoAnalyzerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "VideoAnalyzerDlg.h"

#include "UDPAlerter.h"
#include "Algorithm/ParabolaWarpper.h"
#include "Algorithm/Macro.h"
#include "Option.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int g_debug;

extern struct ParamStruct ParamSet;
extern struct ParamDistinguish ParamDsting;


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVideoAnalyzerDlg 对话框




CVideoAnalyzerDlg::CVideoAnalyzerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoAnalyzerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pVideoGraber = NULL;
    m_pAnalyzer = NULL;
    m_pUDPAlerter = NULL;
    m_eVideoCtrl   = VC_NO;
    m_bPause = false;
    m_uCurrentFrame = 0;
    m_uAlert = 0;
    m_bRecord = false;
    m_pVideoRecoder = NULL;
    m_pHeartBeat = NULL;
    m_iWidth = 352;
    m_iHeight = 288;
}

void CVideoAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHECK_PREVIEW, m_chkPreview);
    DDX_Control(pDX, IDC_COMBO_CAMERA, m_cbCamera);
    DDX_Control(pDX, IDC_VIDEO, m_ctlVideo);
    DDX_Control(pDX, IDC_EDIT_WIDTH, m_edtWidth);
    DDX_Control(pDX, IDC_EDIT_HEIGHT, m_edtHeight);
    DDX_Control(pDX, IDC_EDIT_FRAME_RATE, m_edtFrameRate);
    DDX_Control(pDX, IDC_EDIT_START_FRAME, m_edtStartFrame);
    DDX_Control(pDX, IDC_EDIT_START_TIME, m_edtStartTime);
    DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
    DDX_Control(pDX, IDC_STATIC_CURRENT_FRAME, m_txtCurrentFrame);
    DDX_Control(pDX, IDC_CHECK_DEBUG, m_chkDebug);
    DDX_Control(pDX, IDC_BUTTON_RESTART, m_btnRestart);
    DDX_Control(pDX, IDC_COMBO_AYALYZER, m_cbAnalyzer);
    DDX_Control(pDX, IDC_STATIC_CURRENT_ALERT, m_txtAlert);
    DDX_Control(pDX, IDC_LIST_STATUS, m_lstStatus);
    DDX_Control(pDX, IDC_COMBO_CONFIG_FILE, m_cbConfigFile);
    DDX_Control(pDX, IDC_COMBO_CHANNEL, m_cbChannel);
    DDX_Control(pDX, IDC_EDIT_REC_PATH, m_edtRecPath);
}

BEGIN_MESSAGE_MAP(CVideoAnalyzerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_CBN_DROPDOWN(IDC_COMBO_CAMERA, &CVideoAnalyzerDlg::OnCbnDropdownComboCamera)
    ON_CBN_SELCHANGE(IDC_COMBO_CAMERA, &CVideoAnalyzerDlg::OnCbnSelchangeComboCamera)
    ON_CBN_SELCHANGE(IDC_COMBO_AYALYZER, &CVideoAnalyzerDlg::OnCbnSelchangeComboAyalyzer)
    ON_BN_CLICKED(IDC_CHECK_PREVIEW, &CVideoAnalyzerDlg::OnBnClickedCheckPreview)
    ON_BN_CLICKED(IDC_BUTTON_OPEN_FILE, &CVideoAnalyzerDlg::OnBnClickedButtonOpenFile)
    ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CVideoAnalyzerDlg::OnBnClickedButtonClose)
    ON_BN_CLICKED(IDC_BUTTON_RESTART, &CVideoAnalyzerDlg::OnBnClickedButtonRestart)
    ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CVideoAnalyzerDlg::OnBnClickedButtonPause)
    ON_BN_CLICKED(IDC_BUTTON_STEP, &CVideoAnalyzerDlg::OnBnClickedButtonStep)
    ON_BN_CLICKED(IDC_BUTTON_APPLY_RES, &CVideoAnalyzerDlg::OnBnClickedButtonApplyRes)
    ON_BN_CLICKED(IDC_BUTTON_APPLY_FR, &CVideoAnalyzerDlg::OnBnClickedButtonApplyFr)
    ON_BN_CLICKED(IDC_BUTTON_SETUP, &CVideoAnalyzerDlg::OnBnClickedButtonSetup)
    ON_BN_CLICKED(IDC_CHECK_DEBUG, &CVideoAnalyzerDlg::OnBnClickedCheckDebug)
    ON_BN_CLICKED(IDC_BUTTON_REC_OPEN, &CVideoAnalyzerDlg::OnBnClickedButtonRecOpen)
    ON_BN_CLICKED(IDC_BUTTON_REC_START, &CVideoAnalyzerDlg::OnBnClickedButtonRecStart)
    ON_MESSAGE(WM_VIDEO_END, OnMsgVideoEnd)
    ON_NOTIFY(NM_RCLICK, IDC_LIST_STATUS, &CVideoAnalyzerDlg::OnNMRClickListStatus)
    ON_COMMAND(ID_MENU_CLEAR, &CVideoAnalyzerDlg::OnMenuClear)
    ON_WM_CLOSE()
    ON_EN_SETFOCUS(IDC_EDIT_START_FRAME, &CVideoAnalyzerDlg::OnEnSetfocusEditStartFrame)
    ON_EN_SETFOCUS(IDC_EDIT_START_TIME, &CVideoAnalyzerDlg::OnEnSetfocusEditStartTime)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CVideoAnalyzerDlg 消息处理程序

BOOL CVideoAnalyzerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

    //列表框
    CRect rectListStatus;
    m_lstStatus.GetClientRect(&rectListStatus);
    m_lstStatus.InsertColumn(COL_ID_TIME,   _T("时间"), LVCFMT_LEFT, 110);
    m_lstStatus.InsertColumn(COL_ID_STATUS, _T("运行信息"), LVCFMT_LEFT, 
        rectListStatus.Width() - 130);
    m_lstStatus.SetExtendedStyle(m_lstStatus.GetExtendedStyle()|LVS_EX_GRIDLINES );

    //列表框菜单
    m_menu.LoadMenu(IDR_MENU_LIST);

    //选项
    m_chkPreview.SetCheck(1);
    m_chkDebug.SetCheck(0);//改动
    g_debug = 0;
    
    //选择视频属性
    CString strWidth, strHeight, strFR;
    strWidth.Format("%d", m_iWidth);
    strHeight.Format("%d", m_iHeight);
    strFR.Format("%d", 100);
    m_edtWidth.SetWindowText(strWidth);
    m_edtHeight.SetWindowText(strHeight);
    m_edtFrameRate.SetWindowText(strFR);
    m_ctlVideo.SetWindowPos(NULL, 0, 0, m_iWidth, m_iHeight, SWP_NOMOVE | SWP_NOZORDER);

    //选择摄像机
    OnCbnDropdownComboCamera();

    //选择分析器
    m_cbAnalyzer.AddString("抛物检测");
    m_cbAnalyzer.AddString("无");
    m_cbAnalyzer.SetCurSel(0);
    this->OnCbnSelchangeComboAyalyzer();

    //获取本程序的路径
    GetModuleFileName(NULL,m_strAppPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
    m_strAppPath.ReleaseBuffer();   
    int iPos2;   
    iPos2 = m_strAppPath.ReverseFind('\\');   
    m_strAppPath = m_strAppPath.Left(iPos2);   

    //配置文件，默认文件名与本程序的名称相同，后缀为xml
    CString strConfigFile;
    GetModuleFileName(NULL,strConfigFile.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
    strConfigFile.ReleaseBuffer();   
    int iPos = 0;   
    iPos = strConfigFile.ReverseFind('\\');   
    strConfigFile = strConfigFile.Right(strConfigFile.GetLength() - iPos - 1);   
    iPos = strConfigFile.ReverseFind('.');   
    strConfigFile = strConfigFile.Left(iPos);   
    strConfigFile += ".xml";

    m_cbConfigFile.AddString((LPCTSTR)strConfigFile);
    m_cbConfigFile.SetCurSel(0);
    for (int i = 0; i < 9; i++)
    {
        CString str;
        str.Format("%d", i);
        m_cbChannel.AddString(str);
    }
    m_cbChannel.SetCurSel(0);

    m_edtRecPath.SetWindowText("f:\\0.avi");

    this->loadConfig();

    this->AddRunStatus("程序启动");

    this->autoStart();

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVideoAnalyzerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVideoAnalyzerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVideoAnalyzerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CVideoAnalyzerDlg::OnMsgVideoEnd(WPARAM wParam,LPARAM lParam)
{
    delete m_pVideoGraber;
    m_pVideoGraber = NULL;

    this->AddRunStatus("已关闭");

    switch (m_eVideoCtrl)
    {
    case VC_RESTART: 
        m_btnRestart.EnableWindow(TRUE);
        (void)this->openSource(m_tSource);
    	break;
    case VC_DESTROY:
        SendMessage(WM_CLOSE, 0, 0);
        break;
    case VC_CLOSE:
    default:
        break;
    }

    m_eVideoCtrl = VC_NO;
    m_btnRestart.EnableWindow(TRUE);

    return 0;
}

void CVideoAnalyzerDlg::updateFrame(const IplImage *pFrame)
{
    m_uCurrentFrame++;

//     if (m_chkDebug.GetCheck())
    {
        CString str;
        str.Format("%u", m_uCurrentFrame);
        m_txtCurrentFrame.SetWindowText(str);
    }

    m_ctlVideo.showImage(pFrame);
}

void CVideoAnalyzerDlg::alert(const IplImage *pFrame)
{
    m_uAlert++;

    SYSTEMTIME Systemtime ;
    GetLocalTime(&Systemtime);
    
    //创建目录
    CString strPath;
    strPath.Format(_T("D:\\%d月%d日"),Systemtime.wMonth,Systemtime.wDay);
    CreateDirectory(strPath, NULL);
    
    int iChannel = 0;
    if (TYPE_CAMERA == m_tSource.eType)
    {
        iChannel = m_cbCamera.GetCurSel();
    }
    else
    {
        iChannel = m_cbChannel.GetCurSel();
    }

    if (iChannel < 0)
    {
        iChannel = 0;
    }

    //保存图片
    CString strFile, strChannel;
    m_cbChannel.GetWindowText(strChannel);
    strFile.Format("D:\\%d月%d日\\%d_%d_%d_%d.jpg",
        Systemtime.wMonth,Systemtime.wDay,
        iChannel,
        Systemtime.wHour, Systemtime.wMinute, Systemtime.wSecond );
    cvFlip(pFrame, NULL, 0);//垂直镜像
    int iResult = cvSaveImage((LPCTSTR)strFile, pFrame);

    //列表框提示
    CString str;
    str.Format("%d", m_uAlert);
    m_txtAlert.SetWindowText(str);
    this->AddRunStatus("第%d帧触发警报(%s_%d_%d_%d.jpg)", 
        m_uCurrentFrame,(LPCTSTR)strChannel,
        Systemtime.wHour, Systemtime.wMinute, Systemtime.wSecond);
}

void CVideoAnalyzerDlg::OnBnClickedCheckPreview()
{
    if (m_pVideoGraber)
    {
        if (m_chkPreview.GetCheck())
        {
            m_pVideoGraber->addListener(this);
        } 
        else
        {
            m_pVideoGraber->removeListener(this);
        }
    }
    ExpandDialog (IDC_VIDEO, m_chkPreview.GetCheck());
}


void CVideoAnalyzerDlg::ExpandDialog (int nResourceID, BOOL bExpand)
{
    // 对话框被nResourceID分成上下两部分，如果bExpand的值为TRUE
    // 对话框被完整显示，否则对话框显示上半部分。
    static CRect rcLarge;
    static CRect rcSmall;
    CString sExpand;
    //开始时，对话框只显示上半部分
    if (rcLarge.IsRectNull())
    {
        CRect rcLandmark;
        CWnd* pWndLandmark = GetDlgItem (nResourceID);
        ASSERT(pWndLandmark); 
        GetWindowRect (rcLarge);
        pWndLandmark->GetWindowRect (rcLandmark);
        rcSmall = rcLarge;
        //rcSmall.bottom = rcLandmark.top;
        rcSmall.right = rcLandmark.left;
    }
    if (bExpand)
    {
        //扩展对话框到最大尺寸
        SetWindowPos(NULL, 0, 0, rcLarge.Width(), rcLarge.Height(),
            SWP_NOMOVE | SWP_NOZORDER);
        //EnableVisibleChildren();
    }
    else
    {
        //只显示对话框的上半部分
        SetWindowPos(NULL, 0, 0, rcSmall.Width(), rcSmall.Height(),
            SWP_NOMOVE | SWP_NOZORDER);
        //EnableVisibleChildren();
    } 
}

void CVideoAnalyzerDlg::EnableVisibleChildren()
{
    //去掉没有显示的对话框的控件的功能和快捷键。
    //得到第一个窗口
    CWnd *pWndCtl = GetWindow (GW_CHILD);
    CRect rcTest;
    CRect rcControl;
    CRect rcShow;
    //得到对话框的完整矩形框
    GetWindowRect(rcShow);
    while (pWndCtl != NULL)
    { //得到当前显示的对话框的矩形尺寸
        pWndCtl->GetWindowRect (rcControl);
        if (rcTest.IntersectRect (rcShow, rcControl))
            pWndCtl->EnableWindow(TRUE);
        else
            pWndCtl->EnableWindow(FALSE);
        //得到第二个矩形框
        pWndCtl = pWndCtl->GetWindow (GW_HWNDNEXT);
    }
} 

bool CVideoAnalyzerDlg::autoStart()
{
    bool bAutoStart = false;
    COption& option = COption::Instance();
    if (option.getFileName() != NULL)
    {
        m_tSource.eType = TYPE_FILE;
        m_tSource.strFileName = option.getFileName();
        bAutoStart = true;
    } 
    else if(option.getCameraID() >= 0)
    {
        m_tSource.eType = TYPE_CAMERA;
        m_tSource.iCamID = option.getCameraID();
        this->OnCbnDropdownComboCamera();
        m_cbCamera.SetCurSel(m_tSource.iCamID);
        bAutoStart = true;
    }
    else if (0 /* option.getURL() != NULL */)
    {
        // imcomplete
    }

    //最简界面模式，并设置窗口大小和窗口位置
    const char* pszSimple = option.getSimple();
    int iWidth = -1, iHeight = -1, iPosX = -1, iPosY = -1;
    if (pszSimple)
    {
        sscanf(pszSimple, "%dx%d;(%d,%d)", &iWidth, &iHeight, &iPosX, &iPosY);

        //窗口大小
        if (iWidth < 0 || iHeight < 0)
        {
            iWidth = m_iWidth;
            iHeight = m_iHeight;
        }
        else
        {
            m_iWidth = iWidth;
            m_iHeight = iHeight;
        }
        m_ctlVideo.MoveWindow(0, 0, iWidth, iHeight);
        SetWindowPos(NULL, 0, 0, iWidth, iHeight, SWP_NOMOVE | SWP_NOZORDER);

        //窗口位置
        if (iPosX >= 0 && iPosY >= 0)
        {
            SetWindowPos(NULL, iPosX, iPosY, 0, 0, SWP_NOSIZE |SWP_NOZORDER);
        }

        //去掉标题栏
        SetWindowLong(m_hWnd, GWL_STYLE, WS_VISIBLE);
        //去掉没有显示的对话框的控件的功能和快捷键。
        //得到第一个窗口
        CWnd *pWndCtl = NULL;
        CRect rcTest;
        CRect rcControl;
        CRect rcVideo;
        m_ctlVideo.GetWindowRect(rcVideo);
        pWndCtl = m_ctlVideo.GetWindow (GW_HWNDNEXT);
        while (pWndCtl != NULL)
        {
            pWndCtl->GetWindowRect (rcControl);
            if (rcTest.IntersectRect (rcVideo, rcControl)) //获取两者的重叠区域，如果不重叠，则为0
            {
                pWndCtl->ShowWindow(FALSE);
            }

            pWndCtl = pWndCtl->GetWindow (GW_HWNDNEXT);
        }

        //临时
        GetDlgItem(IDC_BUTTON_OPEN_FILE)->ShowWindow(FALSE);
        m_cbCamera.ShowWindow(FALSE);
    }

    //设置视频分辨率
    CString strWidth, strHeight;
    strWidth.Format("%d", m_iWidth);
    strHeight.Format("%d", m_iHeight);
    m_edtWidth.SetWindowText(strWidth);
    m_edtHeight.SetWindowText(strHeight);

    //如果是自启动，则不显示预览
    if (bAutoStart)
    {
        if(this->openSource(m_tSource))
        {
            // -p 显示预览
            if (!option.isPreview())
            {
                m_chkPreview.SetCheck(BST_UNCHECKED);
                ExpandDialog (IDC_VIDEO, m_chkPreview.GetCheck());
            }

            //由于openSource中会根据视频属性重新设置图片控件的大小
            //所以这里需要按照输入参数将图片控件的大小改回来
            if (pszSimple)
            {
                m_ctlVideo.MoveWindow(0, 0, iWidth, iHeight);
            }
        }
    }

    return true;
}


bool CVideoAnalyzerDlg::openSource(TVideoSource& tSource)
{
    (void)this->loadConfig();

    //打开视频源
    CString strWndCaption = "VideoAnalyzer";
    bool bResult = false;
    if (TYPE_CAMERA == tSource.eType)
    {
        CString tmp;
        tmp.Format("Camera %d ", tSource.iCamID);
		strWndCaption = tmp + strWndCaption; 
        //strWndCaption += tmp;
        SetWindowText(strWndCaption);

        this->AddRunStatus("打开摄像机 %d", tSource.iCamID);
        m_pVideoGraber = new CCameraWarpper(this);
        bResult = m_pVideoGraber->open(tSource.iCamID);
    } 
    else // TYPE_FILE == tSource.eType
    {
        tSource.iCamID = -1;
        strWndCaption += " " + tSource.strFileName;
        SetWindowText(strWndCaption);

        this->AddRunStatus("打开视频文件%s", (LPCTSTR)tSource.strFileName);
        m_pVideoGraber = new CVideoFile(this);
        bResult = m_pVideoGraber->open((LPCTSTR)tSource.strFileName);
    }

    if (!bResult)
    {
        this->AddRunStatus("打开视频源失败");
        delete m_pVideoGraber;
        m_pVideoGraber = NULL;
        return false;
    }

    //设置视频显示
    m_pVideoGraber->addListener(this);

    //设置视频属性
    double dWidth, dHeight, dFrameRate, dStartFrame, dStartTime;
    CString strWidth, strHeight, strFrameRate, strStartFrame, strStartTime;

    m_edtWidth.GetWindowText(strWidth);
    m_edtHeight.GetWindowText(strHeight);
    m_edtFrameRate.GetWindowText(strFrameRate);
    m_edtStartFrame.GetWindowText(strStartFrame);
    m_edtStartTime.GetWindowText(strStartTime);

    m_iWidth = atoi((LPCTSTR)strWidth);
    m_iHeight = atoi((LPCTSTR)strHeight);
    dWidth = m_iWidth;
    dHeight = m_iHeight;
    dFrameRate = atoi((LPCTSTR)strFrameRate);
    dStartFrame = atoi((LPCTSTR)strStartFrame);
    dStartTime = atoi((LPCTSTR)strStartTime);

    m_pVideoGraber->setProperty(PROP_WIDTH, dWidth);
    m_pVideoGraber->setProperty(PROP_HEITHT, dHeight);
    m_pVideoGraber->setProperty(PROP_FRAMERATE, dFrameRate);

    //根据视频属性重新设置对话框中的内容
    m_pVideoGraber->getProperty(PROP_WIDTH, dWidth);
    m_pVideoGraber->getProperty(PROP_HEITHT, dHeight);
    m_iWidth = (int)dWidth;
    m_iHeight = (int)dHeight;
    m_ctlVideo.SetWindowPos(NULL, 0, 0, m_iWidth, m_iHeight, SWP_NOMOVE);
    strWidth.Format("%d", m_iWidth);
    strHeight.Format("%d", m_iHeight);
    m_edtWidth.SetWindowText(strWidth);
    m_edtHeight.SetWindowText(strHeight);

    m_uCurrentFrame = 0;
    if (TYPE_FILE == tSource.eType)
    {
        int iRadioCtrlID = GetCheckedRadioButton(IDC_RADIO_START_FRAME, IDC_RADIO_START_TIME);
        if (IDC_RADIO_START_FRAME == iRadioCtrlID)
        {
            dStartTime = 0;
            m_pVideoGraber->setProperty(PROP_POS_FRAME, dStartFrame);
        } 
        else if(IDC_RADIO_START_TIME == iRadioCtrlID)
        {
            dStartFrame = 0;
            m_pVideoGraber->setProperty(PROP_POS_MSEC, dStartTime);
        }

        if (iRadioCtrlID != 0 && (dStartFrame > 0.1 || dStartTime > 0.0001))
        {
            m_pVideoGraber->pause();
            m_btnPause.SetWindowText("继续");
            m_bPause = true;
            m_uCurrentFrame = (unsigned int)(dStartFrame-1);
        }
        else
        {
            m_btnPause.SetWindowText("暂停");
            m_bPause = false;
        }
    }

    m_uAlert = 0;
    m_txtAlert.SetWindowText("0");

    //设置视频分析器
    OnCbnSelchangeComboAyalyzer();
    if(m_pAnalyzer)
    {
        m_pVideoGraber->addListener(m_pAnalyzer);
    }

    //获取远程、本地IP地址和端口
    unsigned char local[4] = {0, 0, 0, 0};
    unsigned char remote[4] = {127, 0, 0, 1}; // {192, 168, 1, 74}
    int port = 0;

    int tmp[4] = {0};
    const char* ip = NULL;
    if ( NULL == (ip = m_cfgParse.GetGolbalParam("UDPServer")) )
    {
        this->AddRunStatus("获取UDPServer的IP地址失败，使用默认地址'192.168.1.74'");
        ip = "192.168.1.74";
    }
    m_cfgParse.GetGolbalParam("UDPServer");
    sscanf(ip, "%d.%d.%d.%d:%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3], &port);
    remote[0] = (unsigned char)tmp[0];
    remote[1] = (unsigned char)tmp[1];
    remote[2] = (unsigned char)tmp[2];
    remote[3] = (unsigned char)tmp[3];    
    
    //通道号
    int iChannel = 0;
    if (TYPE_CAMERA == m_tSource.eType)
    {
        iChannel = m_cbCamera.GetCurSel();
    }
    else
    {
        iChannel = m_cbChannel.GetCurSel();
    }

    //心跳
    if (NULL == m_pHeartBeat)
    {
        m_pHeartBeat = new CHeartBeat;
    }
    int iInterval = 0;
    const char* pInterval = NULL;
    if ( NULL == (pInterval = m_cfgParse.GetGolbalParam("HeartBeat")) )
    {
        this->AddRunStatus("获取发送心跳消息间隔时间失败，使用默认时间：180秒");
        iInterval = 180;
    }
    else
    {
        iInterval = atoi(pInterval);
    }
    if (!m_pHeartBeat->init(iChannel, local, remote, port, iInterval))
    {
        this->AddRunStatus("初始化心跳消息失败");
        return false;
    }
    m_pHeartBeat->run();

    //设置报警器
    if (m_pAnalyzer && NULL == m_pUDPAlerter)
    {
        CUDPAlerter* pUDPAlerter = new CUDPAlerter ;

        int iAlarmType = 1;
        this->AddRunStatus("报警中心：%d.%d.%d.%d:%d", 
            remote[0], remote[1], remote[2], remote[3], port);
        (void)pUDPAlerter->init(iAlarmType, iChannel, local, remote, port);
        m_pUDPAlerter = pUDPAlerter;
    }
    if (m_pAnalyzer)
    {
        m_pAnalyzer->addListener(m_pUDPAlerter);
    }

    INIT_IMAGE();
    this->AddRunStatus("正在播放");
    m_pVideoGraber->loopGetFrame();

	SetTimer(1,180000,NULL); 

    return true;
}

void CVideoAnalyzerDlg::closeSource()
{
    if (m_pVideoGraber)
    {
        m_pVideoGraber->close();
    }

    if (m_pVideoRecoder)
    {
        m_pVideoRecoder->stop();
    }

    if (m_pHeartBeat)
    {
        m_pHeartBeat->destroy();
    }
}

void CVideoAnalyzerDlg::setVideoControl(VIDEO_CONTROL c)
{
    m_eVideoCtrl = c;
}


void CVideoAnalyzerDlg::OnCbnDropdownComboCamera()
{
    CCameraMgr* pCameraMgr = new CVIMgr;
    int iCount = pCameraMgr->getCount();

    std::vector<CString> vCameraName;
    for(int i = 0; i < iCount; i++)
    {
        CString strName;
        strName.Format("%d|", i);
        strName += pCameraMgr->getName(i);
        vCameraName.push_back(strName);
    }

    m_cbCamera.ResetContent();
    for (unsigned i = 0; i < vCameraName.size(); i++)
    {
        m_cbCamera.AddString(vCameraName[i]);
    }


//     m_tSource.eType = TYPE_CAMERA;
    if (iCount > 0)
    {
        if (m_tSource.iCamID >= 0 && m_tSource.iCamID < iCount)
        {
            m_cbCamera.SetCurSel(m_tSource.iCamID);
        }
        else
        {
            m_cbCamera.SetCurSel(0);
        }
    }

    delete pCameraMgr;
    pCameraMgr = NULL;
}

void CVideoAnalyzerDlg::OnCbnSelchangeComboCamera()
{
    m_tSource.eType = TYPE_CAMERA;
    m_tSource.iCamID = m_cbCamera.GetCurSel();

    this->OnBnClickedButtonRestart();

    CString str;
    str.Format("f:\\%d.avi", m_tSource.iCamID);
    m_edtRecPath.SetWindowText(str);
}

void CVideoAnalyzerDlg::OnBnClickedButtonOpenFile()
{
    CFileDialog	FileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY, "所有文件 (*.*)|*.*||");
    if (FileDlg.DoModal() == IDOK)	
    {
        m_tSource.strFileName = FileDlg.GetPathName();
        m_tSource.eType = TYPE_FILE;
        this->OnBnClickedButtonRestart();
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonClose()
{
    if (!m_pVideoGraber || m_pVideoGraber->getStatus() == STAT_STOPPED)
    {
        this->AddRunStatus("已关闭");
    } 
    else
    {
        this->AddRunStatus("正在关闭");
        this->closeSource();
        m_eVideoCtrl = VC_CLOSE;
    }
}


void CVideoAnalyzerDlg::OnBnClickedButtonRestart()
{
    if (!m_pVideoGraber || m_pVideoGraber->getStatus() == STAT_STOPPED)
    {
        this->openSource(m_tSource);
    } 
    else
    {
        m_btnRestart.EnableWindow(FALSE);
        this->closeSource();
    }
    m_eVideoCtrl = VC_RESTART;
}

void CVideoAnalyzerDlg::OnBnClickedButtonPause()
{
    if (m_pVideoGraber)
    {
        m_pVideoGraber->setProperty(PROP_STEP, 0);
        if (m_bPause)
        {
            m_pVideoGraber->resume();
            m_btnPause.SetWindowText("暂停");
            m_bPause = false;
        } 
        else
        {
            m_pVideoGraber->pause();
            m_btnPause.SetWindowText("继续");
            m_bPause = true;
        }
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonStep()
{
    if (m_pVideoGraber)
    {
        m_pVideoGraber->setProperty(PROP_STEP, 1);
        m_pVideoGraber->resume();
    }
}

void CVideoAnalyzerDlg::OnClose()
{
    if (m_pVideoGraber && m_pVideoGraber->getStatus() != STAT_STOPPED)
    {
        m_eVideoCtrl = VC_DESTROY;
        this->closeSource();
        return;
    }
    else//已经停止
    {
        delete m_pVideoGraber;
        m_pVideoGraber = NULL;

        delete m_pUDPAlerter;
        m_pUDPAlerter = NULL;

        CSubject<IAlerter*>* pSubject = dynamic_cast<CSubject<IAlerter*>*>(m_pAnalyzer);
        if (pSubject)
        {
            pSubject->clearListener();
        }

        delete m_pAnalyzer;
        m_pAnalyzer = NULL;

        delete m_pVideoRecoder;
        m_pVideoRecoder = NULL;

        delete m_pHeartBeat;
        m_pHeartBeat = NULL;

        __super::OnClose();
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonApplyRes()
{
    if (m_pVideoGraber)
    {
        double dWidth, dHeight;
        CString strWidth, strHeight;

        m_edtWidth.GetWindowText(strWidth);
        m_edtHeight.GetWindowText(strHeight);

        m_iWidth = atoi((LPCTSTR)strWidth);
        m_iHeight = atoi((LPCTSTR)strHeight);
        dWidth = m_iWidth;
        dHeight = m_iHeight;

        m_pVideoGraber->setProperty(PROP_WIDTH, dWidth);
        m_pVideoGraber->setProperty(PROP_HEITHT, dHeight);

        m_pVideoGraber->getProperty(PROP_WIDTH, dWidth);
        m_pVideoGraber->getProperty(PROP_HEITHT, dHeight);
        GetDlgItem(IDC_VIDEO)->SetWindowPos(NULL, 0, 0, 
            (int)dWidth, (int)dHeight, SWP_NOMOVE);
        strWidth.Format("%d", int(dWidth));
        strHeight.Format("%d", int(dHeight));
        m_edtWidth.SetWindowText(strWidth);
        m_edtHeight.SetWindowText(strHeight);
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonApplyFr()
{
    if (m_pVideoGraber)
    {
        double dFrameRate;
        CString strFrameRate;

        m_edtFrameRate.GetWindowText(strFrameRate);
        dFrameRate = atoi((LPCTSTR)strFrameRate);
        m_pVideoGraber->setProperty(PROP_FRAMERATE, dFrameRate);
    }
}

void CVideoAnalyzerDlg::OnCbnSelchangeComboAyalyzer()
{
    delete m_pAnalyzer;
    m_pAnalyzer = NULL;
    m_pAnalyzer = new CParabolaWarpper;
    m_pAnalyzer->addListener(this);
}

void CVideoAnalyzerDlg::AddRunStatus(const char* szStatus, ...)
{
    if(m_lstStatus.GetItemCount() >= 100)
    {
        m_lstStatus.DeleteItem(99);
    }

    time_t now = time(NULL);
    struct tm *tmNow = localtime(&now);
    char szTimeString[128] = {0};
    strftime( szTimeString, 128, "%m-%d %H:%M:%S", tmNow );
    int nRow = m_lstStatus.InsertItem(COL_ID_TIME, szTimeString);

    char buf[4096] = {0};
    va_list ap;// typedef char *  va_list;
    va_start(ap, szStatus);//#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
    vsprintf( buf, szStatus, ap );
    va_end( ap);   //#define va_end(ap)      ( ap = (va_list)0 )

    m_lstStatus.SetItemText(nRow, COL_ID_STATUS, buf);
}

void CVideoAnalyzerDlg::OnNMRClickListStatus(NMHDR *pNMHDR, LRESULT *pResult)
{
//     LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);

    CPoint   ptCursorPos; 
    GetCursorPos(&ptCursorPos); 
    CRect   rclist; 
    m_lstStatus.GetWindowRect(&rclist); 
    if(rclist.PtInRect(ptCursorPos)) 
    {
        CMenu* popup = m_menu.GetSubMenu(0);
        ASSERT( popup != NULL );
        m_menu.SetDefaultItem(ID_MENU_CLEAR);
        popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
            ptCursorPos.x, ptCursorPos.y, this );

    }

    *pResult = 0;
}

void CVideoAnalyzerDlg::OnMenuClear()
{
    m_lstStatus.DeleteAllItems();
}

void CVideoAnalyzerDlg::OnBnClickedButtonSetup()
{
    CDlgSetting dlgSetting;
    if ( !this->loadConfig() )
    {
        AfxMessageBox("载入配置数据失败");
    }
//CDlgSetting m_dlgSetting;
	dlgSetting.setImage(m_ctlVideo.getImage());

    if (dlgSetting.DoModal() != IDOK)
    {
		
        return;
    }

    this->saveConfig();
}

bool CVideoAnalyzerDlg::loadConfig()
{
    CString strConfigFile;
    CString strChannel;
    int iChannel = 0;
    m_cbConfigFile.GetWindowText(strConfigFile);
    m_cbChannel.GetWindowText(strChannel);
    strConfigFile = m_strAppPath + "\\" + strConfigFile;
    iChannel = atoi((LPCTSTR)strChannel);

    if (!m_cfgParse.Load((LPCTSTR)strConfigFile))
    {
        this->AddRunStatus("载入失败：%s", (LPCTSTR)strConfigFile);
        return false;
    }

    if (!m_cfgParse.LoadChannel(iChannel, ParamSet, ParamDsting))
    {
        this->AddRunStatus("载入通道%d的数据失败", iChannel);
        return false;
    }

    return true;
}

bool CVideoAnalyzerDlg::saveConfig()
{
    //保存参数到xml
    CString strConfigFile;
    CString strChannel;
    int iChannel = 0;
    m_cbConfigFile.GetWindowText(strConfigFile);
    m_cbChannel.GetWindowText(strChannel);
    strConfigFile = m_strAppPath + "\\" + strConfigFile;
    iChannel = atoi((LPCTSTR)strChannel);

    if (!m_cfgParse.SaveChannel(iChannel, ParamSet, ParamDsting))
    {
        this->AddRunStatus("保存通道%d的数据失败", iChannel);
        return false;
    }

    if (!m_cfgParse.Save((LPCTSTR)strConfigFile))
    {
        this->AddRunStatus("保存失败：%s", (LPCTSTR)strConfigFile);
        return false;
    }

    return true;
}

void CVideoAnalyzerDlg::OnBnClickedCheckDebug()
{
    if (m_chkDebug.GetCheck())
    {
        g_debug = 1;
    }
    else
    {
        g_debug = 0;
    }
}

void CVideoAnalyzerDlg::OnEnSetfocusEditStartFrame()
{
    CheckRadioButton(IDC_RADIO_START_FRAME, IDC_RADIO_START_TIME, IDC_RADIO_START_FRAME);
}

void CVideoAnalyzerDlg::OnEnSetfocusEditStartTime()
{
    CheckRadioButton(IDC_RADIO_START_FRAME, IDC_RADIO_START_TIME, IDC_RADIO_START_TIME);
}


void CVideoAnalyzerDlg::OnBnClickedButtonRecOpen()
{
    CFileDialog	FileDlg(FALSE, "avi", NULL, OFN_HIDEREADONLY, 
        "视频文件 (*.avi)|*.avi||");
    if (FileDlg.DoModal() == IDOK)	
    {
        m_edtRecPath.SetWindowText(FileDlg.GetPathName());
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonRecStart()
{
    if (m_bRecord) //正在录，停止
    {
        this->stopRecord();

        m_bRecord = false;
        m_edtRecPath.EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_REC_OPEN)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_REC_START)->SetWindowText("开始");
        this->AddRunStatus("录像停止");
    } 
    else //没有录，开始录像
    {
        CString strRecPath;
        m_edtRecPath.GetWindowText(strRecPath);
        if (strRecPath.IsEmpty())
        {
            AfxMessageBox("文件名为空");
            return;
        }

        if (!this->startRecord((LPCTSTR)strRecPath))
        {
            return;
        }
        m_bRecord = true;
        m_edtRecPath.EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_REC_OPEN)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_REC_START)->SetWindowText("停止");
        this->AddRunStatus("录像开始：%s", (LPCSTR)strRecPath);
    }
}

bool CVideoAnalyzerDlg::startRecord(const char* szFileName)
{
    if (!m_pVideoGraber)
    {
        this->AddRunStatus("启动录像失败：视频未启动");
        return false;
    }

    if (m_pVideoRecoder)
    {
        m_pVideoRecoder->stop();
        delete m_pVideoRecoder;
    }

    bool bResult = false;
    m_pVideoRecoder = new CVideoRecorder;
    m_pVideoRecoder->setResolution(m_iWidth, m_iHeight);
    if (m_pVideoRecoder->start(szFileName))
    {
        m_pVideoGraber->addListener(m_pVideoRecoder);
        bResult = true;
    }
    else
    {
        this->AddRunStatus("启动录像失败");
        bResult = false;
    }

    return bResult;
}

void CVideoAnalyzerDlg::stopRecord()
{
    if (m_pVideoGraber)
    {
        m_pVideoGraber->removeListener(m_pVideoRecoder);
    }
}

void CVideoAnalyzerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	CString strConfigFile;
	CString strChannel;
	int iChannel = 0;
	m_cbConfigFile.GetWindowText(strConfigFile);
	m_cbChannel.GetWindowText(strChannel);
	strConfigFile = m_strAppPath + "\\" + strConfigFile;
	iChannel = atoi((LPCTSTR)strChannel);

	DWORD temp_settng;
	CString address = _T("SOFTWARE\\HU_VIDEO");

	switch (iChannel)
	{
	case 0 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_0") ) ;
        if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
        {
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
        }

		break;
	case 1 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_1") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;
	case 2 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_2") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag =0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;
	case 3 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_3") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;
	case 4 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_4") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;
	case 5 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_5") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;
	case 6 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_6") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}
				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;
	case 7 : 
		temp_settng = OnRegRead( address , _T("VideoAnalyzer_7") ) ;
		if (temp_settng != ParamSet.bSensitiveFlag && temp_settng != 0 )
		{
			if (temp_settng > 0 && temp_settng <= 4)
			{
				if (temp_settng == 1 || temp_settng == 2 )
				{
					ParamSet.iStyleChange = 0 ;
					ParamDsting.FindObjValMax.FirstValue = 1 ;
					if ( ParamDsting.TopPointVal.SecondValue == 3 )
					{
						ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
					}
					else
					{
						ParamSet.iPersonFlag = 0 ;
					}

				}
				else
				{
					ParamSet.iStyleChange = ParamDsting.TopPointVal.SecondValue;
					ParamDsting.FindObjValMax.FirstValue = 0 ;
					ParamSet.iPersonFlag = ParamDsting.TopPointVal.FouthValue ;
				}
				ParamSet.bSensitiveFlag = temp_settng ;
				this->saveConfig();
			}
		}
		break;

	default :
		break;
	}
	__super::OnTimer(nIDEvent);
}
DWORD CVideoAnalyzerDlg::OnRegRead(CString  address , CString  str)
{
	HKEY hKey;
	long lRet ;

	lRet = RegOpenKey(HKEY_LOCAL_MACHINE, address, &hKey); /*_T("Software\\HybribioDMC5KC\\temperature")*/

	if(lRet != ERROR_SUCCESS)
		return 0;
	DWORD dwType;
	DWORD dwValue;
	DWORD dwAge  = 0 ;
	RegQueryValueEx(hKey,str,0,&dwType,(LPBYTE)&dwAge,&dwValue);
	if(lRet != ERROR_SUCCESS)
		return 0;
	return dwAge;
}
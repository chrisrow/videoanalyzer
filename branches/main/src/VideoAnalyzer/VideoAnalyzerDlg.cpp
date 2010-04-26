// VideoAnalyzerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "VideoAnalyzerDlg.h"

#include "UDPAlerter.h"
#include "Algorithm/ParabolaWarpper.h"
#include "Algorithm/PersonWarpper.h"
#include "Algorithm/Macro.h"
#include "Option.h"
#include "Common.h"

#include "DlgSetting.h"
#include "DlgPersonCfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern int g_debug;

extern struct ParamStruct ParamSet;
extern struct ParamDistinguish ParamDsting;

extern struct TPersonDetect g_personParam;

extern struct TCommonParam g_commParam;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CVideoAnalyzerDlg �Ի���




CVideoAnalyzerDlg::CVideoAnalyzerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoAnalyzerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pVideoGraber = NULL;
    m_pAnalyzer = NULL;
    m_pDlgCfg = NULL;
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
END_MESSAGE_MAP()


// CVideoAnalyzerDlg ��Ϣ�������

BOOL CVideoAnalyzerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

    //�б��
    CRect rectListStatus;
    m_lstStatus.GetClientRect(&rectListStatus);
    m_lstStatus.InsertColumn(COL_ID_TIME,   _T("ʱ��"), LVCFMT_LEFT, 110);
    m_lstStatus.InsertColumn(COL_ID_STATUS, _T("������Ϣ"), LVCFMT_LEFT, 
        rectListStatus.Width() - 130);
    m_lstStatus.SetExtendedStyle(m_lstStatus.GetExtendedStyle()|LVS_EX_GRIDLINES );

    //�б��˵�
    m_menu.LoadMenu(IDR_MENU_LIST);

    //ѡ��
    g_debug = 0;
    m_chkDebug.SetCheck(g_debug);//�Ķ�
    m_chkPreview.SetCheck(1);
    
    //ѡ����Ƶ����
    CString strWidth, strHeight, strFR;
    strWidth.Format("%d", m_iWidth);
    strHeight.Format("%d", m_iHeight);
    strFR.Format("%d", 100);
    m_edtWidth.SetWindowText(strWidth);
    m_edtHeight.SetWindowText(strHeight);
    m_edtFrameRate.SetWindowText(strFR);
    m_ctlVideo.SetWindowPos(NULL, 0, 0, m_iWidth, m_iHeight, SWP_NOMOVE | SWP_NOZORDER);

    //ѡ�������
    OnCbnDropdownComboCamera();

    //ѡ�������
    this->ResetAnalyzer();
    m_cbAnalyzer.SetCurSel(0);
    this->OnCbnSelchangeComboAyalyzer();

    //��ȡ�������·��
    GetModuleFileName(NULL,m_strAppPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);   
    m_strAppPath.ReleaseBuffer();   
    int iPos2;   
    iPos2 = m_strAppPath.ReverseFind('\\');   
    m_strAppPath = m_strAppPath.Left(iPos2);   

    //�����ļ���Ĭ���ļ����뱾�����������ͬ����׺Ϊxml
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

    this->AddRunStatus("��������");

    this->autoStart();

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CVideoAnalyzerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CVideoAnalyzerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CVideoAnalyzerDlg::OnMsgVideoEnd(WPARAM wParam,LPARAM lParam)
{
    delete m_pVideoGraber;
    m_pVideoGraber = NULL;

    m_timer.stop();
    this->AddRunStatus("�ѹرգ���ʱ%f��", m_timer.getElapsedTimeInSec());

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

    CString str;
    str.Format("%u", m_uCurrentFrame);
    m_txtCurrentFrame.SetWindowText(str);

    m_ctlVideo.showImage(pFrame);
}

bool CVideoAnalyzerDlg::CreateMultipleDirectory(const CString& szPath)
{
    CString strDir(szPath);//���Ҫ������Ŀ¼�ַ���
    //ȷ����'\'��β�Դ������һ��Ŀ¼
    if (strDir.GetAt(strDir.GetLength()-1)!=_T('\\'))
    {
        strDir.AppendChar(_T('\\'));
    }
    std::vector<CString> vPath;//���ÿһ��Ŀ¼�ַ���
    CString strTemp;//һ����ʱ����,���Ŀ¼�ַ���
    bool bSuccess = false;//�ɹ���־
    //����Ҫ�������ַ���
    for (int i=0;i<strDir.GetLength();++i)
    {
        if (strDir.GetAt(i) != _T('\\')) 
        {//�����ǰ�ַ�����'\\'
            strTemp.AppendChar(strDir.GetAt(i));
        }
        else 
        {//�����ǰ�ַ���'\\'
            vPath.push_back(strTemp);//����ǰ����ַ�����ӵ�������
            strTemp.AppendChar(_T('\\'));
        }
    }

    //�������Ŀ¼������,����ÿ��Ŀ¼
    std::vector<CString>::const_iterator vIter;
    for (vIter = vPath.begin(); vIter != vPath.end(); vIter++) 
    {
        //���CreateDirectoryִ�гɹ�,����true,���򷵻�false
        bSuccess = CreateDirectory(*vIter, NULL) ? true : false;   
        if (!bSuccess)
        {
            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                bSuccess = true;
            }
        }
    }

    return bSuccess;
}

void CVideoAnalyzerDlg::alert(const IplImage *pFrame)
{
    m_uAlert++;
    CString str;
    str.Format("%d", m_uAlert);
    m_txtAlert.SetWindowText(str);

    SYSTEMTIME Systemtime ;
    GetLocalTime(&Systemtime);
    
    //����Ŀ¼
    CString strPath;
    strPath.Format(_T("%s\\%d��%d��"),g_commParam.szImagePath,Systemtime.wMonth,Systemtime.wDay);
//     if (!CreateDirectory(strPath, NULL))
    if(!CreateMultipleDirectory(strPath))
    {
        this->AddRunStatus("����Ŀ¼ʧ�ܣ�%s", LPCTSTR(strPath));
        return;
    }
    
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

    //����ͼƬ
    CString strFile, strChannel;
    m_cbChannel.GetWindowText(strChannel);
    strFile.Format("%s/%d��%d��/%d_%d_%d_%d.jpg",
        g_commParam.szImagePath,
        Systemtime.wMonth,Systemtime.wDay,
        iChannel,
        Systemtime.wHour, Systemtime.wMinute, Systemtime.wSecond );
//     cvFlip(pFrame, NULL, 0);//��ֱ����

    try
    {
        (void)cvSaveImage((LPCTSTR)strFile, pFrame);
    }
    catch (cv::Exception)
    {
        this->AddRunStatus("����ͼƬ����");
    }

    //�б����ʾ
    this->AddRunStatus("��%d֡��������(%s_%d_%d_%d.jpg)", 
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
    // �Ի���nResourceID�ֳ����������֣����bExpand��ֵΪTRUE
    // �Ի���������ʾ������Ի�����ʾ�ϰ벿�֡�
    static CRect rcLarge;
    static CRect rcSmall;
    CString sExpand;
    //��ʼʱ���Ի���ֻ��ʾ�ϰ벿��
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
        //��չ�Ի������ߴ�
        SetWindowPos(NULL, 0, 0, rcLarge.Width(), rcLarge.Height(),
            SWP_NOMOVE | SWP_NOZORDER);
        //EnableVisibleChildren();
    }
    else
    {
        //ֻ��ʾ�Ի�����ϰ벿��
        SetWindowPos(NULL, 0, 0, rcSmall.Width(), rcSmall.Height(),
            SWP_NOMOVE | SWP_NOZORDER);
        //EnableVisibleChildren();
    } 
}

void CVideoAnalyzerDlg::EnableVisibleChildren()
{
    //ȥ��û����ʾ�ĶԻ���Ŀؼ��Ĺ��ܺͿ�ݼ���
    //�õ���һ������
    CWnd *pWndCtl = GetWindow (GW_CHILD);
    CRect rcTest;
    CRect rcControl;
    CRect rcShow;
    //�õ��Ի�����������ο�
    GetWindowRect(rcShow);
    while (pWndCtl != NULL)
    { //�õ���ǰ��ʾ�ĶԻ���ľ��γߴ�
        pWndCtl->GetWindowRect (rcControl);
        if (rcTest.IntersectRect (rcShow, rcControl))
            pWndCtl->EnableWindow(TRUE);
        else
            pWndCtl->EnableWindow(FALSE);
        //�õ��ڶ������ο�
        pWndCtl = pWndCtl->GetWindow (GW_HWNDNEXT);
    }
} 

bool CVideoAnalyzerDlg::autoStart()
{
    COption& option = COption::Instance();

    //ָ��xml�����ļ�
    const char* szXMLFile = option.getXMLFile();
    if (szXMLFile)
    {
        m_cbConfigFile.InsertString(0, szXMLFile);
        m_cbConfigFile.SetCurSel(0);
    }

    //ָ����ƵԴ
    bool bAutoStart = false;
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

    //������ģʽ�������ô��ڴ�С�ʹ���λ��
    const char* pszSimple = option.getSimple();
    int iWidth = -1, iHeight = -1, iPosX = -1, iPosY = -1;
    if (pszSimple)
    {
        sscanf(pszSimple, "%dx%d;(%d,%d)", &iWidth, &iHeight, &iPosX, &iPosY);

        //���ڴ�С
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

        //����λ��
        if (iPosX >= 0 && iPosY >= 0)
        {
            SetWindowPos(NULL, iPosX, iPosY, 0, 0, SWP_NOSIZE |SWP_NOZORDER);
        }

        //ȥ��������
        SetWindowLong(m_hWnd, GWL_STYLE, WS_VISIBLE);
        //ȥ��û����ʾ�ĶԻ���Ŀؼ��Ĺ��ܺͿ�ݼ���
        //�õ���һ������
        CWnd *pWndCtl = NULL;
        CRect rcTest;
        CRect rcControl;
        CRect rcVideo;
        m_ctlVideo.GetWindowRect(rcVideo);
        pWndCtl = m_ctlVideo.GetWindow (GW_HWNDNEXT);
        while (pWndCtl != NULL)
        {
            pWndCtl->GetWindowRect (rcControl);
            if (rcTest.IntersectRect (rcVideo, rcControl)) //��ȡ���ߵ��ص�����������ص�����Ϊ0
            {
                pWndCtl->ShowWindow(FALSE);
            }

            pWndCtl = pWndCtl->GetWindow (GW_HWNDNEXT);
        }

        //��ʱ
        GetDlgItem(IDC_BUTTON_OPEN_FILE)->ShowWindow(FALSE);
        m_cbCamera.ShowWindow(FALSE);
    }

    //������Ƶ�ֱ���
    CString strWidth, strHeight;
    strWidth.Format("%d", m_iWidth);
    strHeight.Format("%d", m_iHeight);
    m_edtWidth.SetWindowText(strWidth);
    m_edtHeight.SetWindowText(strHeight);

    //�����������������ʾԤ��
    if (bAutoStart)
    {
        if(this->openSource(m_tSource))
        {
            // -p ��ʾԤ��
            if (!option.isPreview())
            {
                m_chkPreview.SetCheck(BST_UNCHECKED);
                ExpandDialog (IDC_VIDEO, m_chkPreview.GetCheck());
            }

            //����openSource�л������Ƶ������������ͼƬ�ؼ��Ĵ�С
            //����������Ҫ�������������ͼƬ�ؼ��Ĵ�С�Ļ���
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

    //����ƵԴ
    CString strWndCaption = "VideoAnalyzer";
    bool bResult = false;
    if (TYPE_CAMERA == tSource.eType)
    {
        CString tmp;
        tmp.Format("Camera %d ", tSource.iCamID);
		strWndCaption = tmp + strWndCaption; 
        //strWndCaption += tmp;
        SetWindowText(strWndCaption);

        this->AddRunStatus("������� %d", tSource.iCamID);
        m_pVideoGraber = new CCameraWarpper(this);
        bResult = m_pVideoGraber->open(tSource.iCamID);
    } 
    else // TYPE_FILE == tSource.eType
    {
        tSource.iCamID = -1;
        strWndCaption += " " + tSource.strFileName;
        SetWindowText(strWndCaption);

        this->AddRunStatus("����Ƶ�ļ�%s", (LPCTSTR)tSource.strFileName);
        m_pVideoGraber = new CVideoFile(this);
        bResult = m_pVideoGraber->open((LPCTSTR)tSource.strFileName);
    }

    if (!bResult)
    {
        this->AddRunStatus("����ƵԴʧ��");
        delete m_pVideoGraber;
        m_pVideoGraber = NULL;
        return false;
    }

    //������Ƶ��ʾ
    m_pVideoGraber->addListener(this);

    //������Ƶ����
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

    //������Ƶ�����������öԻ����е�����
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
            m_btnPause.SetWindowText("����");
            m_bPause = true;
            m_uCurrentFrame = (unsigned int)(dStartFrame-1);
        }
        else
        {
            m_btnPause.SetWindowText("��ͣ");
            m_bPause = false;
        }
    }

    m_uAlert = 0;
    m_txtAlert.SetWindowText("0");

    //������Ƶ������
    ResetAnalyzer();
    this->OnCbnSelchangeComboAyalyzer();
    if(m_pAnalyzer)
    {
        m_pVideoGraber->addListener(m_pAnalyzer);
    }
    
    //ͨ����
    int iChannel = 0;
    if (TYPE_CAMERA == m_tSource.eType)
    {
        iChannel = m_cbCamera.GetCurSel();
    }
    else
    {
        iChannel = m_cbChannel.GetCurSel();
    }

    //����
    if (NULL == m_pHeartBeat)
    {
        m_pHeartBeat = new CHeartBeat;
    }
    if (!m_pHeartBeat->init(iChannel, 
                            g_commParam.szLocalAddr, 
                            g_commParam.szUDPServerIP, 
                            g_commParam.iUDPServerPort, 
                            g_commParam.iHeartBeat))
    {
        this->AddRunStatus("��ʼ��������Ϣʧ��");
        return false;
    }
    m_pHeartBeat->run();

    //���ñ�����
    if (m_pAnalyzer && NULL == m_pUDPAlerter)
    {
        this->AddRunStatus("�������ģ�%d.%d.%d.%d:%d", 
            g_commParam.szUDPServerIP[0], g_commParam.szUDPServerIP[1], 
            g_commParam.szUDPServerIP[2], g_commParam.szUDPServerIP[3], g_commParam.iUDPServerPort);

        CUDPAlerter* pUDPAlerter = new CUDPAlerter ;
        int iAlarmType = 1;

        (void)pUDPAlerter->init(iAlarmType, 
                                iChannel, 
                                g_commParam.szLocalAddr, 
                                g_commParam.szUDPServerIP, 
                                g_commParam.iUDPServerPort);

        m_pUDPAlerter = pUDPAlerter;
    }
    if (m_pAnalyzer)
    {
        m_pAnalyzer->addListener(m_pUDPAlerter);
    }

    INIT_IMAGE();
    this->AddRunStatus("���ڲ���");

    m_timer.start();
    m_pVideoGraber->loopGetFrame();

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
    CFileDialog	FileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY, "�����ļ� (*.*)|*.*||");
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
        this->AddRunStatus("�ѹر�");
    } 
    else
    {
        this->AddRunStatus("���ڹر�");
        this->closeSource();
        m_eVideoCtrl = VC_CLOSE;
    }
}


void CVideoAnalyzerDlg::OnBnClickedButtonRestart()
{
    if (!m_pVideoGraber || m_pVideoGraber->getStatus() == STAT_STOPPED)
    {
        this->openSource(m_tSource);
        m_eVideoCtrl = VC_OPEN;
    } 
    else
    {
        m_btnRestart.EnableWindow(FALSE);
        this->closeSource();
        m_eVideoCtrl = VC_RESTART;
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonPause()
{
    if (m_pVideoGraber)
    {
        m_pVideoGraber->setProperty(PROP_STEP, 0);
        if (m_bPause)
        {
            m_pVideoGraber->resume();
            m_btnPause.SetWindowText("��ͣ");
            m_bPause = false;
        } 
        else
        {
            m_pVideoGraber->pause();
            m_btnPause.SetWindowText("����");
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
    else//�Ѿ�ֹͣ
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

        for (unsigned i = 0; i < m_analyzerMgr.size(); i++)
        {
            delete m_analyzerMgr[i].pAnalyzer;
            delete m_analyzerMgr[i].pDlgCfg;
        }
        m_analyzerMgr.clear();

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

void CVideoAnalyzerDlg::ResetAnalyzer()
{
    int iSel = m_cbAnalyzer.GetCurSel();
    for (unsigned i = 0; i < m_analyzerMgr.size(); i++)
    {
        delete m_analyzerMgr[i].pAnalyzer;
        delete m_analyzerMgr[i].pDlgCfg;
    }
    m_analyzerMgr.clear();
    m_cbAnalyzer.ResetContent();

    m_analyzerMgr.push_back(CAnalyzerMgr(new CPersonWarpper, new CDlgPersonCfg, "��Ա���"));
    m_analyzerMgr.push_back(CAnalyzerMgr(new CParabolaWarpper, new CDlgSetting, "������"));
    m_analyzerMgr.push_back(CAnalyzerMgr(NULL, NULL, "��"));
    for (unsigned i = 0; i < m_analyzerMgr.size(); i++)
    {
        m_cbAnalyzer.InsertString(i, m_analyzerMgr[i].pComment);
    }
    m_cbAnalyzer.SetCurSel(iSel);
}

void CVideoAnalyzerDlg::OnCbnSelchangeComboAyalyzer()
{
    if (m_pVideoGraber)
    {
        m_pVideoGraber->removeListener(m_pAnalyzer);
    }

    int iSel = m_cbAnalyzer.GetCurSel();
    if (iSel+1 > (int)m_analyzerMgr.size())
    {
        AfxMessageBox("��Ч�ķ�����");
        return;
    }

    m_pAnalyzer = m_analyzerMgr[iSel].pAnalyzer;
    m_pDlgCfg = m_analyzerMgr[iSel].pDlgCfg;

    if (m_pAnalyzer)
    {
        m_pAnalyzer->addListener(this);
        if (m_pVideoGraber)
        {
            m_pVideoGraber->addListener(m_pAnalyzer);
        }
    }
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
    if ( !this->loadConfig() )
    {
        AfxMessageBox("������������ʧ��");
    }

    CDlgCfgBase* pDlgCfg = dynamic_cast<CDlgCfgBase*>(m_pDlgCfg);
    if (NULL == pDlgCfg)
    {
        AfxMessageBox("��������ô���");
        return;
    }
    pDlgCfg->setImage(m_ctlVideo.getImage());
    if (m_pDlgCfg->DoModal() != IDOK)
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
        this->AddRunStatus("����ʧ�ܣ�%s", (LPCTSTR)strConfigFile);
        return false;
    }

    if(::loadCommonParam((LPCTSTR)strConfigFile))
    {
        return false;
    }

    //����
    if (!m_cfgParse.LoadChannel(iChannel, ParamSet, ParamDsting))
    {
        this->AddRunStatus("����ͨ��%d������ʧ��", iChannel);
        return false;
    }

    //��Ա���
    if (!m_cfgParse.LoadChannel(iChannel, g_personParam))
    {
        this->AddRunStatus("����ͨ��%d������ʧ��", iChannel);
        return false;
    }

    return true;
}

bool CVideoAnalyzerDlg::saveConfig()
{
    //���������xml
    CString strConfigFile;
    CString strChannel;
    int iChannel = 0;
    m_cbConfigFile.GetWindowText(strConfigFile);
    m_cbChannel.GetWindowText(strChannel);
    strConfigFile = m_strAppPath + "\\" + strConfigFile;
    iChannel = atoi((LPCTSTR)strChannel);

    //����
    if (!m_cfgParse.SaveChannel(iChannel, ParamSet, ParamDsting))
    {
        this->AddRunStatus("����ͨ��%d������ʧ��", iChannel);
        return false;
    }

    //��Ա���
    if (!m_cfgParse.SaveChannel(iChannel, g_personParam))
    {
        this->AddRunStatus("����ͨ��%d������ʧ��", iChannel);
        return false;
    }

    if (!m_cfgParse.Save((LPCTSTR)strConfigFile))
    {
        this->AddRunStatus("����ʧ�ܣ�%s", (LPCTSTR)strConfigFile);
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
        "��Ƶ�ļ� (*.avi)|*.avi||");
    if (FileDlg.DoModal() == IDOK)	
    {
        m_edtRecPath.SetWindowText(FileDlg.GetPathName());
    }
}

void CVideoAnalyzerDlg::OnBnClickedButtonRecStart()
{
    if (m_bRecord) //����¼��ֹͣ
    {
        this->stopRecord();

        m_bRecord = false;
        m_edtRecPath.EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_REC_OPEN)->EnableWindow(TRUE);
        GetDlgItem(IDC_BUTTON_REC_START)->SetWindowText("��ʼ");
        this->AddRunStatus("¼��ֹͣ");
    } 
    else //û��¼����ʼ¼��
    {
        CString strRecPath;
        m_edtRecPath.GetWindowText(strRecPath);
        if (strRecPath.IsEmpty())
        {
            AfxMessageBox("�ļ���Ϊ��");
            return;
        }

        if (!this->startRecord((LPCTSTR)strRecPath))
        {
            return;
        }
        m_bRecord = true;
        m_edtRecPath.EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_REC_OPEN)->EnableWindow(FALSE);
        GetDlgItem(IDC_BUTTON_REC_START)->SetWindowText("ֹͣ");
        this->AddRunStatus("¼��ʼ��%s", (LPCSTR)strRecPath);
    }
}

bool CVideoAnalyzerDlg::startRecord(const char* szFileName)
{
    if (!m_pVideoGraber)
    {
        this->AddRunStatus("����¼��ʧ�ܣ���Ƶδ����");
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
        this->AddRunStatus("����¼��ʧ��");
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

// E:\code\VideoAnalyzer\src\VideoAnalyzer\DlgSetting.cpp : implementation file
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "VideoAnalyzer\DlgSetting.h"
#include "Algorithm/CParabolaDetect.h"

extern struct ParamStruct ParamSet;
extern struct ParamDistinguish ParamDsting;
// CDlgSetting dialog

IMPLEMENT_DYNAMIC(CDlgSetting, CDialog)

CDlgSetting::CDlgSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetting::IDD, pParent)
	, m_edit_two_value(0)
	, m_edit_filter_value(0)
	, m_edit_imdilate_value(0)
	, m_edit_white_spot_max(0)
	, m_edit_white_min(0)
	, m_edit_night_thr(0)
	, m_edit_max_frame(0)
{
	m_pImage = NULL;
}

CDlgSetting::~CDlgSetting()
{
    for (unsigned i = 0; i < COMBOBOX_ITEM_NUM; i++)
    {
        m_polyLineArrays[i].clear();
    }

    if (m_pImage)
    {
        cvReleaseImage(&m_pImage);
    }
}

void CDlgSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMAGE, m_ctrlImage);
	DDX_Text(pDX, IDC_EDIT_TWO_VALUE, m_edit_two_value);
	DDX_Text(pDX, IDC_EDIT_FILTER_VALUE, m_edit_filter_value);
	DDX_Text(pDX, IDC_EDIT_IMDILATE_VALUE, m_edit_imdilate_value);
	DDX_Text(pDX, IDC_EDIT_WHITE_SPOT_MAX, m_edit_white_spot_max);
	DDX_Text(pDX, IDC_EDIT_WHITE_SPOT_MIN, m_edit_white_min);
	DDX_Text(pDX, IDC_EDIT_NIGHT_THR, m_edit_night_thr);
	DDX_Text(pDX, IDC_EDIT_MAX_FRAME, m_edit_max_frame);
}


BEGIN_MESSAGE_MAP(CDlgSetting, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CDlgSetting::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CDlgSetting::OnBnClickedButtonSave)
	ON_CBN_SELCHANGE(IDC_COMBO_CONTROL, &CDlgSetting::OnCbnSelchangeComboControl)
	ON_BN_CLICKED(IDC_CHECK_LENS, &CDlgSetting::OnBnClickedCheckLens)
	ON_BN_CLICKED(IDC_RADIO_NATURAL, &CDlgSetting::OnBnClickedRadioNatural)
	ON_BN_CLICKED(IDC_RADIO_TREE, &CDlgSetting::OnBnClickedRadioTree)
	ON_BN_CLICKED(IDC_RADIO_CURVER, &CDlgSetting::OnBnClickedRadioCurver)
	ON_BN_CLICKED(IDC_RADIO_NATUARL_CURVER, &CDlgSetting::OnBnClickedRadioNatuarlCurver)
	ON_BN_CLICKED(IDC_RADIO_NATUARL_NATUARL, &CDlgSetting::OnBnClickedRadioNatuarlNatuarl)
	ON_BN_CLICKED(IDC_RADIO_TREE_CURVER, &CDlgSetting::OnBnClickedRadioTreeCurver)
	ON_BN_CLICKED(IDC_RADIO_DEPTH1, &CDlgSetting::OnBnClickedRadioDepth1)
	ON_BN_CLICKED(IDC_RADIO_DEPTH2, &CDlgSetting::OnBnClickedRadioDepth2)
	ON_BN_CLICKED(IDC_RADIO_DEPTH3, &CDlgSetting::OnBnClickedRadioDepth3)
	ON_BN_CLICKED(IDC_RADIO_SENSITIVE1, &CDlgSetting::OnBnClickedRadioSensitive1)
	ON_BN_CLICKED(IDC_RADIO_SENSITIVE2, &CDlgSetting::OnBnClickedRadioSensitive2)
	ON_BN_CLICKED(IDC_RADIO_SENSITIVE3, &CDlgSetting::OnBnClickedRadioSensitive3)
	ON_EN_CHANGE(IDC_EDIT_TWO_VALUE, &CDlgSetting::OnEnChangeEditTwoValue)
//	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgSetting::OnBnClickedButtonCancel)
	ON_BN_CLICKED(IDC_CHECK_PERSON, &CDlgSetting::OnBnClickedCheckPerson)
    ON_BN_CLICKED(IDOK, &CDlgSetting::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgSetting message handlers

BOOL CDlgSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ctrlImage.ShowImage(m_pImage);
	m_ctrlImage.SetGraphicsColor(RGB(255, 0, 0));

// 	m_ctrlImage.setPolyLineArray(m_polyLineArrays[0]);
// 	m_ctrlImage.setRectArray(m_testRectArray);
    this->LoadComboboxData();

	m_ComboChangeNum = 0 ;	

	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("A.黑框(5)"));  //0
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("A.遮挡线左")); //1
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("A.遮挡线右")); //2

	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("B.越界线左")); //3
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("B.越界线右")); //4
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("B.拟合曲线")); //5

	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("C.远景区域")); //6
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("D.树间区域")); //7
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("E.人区域左线")); //8
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("E.人区域右线")); //9
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("E.人区域高线")); //10
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("F.夜间区域1")); //11
	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("F.夜间区域2")); //12


	((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(0); //! 

    OnCbnSelchangeComboControl();

   switch (ParamSet.iStyleChange)
   {
   case 0: 
	   ((CButton *)GetDlgItem(IDC_RADIO_NATURAL))->SetCheck(TRUE);
	   break ;
   case 1: 
	   ((CButton *)GetDlgItem(IDC_RADIO_TREE))->SetCheck(TRUE);
	   break ;

   case 2: 
	   ((CButton *)GetDlgItem(IDC_RADIO_CURVER))->SetCheck(TRUE);
	   break ;

   case 3: 
	   ((CButton *)GetDlgItem(IDC_RADIO_NATUARL_CURVER))->SetCheck(TRUE);
	   break ;

   case 4: 
	   ((CButton *)GetDlgItem(IDC_RADIO_NATUARL_NATUARL))->SetCheck(TRUE);
	   break ;
   case 5: 
	   ((CButton *)GetDlgItem(IDC_RADIO_TREE_CURVER))->SetCheck(TRUE);
	   break ;
   default:
	   break;

   }

	if (ParamSet.iPersonFlag)
	{
		((CButton *)GetDlgItem(IDC_CHECK_PERSON))->SetCheck(TRUE);
	}

	if (ParamSet.bTransLensImage)
	{
		((CButton *)GetDlgItem(IDC_CHECK_LENS))->SetCheck(TRUE);
	}

	if (ParamSet.bSensitiveFlag)
	{
		((CButton *)GetDlgItem(IDC_RADIO_SENSITIVE1))->SetCheck(TRUE);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_SENSITIVE2))->SetCheck(TRUE);
	}

	if (ParamSet.iPersonWhitePotNum < 100 )
	{
		((CButton *)GetDlgItem(IDC_RADIO_DEPTH3))->SetCheck(TRUE);
	}
	else if (ParamSet.iPersonWhitePotNum >= 300 )
	{
		((CButton *)GetDlgItem(IDC_RADIO_DEPTH1))->SetCheck(TRUE);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_DEPTH2))->SetCheck(TRUE);
	}
   
	m_edit_two_value = ParamSet.iBinarizeSubThreshold ;
	m_edit_filter_value = ParamSet.iImfilterSingleThreshold;
	m_edit_imdilate_value = ParamSet.iImdilateThreshold;
	m_edit_white_spot_max = ParamSet.iWhiteSpotNumMax ;
	m_edit_white_min = ParamSet.iWhiteSpotNumMin ;
	m_edit_night_thr = ParamSet.iNightSubThreshold ;
	m_edit_max_frame = ParamSet.iTrackMaxFrameNum ;

	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetting::setImage(const IplImage *pImage)
{
    if (pImage)
    {
        m_pImage = cvCloneImage(pImage);
    }
    else
    {
        m_pImage = NULL;
    }
}

void CDlgSetting::OnBnClickedButtonClear()
{
	m_ctrlImage.Refresh();
// 	m_testPolyLineArray.clear();
    m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
    m_polyLineArrays[m_ComboChangeNum].clear();
    
    m_ctrlImage.setPolyLineArray(m_polyLineArrays[m_ComboChangeNum]);
    m_ctrlImage.ShowImage(m_pImage);

}

void CDlgSetting::LoadComboboxData()
{
    for (unsigned i = 0; i < COMBOBOX_ITEM_NUM; i++)
    {
        m_polyLineArrays[i].clear();
    }

    for (unsigned i = 0 ; i < 5 ; i++)
    {
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tRectBlackBlock[i].BeginPointX,
                                ParamSet.tRectBlackBlock[i].BeginPointY));
        plLine.push_back(CPoint(ParamSet.tRectBlackBlock[i].EndPointX,
                                ParamSet.tRectBlackBlock[i].EndPointY));

        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            continue;
        }
        m_polyLineArrays[0].push_back(plLine);
    }

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineBlackLeft.BeginPointX,
                                ParamSet.tLineBlackLeft.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineBlackLeft.EndPointX,
                                ParamSet.tLineBlackLeft.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[1].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineBlackRight.BeginPointX,
                                ParamSet.tLineBlackRight.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineBlackRight.EndPointX,
                                ParamSet.tLineBlackRight.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[2].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineStraightFirst.BeginPointX,
                                ParamSet.tLineStraightFirst.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineStraightFirst.EndPointX,
                                ParamSet.tLineStraightFirst.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[3].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineStraightSecond.BeginPointX,
                                ParamSet.tLineStraightSecond.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineStraightSecond.EndPointX,
                                ParamSet.tLineStraightSecond.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[4].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineCurverRange.BeginPointX,
                                ParamSet.tLineCurverRange.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineCurverRange.EndPointX,
                                ParamSet.tLineCurverRange.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[5].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tRectLittleRegion.BeginPointX,
                                ParamSet.tRectLittleRegion.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tRectLittleRegion.EndPointX,
                                ParamSet.tRectLittleRegion.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[6].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tRectTreeLittleRegion.BeginPointX,
                                ParamSet.tRectTreeLittleRegion.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tRectTreeLittleRegion.EndPointX,
                                ParamSet.tRectTreeLittleRegion.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[7].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[2].BeginPointX,
                                ParamSet.iPersonRange[2].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[2].EndPointX,
                                ParamSet.iPersonRange[2].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[8].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[1].BeginPointX,
                                ParamSet.iPersonRange[1].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[1].EndPointX,
                                ParamSet.iPersonRange[1].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[9].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[0].BeginPointX,
                                ParamSet.iPersonRange[0].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[0].EndPointX,
                                ParamSet.iPersonRange[0].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[10].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tNightRange[0].BeginPointX,
                                ParamSet.tNightRange[0].BeginPointY));
        plLine.push_back(CPoint(ParamSet.tNightRange[0].EndPointX,
                                ParamSet.tNightRange[0].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[11].push_back(plLine);
    }while(0);

    do{
        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[1].BeginPointX,
                                ParamSet.iPersonRange[1].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[1].EndPointX,
                                ParamSet.iPersonRange[1].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[12].push_back(plLine);
    }while(0);

}

void CDlgSetting::SaveComboboxData()
{
    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[0];
        int size_num = m_testPolyLineArray.size();
        if (size_num > 5 )
        {
            MessageBox(_T("tRectBlackBlock 超出给定数 5"));
        }
        else
        {
            for (int i = 0 ; i < size_num ; i++)
            {
                ParamSet.tRectBlackBlock[i].BeginPointX = m_testPolyLineArray[i][0].x;
                ParamSet.tRectBlackBlock[i].BeginPointY = m_testPolyLineArray[i][0].y;
                ParamSet.tRectBlackBlock[i].EndPointX = m_testPolyLineArray[i][1].x;
                ParamSet.tRectBlackBlock[i].EndPointY = m_testPolyLineArray[i][1].y;
                ParamSet.tRectBlackBlock[i].bFlag = 1 ;
            }
        }
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[1];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineBlackLeft 超出给定数 1"));
        }
        else if(size_num == 1)
        {

            ParamSet.tLineBlackLeft.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tLineBlackLeft.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tLineBlackLeft.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tLineBlackLeft.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tLineBlackLeft.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[2];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineBlackRight 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineBlackRight.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tLineBlackRight.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tLineBlackRight.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tLineBlackRight.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tLineBlackRight.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[3];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineStraightFirst 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineStraightFirst.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tLineStraightFirst.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tLineStraightFirst.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tLineStraightFirst.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tLineStraightFirst.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[4];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineStraightSecond 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineStraightSecond.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tLineStraightSecond.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tLineStraightSecond.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tLineStraightSecond.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tLineStraightSecond.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[5];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineCurverRange 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineCurverRange.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tLineCurverRange.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tLineCurverRange.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tLineCurverRange.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tLineCurverRange.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[6];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tRectLittleRegion 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tRectLittleRegion.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tRectLittleRegion.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tRectLittleRegion.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tRectLittleRegion.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tRectLittleRegion.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[7];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tRectTreeLittleRegion 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tRectTreeLittleRegion.BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tRectTreeLittleRegion.BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tRectTreeLittleRegion.EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tRectTreeLittleRegion.EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tRectTreeLittleRegion.bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[8];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("iPersonRange[2] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.iPersonRange[2].BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.iPersonRange[2].BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.iPersonRange[2].EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.iPersonRange[2].EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.iPersonRange[2].bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[9];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("iPersonRange[1] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.iPersonRange[1].BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.iPersonRange[1].BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.iPersonRange[1].EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.iPersonRange[1].EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.iPersonRange[1].bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[10];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("iPersonRange 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.iPersonRange[0].BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.iPersonRange[0].BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.iPersonRange[0].EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.iPersonRange[0].EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.iPersonRange[0].bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[11];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tNightRange[0] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tNightRange[0].BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tNightRange[0].BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tNightRange[0].EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tNightRange[0].EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tNightRange[0].bFlag = 1 ;			
        }		
    }

    {
        PolyLineArray& m_testPolyLineArray = m_polyLineArrays[12];
        int size_num = m_testPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tNightRange[1] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tNightRange[1].BeginPointX = m_testPolyLineArray[0][0].x;
            ParamSet.tNightRange[1].BeginPointY = m_testPolyLineArray[0][0].y;
            ParamSet.tNightRange[1].EndPointX = m_testPolyLineArray[0][1].x;
            ParamSet.tNightRange[1].EndPointY = m_testPolyLineArray[0][1].y;
            ParamSet.tNightRange[1].bFlag = 1 ;			
        }		
    }
}

void CDlgSetting::OnBnClickedButtonSave()
{
// 	// TODO: Add your control notification handler code here
// 	m_ctrlImage.Refresh() ;
// 
// 	int size_num = m_testPolyLineArray.size();
// 
// 	if (size_num == 0)
// 	{
// 		OnBnClickedButtonCancel();
// 		m_ComboChangeNum += 1;
// 		((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		return ;
// 	}
// 
// 	switch ( m_ComboChangeNum )
// 	{
// 	case 0:
// 		if (size_num > 5 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 			OnBnClickedButtonCancel();
// 
// 			for (int i = 0 ; i < size_num ; i++)
// 			{
// 				ParamSet.tRectBlackBlock[i].BeginPointX = m_testPolyLineArray[i][0].x;
// 				ParamSet.tRectBlackBlock[i].BeginPointY = m_testPolyLineArray[i][0].y;
// 				ParamSet.tRectBlackBlock[i].EndPointX = m_testPolyLineArray[i][1].x;
// 				ParamSet.tRectBlackBlock[i].EndPointY = m_testPolyLineArray[i][1].y;
// 				ParamSet.tRectBlackBlock[i].bFlag = 1 ;
// 			}
// 			
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		m_testPolyLineArray.clear();
// 
// 		break;
// 	case 1:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tLineBlackLeft.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tLineBlackLeft.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tLineBlackLeft.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tLineBlackLeft.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tLineBlackLeft.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}		
// 
// 		break;
// 	case 2:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tLineBlackRight.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tLineBlackRight.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tLineBlackRight.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tLineBlackRight.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tLineBlackRight.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 
// 		break;
// 	case 3:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 			ParamSet.tLineStraightFirst.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tLineStraightFirst.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tLineStraightFirst.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tLineStraightFirst.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tLineStraightFirst.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 
// 		break;
// 	case 4:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tLineStraightSecond.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tLineStraightSecond.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tLineStraightSecond.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tLineStraightSecond.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tLineStraightSecond.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 
// 		break;
// 	case 5:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tLineCurverRange.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tLineCurverRange.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tLineCurverRange.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tLineCurverRange.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tLineCurverRange.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 
// 		break;
// 	case 6:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tRectLittleRegion.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tRectLittleRegion.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tRectLittleRegion.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tRectLittleRegion.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tRectLittleRegion.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 	case 7:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tRectTreeLittleRegion.BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tRectTreeLittleRegion.BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tRectTreeLittleRegion.EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tRectTreeLittleRegion.EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tRectTreeLittleRegion.bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 	case 8:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.iPersonRange[2].BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.iPersonRange[2].BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.iPersonRange[2].EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.iPersonRange[2].EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.iPersonRange[2].bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 	case 9:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.iPersonRange[1].BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.iPersonRange[1].BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.iPersonRange[1].EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.iPersonRange[1].EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.iPersonRange[1].bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 	case 10:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox( _T("超出给定数") );
// 		}
// 		else
// 		{
// 
// 			ParamSet.iPersonRange[0].BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.iPersonRange[0].BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.iPersonRange[0].EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.iPersonRange[0].EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.iPersonRange[0].bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 	case 11:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tNightRange[0].BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tNightRange[0].BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tNightRange[0].EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tNightRange[0].EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tNightRange[0].bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 	case 12:
// 		if ( size_num != 1 )
// 		{
// 			MessageBox(_T("超出给定数"));
// 		}
// 		else
// 		{
// 
// 			ParamSet.tNightRange[1].BeginPointX = m_testPolyLineArray[0][0].x;
// 			ParamSet.tNightRange[1].BeginPointY = m_testPolyLineArray[0][0].y;
// 			ParamSet.tNightRange[1].EndPointX = m_testPolyLineArray[0][1].x;
// 			ParamSet.tNightRange[1].EndPointY = m_testPolyLineArray[0][1].y;
// 			ParamSet.tNightRange[1].bFlag = 1 ;			
// 
// 			m_ComboChangeNum += 1;
// 
// 			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
// 		}
// 		break;
// 
// 	default:            
// 		break;
// 	}
// 
// 	m_testPolyLineArray.clear();

}

void CDlgSetting::OnCbnSelchangeComboControl()
{
	// TODO: Add your control notification handler code here
	m_ctrlImage.Refresh() ;
	m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
    m_ctrlImage.setPolyLineArray(m_polyLineArrays[m_ComboChangeNum]);
    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgSetting::OnBnClickedCheckLens()
{
	// TODO: Add your control notification handler code here
	if( ((CButton*)GetDlgItem(IDC_CHECK_LENS))->GetCheck() == BST_UNCHECKED )
	{
		ParamSet.bTransLensImage = 0 ; 
	}
	else
	{
        ParamSet.bTransLensImage = 1 ;
	}
}

void CDlgSetting::OnBnClickedRadioNatural()
{
	// TODO: Add your control notification handler code here
	ParamSet.iStyleChange = 0 ;
}

void CDlgSetting::OnBnClickedRadioTree()
{
	// TODO: Add your control notification handler code here
	ParamSet.iStyleChange = 1 ;
}

void CDlgSetting::OnBnClickedRadioCurver()
{
	// TODO: Add your control notification handler code here
	ParamSet.iStyleChange = 2 ;
}

void CDlgSetting::OnBnClickedRadioNatuarlCurver()
{
	// TODO: Add your control notification handler code 
	ParamSet.iStyleChange = 3 ;

}

void CDlgSetting::OnBnClickedRadioNatuarlNatuarl()
{
	// TODO: Add your control notification handler code here
	ParamSet.iStyleChange = 4 ;
}

void CDlgSetting::OnBnClickedRadioTreeCurver()
{
	// TODO: Add your control notification handler code here
	ParamSet.iStyleChange = 5 ;
}

void CDlgSetting::OnBnClickedRadioDepth1()
{
	// TODO: Add your control notification handler code here
	ParamSet.iPersonWhitePotNum = 300 ;
	ParamSet.iXTrackContinueThreshold = 3 ;
	ParamSet.iWhiteSpotNumMin = 40 ;
}

void CDlgSetting::OnBnClickedRadioDepth2()
{
	// TODO: Add your control notification handler code here
	ParamSet.iPersonWhitePotNum = 200 ;
	ParamSet.iXTrackContinueThreshold = 2 ;
	ParamSet.iWhiteSpotNumMin = 30 ;
}

void CDlgSetting::OnBnClickedRadioDepth3()
{
	// TODO: Add your control notification handler code here
	ParamSet.iPersonWhitePotNum = 80 ;
	ParamSet.iXTrackContinueThreshold = 1 ;
	ParamSet.iWhiteSpotNumMin = 20 ;
}

void CDlgSetting::OnBnClickedRadioSensitive1()
{
	// TODO: Add your control notification handler code here
	ParamSet.bSensitiveFlag = 1 ;
}

void CDlgSetting::OnBnClickedRadioSensitive2()
{
	// TODO: Add your control notification handler code here
	ParamSet.bSensitiveFlag = 0 ;
}

void CDlgSetting::OnBnClickedRadioSensitive3()
{
	// TODO: Add your control notification handler code here
	ParamSet.bSensitiveFlag = 0 ;
}

void CDlgSetting::OnEnChangeEditTwoValue()
{
	// TODO:  Add your control notification handler code here
	int tempVal = m_edit_two_value;
	UpdateData(true);
	if ( m_edit_two_value >= 0 && m_edit_two_value <= 60 )
	{
        ParamSet.iBinarizeSubThreshold = m_edit_two_value ;
	}
	else
	{
		m_edit_two_value = tempVal ;
        UpdateData(false);
	}
}

//void CDlgSetting::OnClose()
//{
//	// TODO: Add your message handler code here and/or call default
//	m_pCfgParse->SaveChannel(0, ParamSet,ParamDsting);
//	m_pCfgParse->Save(m_FileName);
//	CDialog::OnClose();
//}

void CDlgSetting::OnBnClickedButtonCancel()
{
	// TODO: Add your control notification handler code here
	switch ( m_ComboChangeNum )
	{
	case 0:
		for (int i = 0 ; i < 5 ; i++)
		{
			ParamSet.tRectBlackBlock[i].BeginPointX = 0;
			ParamSet.tRectBlackBlock[i].BeginPointY = 0;
			ParamSet.tRectBlackBlock[i].EndPointX = 0;
			ParamSet.tRectBlackBlock[i].EndPointY = 0;
			ParamSet.tRectBlackBlock[i].bFlag = 0 ;
		}
		break ;
	case 1:
		ParamSet.tLineBlackLeft.BeginPointX = 0;
		ParamSet.tLineBlackLeft.BeginPointY = 0;
		ParamSet.tLineBlackLeft.EndPointX = 0;
		ParamSet.tLineBlackLeft.EndPointY = 0;
		ParamSet.tLineBlackLeft.bFlag = 0 ;
		break ;
	case 2:
		ParamSet.tLineBlackRight.BeginPointX = 0;
		ParamSet.tLineBlackRight.BeginPointY = 0;
		ParamSet.tLineBlackRight.EndPointX = 0;
		ParamSet.tLineBlackRight.EndPointY = 0;
		ParamSet.tLineBlackRight.bFlag = 0 ;
		break ;
	case 3:
		ParamSet.tLineStraightFirst.BeginPointX = 0;
		ParamSet.tLineStraightFirst.BeginPointY = 0;
		ParamSet.tLineStraightFirst.EndPointX = 0;
		ParamSet.tLineStraightFirst.EndPointY = 0;
		ParamSet.tLineStraightFirst.bFlag = 0 ;	
		break ;
	case 4:
		ParamSet.tLineStraightSecond.BeginPointX = 0;
		ParamSet.tLineStraightSecond.BeginPointY = 0;
		ParamSet.tLineStraightSecond.EndPointX = 0;
		ParamSet.tLineStraightSecond.EndPointY = 0;
		ParamSet.tLineStraightSecond.bFlag = 0 ;
		break ;
	case 5:
		ParamSet.tLineCurverRange.BeginPointX = 0;
		ParamSet.tLineCurverRange.BeginPointY = 0;
		ParamSet.tLineCurverRange.EndPointX = 0;
		ParamSet.tLineCurverRange.EndPointY = 0;
		ParamSet.tLineCurverRange.bFlag = 0 ;
		break ;
	case 6:
		ParamSet.tRectLittleRegion.BeginPointX = 0;
		ParamSet.tRectLittleRegion.BeginPointY = 0;
		ParamSet.tRectLittleRegion.EndPointX = 0;
		ParamSet.tRectLittleRegion.EndPointY = 0;
		ParamSet.tRectLittleRegion.bFlag = 0 ;	
		break ;
	case 7:
		ParamSet.tRectTreeLittleRegion.BeginPointX = 0;
		ParamSet.tRectTreeLittleRegion.BeginPointY = 0;
		ParamSet.tRectTreeLittleRegion.EndPointX = 0;
		ParamSet.tRectTreeLittleRegion.EndPointY = 0;
		ParamSet.tRectTreeLittleRegion.bFlag = 0 ;
		break ;
	case 8:
		ParamSet.iPersonRange[2].BeginPointX = 0;
		ParamSet.iPersonRange[2].BeginPointY = 0;
		ParamSet.iPersonRange[2].EndPointX = 0;
		ParamSet.iPersonRange[2].EndPointY = 0;
		ParamSet.iPersonRange[2].bFlag = 0 ;
		break ;
	case 9:
		ParamSet.iPersonRange[1].BeginPointX = 0;
		ParamSet.iPersonRange[1].BeginPointY = 0;
		ParamSet.iPersonRange[1].EndPointX = 0;
		ParamSet.iPersonRange[1].EndPointY = 0;
		ParamSet.iPersonRange[1].bFlag = 0 ;
		break ;
	case 10:
		ParamSet.iPersonRange[0].BeginPointX = 0;
		ParamSet.iPersonRange[0].BeginPointY = 0;
		ParamSet.iPersonRange[0].EndPointX = 0;
		ParamSet.iPersonRange[0].EndPointY = 0;
		ParamSet.iPersonRange[0].bFlag = 0 ;
		break ;
	case 11:
		ParamSet.tNightRange[0].BeginPointX = 0;
		ParamSet.tNightRange[0].BeginPointY = 0;
		ParamSet.tNightRange[0].EndPointX = 0;
		ParamSet.tNightRange[0].EndPointY = 0;
		ParamSet.tNightRange[0].bFlag = 0 ;
		break ;
	case 12:
		ParamSet.tNightRange[1].BeginPointX = 0;
		ParamSet.tNightRange[1].BeginPointY = 0;
		ParamSet.tNightRange[1].EndPointX = 0;
		ParamSet.tNightRange[1].EndPointY = 0;
		ParamSet.tNightRange[1].bFlag = 0 ;
		break ;
	default:            
		break;
	}
}

void CDlgSetting::OnBnClickedCheckPerson()
{
	// TODO: Add your control notification handler code here
	if( ((CButton*)GetDlgItem(IDC_CHECK_PERSON))->GetCheck() == BST_UNCHECKED )
	{
		ParamSet.iPersonFlag = 0 ; 
	}
	else
	{
		ParamSet.iPersonFlag = 1 ;
	}
}

void CDlgSetting::OnBnClickedOk()
{
//     this->OnBnClickedButtonSave();
    SaveComboboxData();
    OnOK();
}

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
END_MESSAGE_MAP()


// CDlgSetting message handlers

BOOL CDlgSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ctrlImage.ShowImage(m_pImage);
	m_ctrlImage.SetGraphicsColor(RGB(255, 0, 0));

	m_ctrlImage.setPolyLineArray(m_testPolyLineArray);
	m_ctrlImage.setRectArray(m_testRectArray);

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
	m_pImage = pImage;
}
void CDlgSetting::OnBnClickedButtonClear()
{
	m_ctrlImage.Refresh();
	m_testPolyLineArray.clear();
}

void CDlgSetting::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	m_ctrlImage.Refresh() ;

	int size_num = m_testPolyLineArray.size();

	switch ( m_ComboChangeNum )
	{
	case 0:
		if (size_num > 5 || size_num == 0)
		{
			MessageBox(_T("超出给定数"));
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
			
			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		m_testPolyLineArray.clear();

		break;
	case 1:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tLineBlackLeft.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tLineBlackLeft.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tLineBlackLeft.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tLineBlackLeft.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tLineBlackLeft.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}		

		break;
	case 2:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tLineBlackRight.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tLineBlackRight.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tLineBlackRight.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tLineBlackRight.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tLineBlackRight.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}

		break;
	case 3:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{
			ParamSet.tLineStraightFirst.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tLineStraightFirst.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tLineStraightFirst.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tLineStraightFirst.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tLineStraightFirst.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}

		break;
	case 4:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tLineStraightSecond.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tLineStraightSecond.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tLineStraightSecond.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tLineStraightSecond.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tLineStraightSecond.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}

		break;
	case 5:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tLineCurverRange.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tLineCurverRange.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tLineCurverRange.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tLineCurverRange.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tLineCurverRange.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}

		break;
	case 6:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tRectLittleRegion.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tRectLittleRegion.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tRectLittleRegion.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tRectLittleRegion.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tRectLittleRegion.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;
	case 7:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tRectTreeLittleRegion.BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tRectTreeLittleRegion.BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tRectTreeLittleRegion.EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tRectTreeLittleRegion.EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tRectTreeLittleRegion.bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;
	case 8:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.iPersonRange[0].BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.iPersonRange[0].BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.iPersonRange[0].EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.iPersonRange[0].EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.iPersonRange[0].bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;
	case 9:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.iPersonRange[1].BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.iPersonRange[1].BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.iPersonRange[1].EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.iPersonRange[1].EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.iPersonRange[1].bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;
	case 10:
		if ( size_num != 1 )
		{
			MessageBox( _T("超出给定数") );
		}
		else
		{

			ParamSet.iPersonRange[2].BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.iPersonRange[2].BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.iPersonRange[2].EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.iPersonRange[2].EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.iPersonRange[2].bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;
	case 11:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tNightRange[0].BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tNightRange[0].BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tNightRange[0].EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tNightRange[0].EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tNightRange[0].bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;
	case 12:
		if ( size_num != 1 )
		{
			MessageBox(_T("超出给定数"));
		}
		else
		{

			ParamSet.tNightRange[1].BeginPointX = m_testPolyLineArray[0][0].x;
			ParamSet.tNightRange[1].BeginPointY = m_testPolyLineArray[0][0].y;
			ParamSet.tNightRange[1].EndPointX = m_testPolyLineArray[0][1].x;
			ParamSet.tNightRange[1].EndPointY = m_testPolyLineArray[0][1].y;
			ParamSet.tNightRange[1].bFlag = 1 ;			

			m_ComboChangeNum += 1;

			((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(m_ComboChangeNum); //! 
		}
		break;

	default:            
		break;
	}

	m_testPolyLineArray.clear();

}

void CDlgSetting::OnCbnSelchangeComboControl()
{
	// TODO: Add your control notification handler code here
	m_ctrlImage.Refresh() ;
	m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
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
	if ( m_edit_two_value > 30 && m_edit_two_value < 60 )
	{
        ParamSet.iBinarizeSubThreshold = m_edit_two_value ;
	}
	else
	{
		m_edit_two_value = tempVal ;
        UpdateData(false);
	}
}

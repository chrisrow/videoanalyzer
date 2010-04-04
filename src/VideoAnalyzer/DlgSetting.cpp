// E:\code\VideoAnalyzer\src\VideoAnalyzer\DlgSetting.cpp : implementation file
//

#include "stdafx.h"
#include "VideoAnalyzer.h"
#include "VideoAnalyzer/DlgSetting.h"
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
	, m_edit_alarm(0)
	, m_edit_alarm_delay(0)
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
    //DDX_Text(pDX, IDC_EDIT_MAX_FRAME, m_edit_max_frame);
    DDX_Text(pDX, IDC_EDIT_ALARM, m_edit_alarm);
    DDX_Text(pDX, IDC_EDIT_ALARM_DELAY, m_edit_alarm_delay);
    DDX_Control(pDX, IDC_COMBO_CONTROL, m_cbGraphicsType);
}


BEGIN_MESSAGE_MAP(CDlgSetting, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CDlgSetting::OnBnClickedButtonClear)
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
    ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgSetting::OnBnClickedButtonCancel)
    ON_BN_CLICKED(IDC_CHECK_PERSON, &CDlgSetting::OnBnClickedCheckPerson)
    ON_BN_CLICKED(IDOK, &CDlgSetting::OnBnClickedOk)
    ON_EN_CHANGE(IDC_EDIT_TWO_VALUE, &CDlgSetting::OnEnChangeEditTwoValue)
    //ON_EN_CHANGE(IDC_EDIT_MAX_FRAME, &CDlgSetting::OnEnChangeEditMaxFrame)
	ON_EN_CHANGE(IDC_EDIT_WHITE_SPOT_MIN, &CDlgSetting::OnEnChangeEditWhiteSpotMin)
	ON_EN_CHANGE(IDC_EDIT_FILTER_VALUE, &CDlgSetting::OnEnChangeEditFilterValue)
	ON_EN_CHANGE(IDC_EDIT_IMDILATE_VALUE, &CDlgSetting::OnEnChangeEditImdilateValue)
	ON_EN_CHANGE(IDC_EDIT_WHITE_SPOT_MAX, &CDlgSetting::OnEnChangeEditWhiteSpotMax)
	ON_EN_CHANGE(IDC_EDIT_NIGHT_THR, &CDlgSetting::OnEnChangeEditNightThr)
	ON_EN_CHANGE(IDC_EDIT_ALARM, &CDlgSetting::OnEnChangeEditAlarm)
	ON_EN_CHANGE(IDC_EDIT_ALARM_DELAY, &CDlgSetting::OnEnChangeEditAlarmDelay)
    ON_BN_CLICKED(IDC_BUTTON_NEXT, &CDlgSetting::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_CHECK_NIGHT, &CDlgSetting::OnBnClickedCheckNight)
END_MESSAGE_MAP()


// CDlgSetting message handlers

BOOL CDlgSetting::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  Add extra initialization here
    m_ctrlImage.ShowImage(m_pImage);
    m_ctrlImage.SetGraphicsColor(RGB(255, 0, 0));

    this->LoadComboboxData();

    m_ComboChangeNum = 0 ;    

    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("A1.黑框(5)"));  //0
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("A2.遮挡线左")); //1
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("A3.遮挡线右")); //2

    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("B1.越界线左")); //3
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("B2.越界线右")); //4
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("B3.拟合曲线")); //5

    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("C1.人区域左线")); //8
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("C2.人区域右线")); //9
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("C3.人区域高线")); //10

    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("D1.夜间区域1")); //11
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("D2.夜间区域2")); //12

    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("E1.直抛区域")); //6
    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->AddString(_T("E2.直抛高度")); //7


    ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->SetCurSel(0); //! 

    SetPaintType();
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
       ((CButton *)GetDlgItem(IDC_RADIO_NATUARL_CURVER ))->SetCheck(TRUE);
       break ;

   case 3: 
       ((CButton *)GetDlgItem(IDC_RADIO_NATUARL_NATUARL))->SetCheck(TRUE);
       break ;

   case 4: 
       ((CButton *)GetDlgItem(IDC_RADIO_CURVER  ))->SetCheck(TRUE);
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

	if (ParamDsting.FindObjValMax.FirstValue == 1)
	{
		((CButton *)GetDlgItem(IDC_CHECK_NIGHT))->SetCheck(TRUE);
	}

    if (ParamSet.bTransLensImage)
    {
        ((CButton *)GetDlgItem(IDC_CHECK_LENS))->SetCheck(TRUE);
    }

    if (ParamSet.bSensitiveFlag == 0 )
    {
        ((CButton *)GetDlgItem(IDC_RADIO_SENSITIVE1))->SetCheck(TRUE);
    }
    else if(ParamSet.bSensitiveFlag == 1 )
    {
        ((CButton *)GetDlgItem(IDC_RADIO_SENSITIVE2))->SetCheck(TRUE);
    }
	else
	{
        ((CButton *)GetDlgItem(IDC_RADIO_SENSITIVE3))->SetCheck(TRUE);
	}

    if (ParamSet.iSceneDepth == 0 )
    {
        ((CButton *)GetDlgItem(IDC_RADIO_DEPTH3))->SetCheck(TRUE);
    }
    else if (ParamSet.iSceneDepth == 1 )
    {
        ((CButton *)GetDlgItem(IDC_RADIO_DEPTH2))->SetCheck(TRUE);
    }
    else
    {
        ((CButton *)GetDlgItem(IDC_RADIO_DEPTH1))->SetCheck(TRUE);
    }
   
    m_edit_two_value = ParamSet.iBinarizeSubThreshold ;
    m_edit_filter_value = ParamSet.iImfilterSingleThreshold;
    m_edit_imdilate_value = ParamSet.iImdilateThreshold;
    m_edit_white_spot_max = ParamSet.iWhiteSpotNumMax ;
    m_edit_white_min = ParamSet.iWhiteSpotNumMin ;
    m_edit_night_thr = ParamSet.iNightSubThreshold ;
    m_edit_max_frame = ParamSet.iTrackMaxFrameNum ;
	m_edit_alarm_delay = ParamSet.iAlarmDelay  ;
	m_edit_alarm = ParamDsting.FindObjValue.FirstValue ;

    UpdateData(false);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSetting::OnBnClickedButtonClear()
{
    m_ctrlImage.Refresh();

    m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
    m_polyLineArrays[m_ComboChangeNum].clear();
    m_rectArrays[m_ComboChangeNum].clear();

    OnBnClickedButtonCancel();
    
    m_ctrlImage.setPolyLineArray(m_polyLineArrays[m_ComboChangeNum]);
    m_ctrlImage.setRectArray(m_rectArrays[m_ComboChangeNum]);
    m_ctrlImage.ShowImage(m_pImage);
}
void CDlgSetting::SetPaintType()
{
    m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
    if (  m_ComboChangeNum == COMBOBOX_ITEM_RECT_BLACK_BLOCK 
        ||m_ComboChangeNum == COMBOBOX_ITEM_NIGHT_RANGE_0 
        ||m_ComboChangeNum == COMBOBOX_ITEM_NIGHT_RANGE_1
        ||m_ComboChangeNum == COMBOBOX_ITEM_RECT_LITTLE_REGION  )
    {
        m_ctrlImage.SetGraphicsType(GT_Rectangle);
    }
    else
    {
        m_ctrlImage.SetGraphicsType(GT_Polyline);
    }
}

void CDlgSetting::LoadComboboxData()
{
    for (unsigned i = 0; i < COMBOBOX_ITEM_NUM; i++)
    {
        m_polyLineArrays[i].clear();
        m_rectArrays[i].clear();
    }

    for (unsigned i = 0 ; i < 5 ; i++)
    {
        if (0 == ParamSet.tRectBlackBlock[i].bFlag)
        {
            continue;
        }

        CRect rect;
        rect.left   = ParamSet.tRectBlackBlock[i].BeginPointX;
        rect.top    = ParamSet.tRectBlackBlock[i].BeginPointY;
        rect.right  = ParamSet.tRectBlackBlock[i].EndPointX;
        rect.bottom = ParamSet.tRectBlackBlock[i].EndPointY;

        if (rect.left == 0 && rect.top == 0 && rect.right == 0  && rect.bottom == 0 )
        {
            continue;
        }
        m_rectArrays[COMBOBOX_ITEM_RECT_BLACK_BLOCK].push_back(rect);
    }

    do{
        if (0 == ParamSet.tLineBlackLeft.bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineBlackLeft.BeginPointX,
                                ParamSet.tLineBlackLeft.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineBlackLeft.EndPointX,
                                ParamSet.tLineBlackLeft.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_LINE_BLACK_LEFT].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.tLineBlackRight.bFlag)
        {
            break;
        }

       PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineBlackRight.BeginPointX,
                                ParamSet.tLineBlackRight.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineBlackRight.EndPointX,
                                ParamSet.tLineBlackRight.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_LINE_BLACK_RIGHT].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.tLineStraightFirst.bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineStraightFirst.BeginPointX,
                                ParamSet.tLineStraightFirst.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineStraightFirst.EndPointX,
                                ParamSet.tLineStraightFirst.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_LINE_STRAIGHT_FIRST].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.tLineStraightSecond.bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineStraightSecond.BeginPointX,
                                ParamSet.tLineStraightSecond.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineStraightSecond.EndPointX,
                                ParamSet.tLineStraightSecond.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_LINE_STRAIGHT_SECOND].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.tLineCurverRange.bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLineCurverRange.BeginPointX,
                                ParamSet.tLineCurverRange.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLineCurverRange.EndPointX,
                                ParamSet.tLineCurverRange.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_LINE_CURVER_RANGE].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.iPersonRange[0].bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[0].BeginPointX,
                                ParamSet.iPersonRange[0].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[0].EndPointX,
                                ParamSet.iPersonRange[0].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_PERSON_RANGE_0].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.iPersonRange[1].bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[1].BeginPointX,
                                ParamSet.iPersonRange[1].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[1].EndPointX,
                                ParamSet.iPersonRange[1].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_PERSON_RANGE_1].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.iPersonRange[2].bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.iPersonRange[2].BeginPointX,
                                ParamSet.iPersonRange[2].BeginPointY));
        plLine.push_back(CPoint(ParamSet.iPersonRange[2].EndPointX,
                                ParamSet.iPersonRange[2].EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_PERSON_RANGE_2].push_back(plLine);
    }while(0);

    do{
        if (0 == ParamSet.tNightRange[0].bFlag)
        {
            break;
        }

        CRect rect;
        rect.left   = ParamSet.tNightRange[0].BeginPointX;
        rect.top    = ParamSet.tNightRange[0].BeginPointY;
        rect.right  = ParamSet.tNightRange[0].EndPointX;
        rect.bottom = ParamSet.tNightRange[0].EndPointY;

        if (rect.left == 0 && rect.top == 0 && rect.right == 0  && rect.bottom == 0 )
        {
            continue;
        }
        m_rectArrays[COMBOBOX_ITEM_NIGHT_RANGE_0].push_back(rect);
    }while(0);

    do{
        if (0 == ParamSet.tNightRange[1].bFlag)
        {
            break;
        }

        CRect rect;
        rect.left   = ParamSet.tNightRange[1].BeginPointX;
        rect.top    = ParamSet.tNightRange[1].BeginPointY;
        rect.right  = ParamSet.tNightRange[1].EndPointX;
        rect.bottom = ParamSet.tNightRange[1].EndPointY;

        if (rect.left == 0 && rect.top == 0 && rect.right == 0  && rect.bottom == 0 )
        {
            continue;
        }
        m_rectArrays[COMBOBOX_ITEM_NIGHT_RANGE_1].push_back(rect);
    }while(0);

    do{
        if (0 == ParamSet.tRectLittleRegion.bFlag)
        {
            break;
        }

        CRect rect;
        rect.left   = ParamSet.tRectLittleRegion.BeginPointX;
        rect.top    = ParamSet.tRectLittleRegion.BeginPointY;
        rect.right  = ParamSet.tRectLittleRegion.EndPointX;
        rect.bottom = ParamSet.tRectLittleRegion.EndPointY;

        if (rect.left == 0 && rect.top == 0 && rect.right == 0  && rect.bottom == 0 )
        {
            continue;
        }
        m_rectArrays[COMBOBOX_ITEM_RECT_LITTLE_REGION].push_back(rect);
    }while(0);

    do{
        if (0 == ParamSet.tLittleRegionYLine.bFlag)
        {
            break;
        }

        PolyLine plLine;
        plLine.push_back(CPoint(ParamSet.tLittleRegionYLine.BeginPointX,
                                ParamSet.tLittleRegionYLine.BeginPointY));
        plLine.push_back(CPoint(ParamSet.tLittleRegionYLine.EndPointX,
                                ParamSet.tLittleRegionYLine.EndPointY));
        if (plLine[0].x == 0 && plLine[0].y == 0 && plLine[1].x == 0  && plLine[1].y == 0 )
        {
            break;
        }
        m_polyLineArrays[COMBOBOX_ITEM_LITTLE_REGION_Y_LINE].push_back(plLine);
    }while(0);

}

void CDlgSetting::SaveComboboxData()
{
    {
        RectArray& tmpRectArray = m_rectArrays[COMBOBOX_ITEM_RECT_BLACK_BLOCK];
        int size_num = tmpRectArray.size();
        if (size_num > 5 )
        {
            MessageBox(_T("tRectBlackBlock 超出给定数 5"));
        }
        else
        {
            for (int i = 0 ; i < size_num ; i++)
            {
                ParamSet.tRectBlackBlock[i].BeginPointX = tmpRectArray[i].left;
                ParamSet.tRectBlackBlock[i].BeginPointY = tmpRectArray[i].top;
                ParamSet.tRectBlackBlock[i].EndPointX = tmpRectArray[i].right;
                ParamSet.tRectBlackBlock[i].EndPointY = tmpRectArray[i].bottom;
                ParamSet.tRectBlackBlock[i].bFlag = 1 ;
            }
        }
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_LINE_BLACK_LEFT];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineBlackLeft 超出给定数 1"));
        }
        else if(size_num == 1)
        {

            ParamSet.tLineBlackLeft.BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.tLineBlackLeft.BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.tLineBlackLeft.EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.tLineBlackLeft.EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.tLineBlackLeft.bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_LINE_BLACK_RIGHT];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineBlackRight 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineBlackRight.BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.tLineBlackRight.BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.tLineBlackRight.EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.tLineBlackRight.EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.tLineBlackRight.bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_LINE_STRAIGHT_FIRST];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineStraightFirst 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineStraightFirst.BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.tLineStraightFirst.BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.tLineStraightFirst.EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.tLineStraightFirst.EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.tLineStraightFirst.bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_LINE_STRAIGHT_SECOND];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineStraightSecond 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineStraightSecond.BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.tLineStraightSecond.BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.tLineStraightSecond.EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.tLineStraightSecond.EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.tLineStraightSecond.bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_LINE_CURVER_RANGE];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLineCurverRange 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLineCurverRange.BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.tLineCurverRange.BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.tLineCurverRange.EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.tLineCurverRange.EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.tLineCurverRange.bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_PERSON_RANGE_0];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("iPersonRange[0] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.iPersonRange[0].BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.iPersonRange[0].BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.iPersonRange[0].EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.iPersonRange[0].EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.iPersonRange[0].bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_PERSON_RANGE_1];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("iPersonRange[1] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.iPersonRange[1].BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.iPersonRange[1].BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.iPersonRange[1].EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.iPersonRange[1].EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.iPersonRange[1].bFlag = 1 ;            
        }        
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_PERSON_RANGE_2];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("iPersonRange[2] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.iPersonRange[2].BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.iPersonRange[2].BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.iPersonRange[2].EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.iPersonRange[2].EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.iPersonRange[2].bFlag = 1 ;            
        }        
    }

    {
        RectArray& tmpRectArray = m_rectArrays[COMBOBOX_ITEM_NIGHT_RANGE_0];
        int size_num = tmpRectArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tNightRange[0] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tNightRange[0].BeginPointX = tmpRectArray[0].left;
            ParamSet.tNightRange[0].BeginPointY = tmpRectArray[0].top;
            ParamSet.tNightRange[0].EndPointX = tmpRectArray[0].right;
            ParamSet.tNightRange[0].EndPointY = tmpRectArray[0].bottom;
            ParamSet.tNightRange[0].bFlag = 1 ;
        }
    }

    {
        RectArray& tmpRectArray = m_rectArrays[COMBOBOX_ITEM_NIGHT_RANGE_1];
        int size_num = tmpRectArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tNightRange[1] 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tNightRange[1].BeginPointX = tmpRectArray[0].left;
            ParamSet.tNightRange[1].BeginPointY = tmpRectArray[0].top;
            ParamSet.tNightRange[1].EndPointX = tmpRectArray[0].right;
            ParamSet.tNightRange[1].EndPointY = tmpRectArray[0].bottom;
            ParamSet.tNightRange[1].bFlag = 1 ;
        }
    }

    {
        RectArray& tmpRectArray = m_rectArrays[COMBOBOX_ITEM_RECT_LITTLE_REGION];
        int size_num = tmpRectArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tRectLittleRegion 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tRectLittleRegion.BeginPointX = tmpRectArray[0].left;
            ParamSet.tRectLittleRegion.BeginPointY = tmpRectArray[0].top;
            ParamSet.tRectLittleRegion.EndPointX = tmpRectArray[0].right;
            ParamSet.tRectLittleRegion.EndPointY = tmpRectArray[0].bottom;
            ParamSet.tRectLittleRegion.bFlag = 1 ;
        }
    }

    {
        PolyLineArray& tmpPolyLineArray = m_polyLineArrays[COMBOBOX_ITEM_LITTLE_REGION_Y_LINE];
        int size_num = tmpPolyLineArray.size();
        if ( size_num > 1 )
        {
            MessageBox(_T("tLittleRegionYLine 超出给定数 1"));
        }
        else if(size_num == 1)
        {
            ParamSet.tLittleRegionYLine.BeginPointX = tmpPolyLineArray[0][0].x;
            ParamSet.tLittleRegionYLine.BeginPointY = tmpPolyLineArray[0][0].y;
            ParamSet.tLittleRegionYLine.EndPointX = tmpPolyLineArray[0][1].x;
            ParamSet.tLittleRegionYLine.EndPointY = tmpPolyLineArray[0][1].y;
            ParamSet.tLittleRegionYLine.bFlag = 1 ;            
        }        
    }
}

void CDlgSetting::OnCbnSelchangeComboControl()
{
    m_ctrlImage.Refresh() ;
    SetPaintType();
    m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
    m_ctrlImage.setPolyLineArray(m_polyLineArrays[m_ComboChangeNum]);
    m_ctrlImage.setRectArray(m_rectArrays[m_ComboChangeNum]);
    m_ctrlImage.ShowImage(m_pImage);
}

void CDlgSetting::OnBnClickedCheckLens()
{
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
    ParamSet.iStyleChange = 0 ;
}

void CDlgSetting::OnBnClickedRadioTree()
{
    ParamSet.iStyleChange = 1 ;
}

void CDlgSetting::OnBnClickedRadioCurver()
{
    ParamSet.iStyleChange = 5 ;
}

void CDlgSetting::OnBnClickedRadioNatuarlCurver()
{
    ParamSet.iStyleChange = 2 ;
}

void CDlgSetting::OnBnClickedRadioNatuarlNatuarl()
{
    ParamSet.iStyleChange = 3 ;
}

void CDlgSetting::OnBnClickedRadioTreeCurver()
{
    ParamSet.iStyleChange = 6 ;
}

void CDlgSetting::OnBnClickedRadioDepth1()
{
	ParamSet.iSceneDepth = 2 ;
    ParamSet.iPersonWhitePotNum = 300 ;
    ParamSet.iXTrackContinueThreshold = 3 ;
    ParamSet.iWhiteSpotNumMin = 40 ;
}

void CDlgSetting::OnBnClickedRadioDepth2()
{
	ParamSet.iSceneDepth = 1 ;
    ParamSet.iPersonWhitePotNum = 200 ;
    ParamSet.iXTrackContinueThreshold = 2 ;
    ParamSet.iWhiteSpotNumMin = 30 ;
}

void CDlgSetting::OnBnClickedRadioDepth3()
{
	ParamSet.iSceneDepth = 0 ;
    ParamSet.iPersonWhitePotNum = 80 ;
    ParamSet.iXTrackContinueThreshold = 1 ;
    ParamSet.iWhiteSpotNumMin = 20 ;
}

void CDlgSetting::OnBnClickedRadioSensitive1()
{
    ParamSet.bSensitiveFlag = 0 ;
}

void CDlgSetting::OnBnClickedRadioSensitive2()
{
    ParamSet.bSensitiveFlag = 1 ;
}

void CDlgSetting::OnBnClickedRadioSensitive3()
{
    ParamSet.bSensitiveFlag = 2 ;
}

void CDlgSetting::OnEnChangeEditTwoValue()
{
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

void CDlgSetting::OnBnClickedButtonCancel()
{
    m_ComboChangeNum = ((CComboBox*)GetDlgItem(IDC_COMBO_CONTROL))->GetCurSel(); //! 
    switch ( m_ComboChangeNum )
    {
        //A1.黑框(5)
    case COMBOBOX_ITEM_RECT_BLACK_BLOCK:
        for (int i = 0 ; i < 5 ; i++)
        {
            ParamSet.tRectBlackBlock[i].BeginPointX = 0;
            ParamSet.tRectBlackBlock[i].BeginPointY = 0;
            ParamSet.tRectBlackBlock[i].EndPointX = 0;
            ParamSet.tRectBlackBlock[i].EndPointY = 0;
            ParamSet.tRectBlackBlock[i].bFlag = 0 ;
        }
        break ;
        //A2.遮挡线左
    case COMBOBOX_ITEM_LINE_BLACK_LEFT:
        ParamSet.tLineBlackLeft.BeginPointX = 0;
        ParamSet.tLineBlackLeft.BeginPointY = 0;
        ParamSet.tLineBlackLeft.EndPointX = 0;
        ParamSet.tLineBlackLeft.EndPointY = 0;
        ParamSet.tLineBlackLeft.bFlag = 0 ;
        break ;
        //A3.遮挡线右
    case COMBOBOX_ITEM_LINE_BLACK_RIGHT:
        ParamSet.tLineBlackRight.BeginPointX = 0;
        ParamSet.tLineBlackRight.BeginPointY = 0;
        ParamSet.tLineBlackRight.EndPointX = 0;
        ParamSet.tLineBlackRight.EndPointY = 0;
        ParamSet.tLineBlackRight.bFlag = 0 ;
        break ;
        //B1.越界线左
    case COMBOBOX_ITEM_LINE_STRAIGHT_FIRST:
        ParamSet.tLineStraightFirst.BeginPointX = 0;
        ParamSet.tLineStraightFirst.BeginPointY = 0;
        ParamSet.tLineStraightFirst.EndPointX = 0;
        ParamSet.tLineStraightFirst.EndPointY = 0;
        ParamSet.tLineStraightFirst.bFlag = 0 ;    
        break ;
        //B2.越界线右
    case COMBOBOX_ITEM_LINE_STRAIGHT_SECOND:
        ParamSet.tLineStraightSecond.BeginPointX = 0;
        ParamSet.tLineStraightSecond.BeginPointY = 0;
        ParamSet.tLineStraightSecond.EndPointX = 0;
        ParamSet.tLineStraightSecond.EndPointY = 0;
        ParamSet.tLineStraightSecond.bFlag = 0 ;
        break ;
        //B3.拟合曲线
    case COMBOBOX_ITEM_LINE_CURVER_RANGE:
        ParamSet.tLineCurverRange.BeginPointX = 0;
        ParamSet.tLineCurverRange.BeginPointY = 0;
        ParamSet.tLineCurverRange.EndPointX = 0;
        ParamSet.tLineCurverRange.EndPointY = 0;
        ParamSet.tLineCurverRange.bFlag = 0 ;
        break ;
        //C1.人区域左线
    case COMBOBOX_ITEM_PERSON_RANGE_0:
        ParamSet.iPersonRange[0].BeginPointX = 0;
        ParamSet.iPersonRange[0].BeginPointY = 0;
        ParamSet.iPersonRange[0].EndPointX = 0;
        ParamSet.iPersonRange[0].EndPointY = 0;
        ParamSet.iPersonRange[0].bFlag = 0 ;
        break ;
        //C2.人区域右线
    case COMBOBOX_ITEM_PERSON_RANGE_1:
        ParamSet.iPersonRange[1].BeginPointX = 0;
        ParamSet.iPersonRange[1].BeginPointY = 0;
        ParamSet.iPersonRange[1].EndPointX = 0;
        ParamSet.iPersonRange[1].EndPointY = 0;
        ParamSet.iPersonRange[1].bFlag = 0 ;
        break ;
        //C3.人区域高线
    case COMBOBOX_ITEM_PERSON_RANGE_2:
        ParamSet.iPersonRange[2].BeginPointX = 0;
        ParamSet.iPersonRange[2].BeginPointY = 0;
        ParamSet.iPersonRange[2].EndPointX = 0;
        ParamSet.iPersonRange[2].EndPointY = 0;
        ParamSet.iPersonRange[2].bFlag = 0 ;
        break ;
        //D1.夜间区域
    case COMBOBOX_ITEM_NIGHT_RANGE_0:
        ParamSet.tNightRange[0].BeginPointX = 0;
        ParamSet.tNightRange[0].BeginPointY = 0;
        ParamSet.tNightRange[0].EndPointX = 0;
        ParamSet.tNightRange[0].EndPointY = 0;
        ParamSet.tNightRange[0].bFlag = 0 ;
        break ;
        //D2.夜间区域
    case COMBOBOX_ITEM_NIGHT_RANGE_1:
        ParamSet.tNightRange[1].BeginPointX = 0;
        ParamSet.tNightRange[1].BeginPointY = 0;
        ParamSet.tNightRange[1].EndPointX = 0;
        ParamSet.tNightRange[1].EndPointY = 0;
        ParamSet.tNightRange[1].bFlag = 0 ;
        break ;
        //E1.直抛区域
    case COMBOBOX_ITEM_RECT_LITTLE_REGION:
        ParamSet.tRectLittleRegion.BeginPointX = 0;
        ParamSet.tRectLittleRegion.BeginPointY = 0;
        ParamSet.tRectLittleRegion.EndPointX = 0;
        ParamSet.tRectLittleRegion.EndPointY = 0;
        ParamSet.tRectLittleRegion.bFlag = 0 ;    
        break ;
        //E2.直抛高度
    case COMBOBOX_ITEM_LITTLE_REGION_Y_LINE:
        ParamSet.tLittleRegionYLine.BeginPointX = 0;
        ParamSet.tLittleRegionYLine.BeginPointY = 0;
        ParamSet.tLittleRegionYLine.EndPointX = 0;
        ParamSet.tLittleRegionYLine.EndPointY = 0;
        ParamSet.tLittleRegionYLine.bFlag = 0 ;
        break ;
    default:            
        break;
    }
}

void CDlgSetting::OnBnClickedCheckPerson()
{
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
    SaveComboboxData();
    OnOK();
}
//void CDlgSetting::OnEnChangeEditMaxFrame()
//{
//}

void CDlgSetting::OnEnChangeEditWhiteSpotMin()
{
	// TODO:  Add your control notification handler code here
	int tempVal = m_edit_white_min;
	UpdateData(true);
	if ( m_edit_white_min >= 0 && m_edit_white_min <= 50 )
	{
		ParamSet.iWhiteSpotNumMin = m_edit_white_min ;
	}
	else
	{
		m_edit_white_min = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnEnChangeEditFilterValue()
{
	int tempVal = m_edit_filter_value;
	UpdateData(true);
	if ( m_edit_filter_value >= 0 && m_edit_filter_value <= 3 )
	{
		ParamSet.iImfilterSingleThreshold = m_edit_filter_value ;
	}
	else
	{
		m_edit_filter_value = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnEnChangeEditImdilateValue()
{	
	int tempVal = m_edit_imdilate_value;
	UpdateData(true);
	if ( m_edit_imdilate_value >= 0 && m_edit_imdilate_value <= 5 )
	{
		ParamSet.iImdilateThreshold = m_edit_imdilate_value ;
	}
	else
	{
		m_edit_imdilate_value = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnEnChangeEditWhiteSpotMax()
{
	int tempVal = m_edit_white_spot_max;
	UpdateData(true);
	if ( m_edit_white_spot_max >= 0 && m_edit_white_spot_max <= 30000 )
	{
		ParamSet.iWhiteSpotNumMax = m_edit_white_spot_max ;
	}
	else
	{
		m_edit_white_spot_max = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnEnChangeEditNightThr()
{
	int tempVal = m_edit_night_thr;
	UpdateData(true);
	if ( m_edit_night_thr >= 0 && m_edit_night_thr <= 80 )
	{
		ParamSet.iNightSubThreshold = m_edit_night_thr ;
	}
	else
	{
		m_edit_night_thr = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnEnChangeEditAlarm()
{
	int tempVal = m_edit_alarm;
	UpdateData(true);
	if ( m_edit_alarm >= 0 && m_edit_alarm <= 10 )
	{
		ParamDsting.FindObjValue.FirstValue = m_edit_alarm ;
	}
	else
	{
		m_edit_alarm = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnEnChangeEditAlarmDelay()
{
	int tempVal = m_edit_alarm_delay;
	UpdateData(true);
	if ( m_edit_alarm_delay >= 0 && m_edit_alarm_delay <= 30 )
	{
		ParamSet.iAlarmDelay = m_edit_alarm_delay ;
	}
	else
	{
		m_edit_alarm_delay = tempVal ;
		UpdateData(false);
	}
}

void CDlgSetting::OnBnClickedButtonNext()
{
    m_ComboChangeNum = m_cbGraphicsType.GetCurSel();
    int iTotal = m_cbGraphicsType.GetCount();
    if(m_ComboChangeNum+1 >= iTotal)
    {
        AfxMessageBox(_T("已经达到最后一项"));
        return;
    }
    else if(m_ComboChangeNum < 0)
    {
        m_ComboChangeNum = 0;
    }

    m_ComboChangeNum++;
    m_cbGraphicsType.SetCurSel(m_ComboChangeNum);
    this->OnCbnSelchangeComboControl();
}

void CDlgSetting::OnBnClickedCheckNight()
{
	// TODO: Add your control notification handler code here
	if( ((CButton*)GetDlgItem(IDC_CHECK_NIGHT))->GetCheck() == BST_UNCHECKED )
	{
		ParamDsting.FindObjValMax.FirstValue = 0 ; 
	}
	else
	{
		ParamDsting.FindObjValMax.FirstValue = 1 ;
	}
}

#pragma once

#include "MaskStatic.h"
#include "cv.h"
#include "highgui.h"

// CDlgPersonCfg �Ի���

class CDlgPersonCfg : public CDialog
{
	DECLARE_DYNAMIC(CDlgPersonCfg)

public:
	CDlgPersonCfg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgPersonCfg();

// �Ի�������
	enum { IDD = IDD_PERSON_CFG };


public:
    virtual BOOL OnInitDialog();
    void setImage(const IplImage *pImage);

private:
    IplImage *m_pImage;
    CMaskStatic m_ctrlImage;

    PolyLineArray m_mask; //����
    LineArray m_warningLine;   //Ԥ����
    RectArray m_rect;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
};

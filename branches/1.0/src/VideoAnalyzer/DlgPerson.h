#pragma once


// CDlgPerson dialog

#include "Line.h"
#include "resource.h"

class CDlgPerson : public CDialog
{
	DECLARE_DYNAMIC(CDlgPerson)

public:
	CDlgPerson(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPerson();

// Dialog Data
	enum { IDD = IDD_SETDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  /*�ֶ����*/
public:
  unsigned char* datafromline;
  Line*          line_TestGUI;
  int                 m_nWidth;
  int                 m_nHeight;

  void desrory( );
  void reinit(int imWidthin, int imHeightin );

  /*�ֶ���� .end */

	DECLARE_MESSAGE_MAP()
  afx_msg void OnBnClickedBtnRefresh();
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnBnClickedBtnFill();
  afx_msg void OnBnClickedBtnCar();
  afx_msg void OnBnClickedBtnUndo();
  afx_msg void OnBnClickedBtnOutput();
  afx_msg void OnBnClickedBtnSave();
  virtual BOOL OnInitDialog();
  afx_msg void OnPaint();
  afx_msg void OnBnClickedBtnInverse();
};

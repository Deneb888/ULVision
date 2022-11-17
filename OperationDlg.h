#pragma once

#include "ManuDlg.h"
#include "ParsaveDlg.h"


//*****************************************************************
//Constant definition
//*****************************************************************
// operation inner message
//#define UM_OPERPROCESS WM_USER + 108			// operation dialog 自定义消息序号
#define UM_OPERPROCESS WM_USER + 201			// operation dialog 自定义消息序号
#define UM_OPERPIDPROCESS WM_USER + 202			// operation dialog PID返回数据处理消息序号
#define UM_OPERLEDPROCESS WM_USER + 203			// operation dialog LED返回数据处理消息序号
#define UM_OPEROVERSHOOTPROCESS WM_USER + 204	// operation dialog overshoot返回数据处理消息序号

// parameter default
#define PITEMP			"10"
#define INIDENTEMP		"10"
#define INIDENTIME		"100"
#define DENTEMP			"10"
#define DENTIME			"100"
#define ANNTEMP			"10"
#define ANNTIME			"100"
#define EXTTEMP			"10"
#define EXTTIME			"100"
#define HOLDTEMP		"10"
#define HOLDTIME		"100"
#define ANNCYCLE		"5"
#define KP				"10"
#define KI				"10"
#define KD				"10"
#define KL				"10"
// operation dialog message to main dialog
#define READSTATUS		1
#define sendopemsg		2


//*****************************************************************
//External variable definition
//*****************************************************************
extern CString sSave;		// save parameter buffer
extern int OperDlgFlag;		// operation dialog message flag
extern int OperReadstautsFlag;		// read status flag
extern BYTE OperBuf [200];				// operation dialog buffer

// COperationDlg dialog

class COperationDlg : public CDialog
{
	DECLARE_DYNAMIC(COperationDlg)

public:
	COperationDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COperationDlg();

// Dialog Data
	enum { IDD = IDD_OPERATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
public:
	CManuDlg m_ManuDlg;
	CParsaveDlg m_ParsaveDlg;
	afx_msg LRESULT OnOpearProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearPIDProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearLEDProcess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpearOvershootProcess(WPARAM wParam, LPARAM lParam);
	void OperCalMainMsg();
	afx_msg void OnBnClickedBtnOpeLoad();
	afx_msg void OnBnClickedBtnOpeSavestatus();
	afx_msg void OnBnClickedBtnOpeClear();
//	afx_msg void OnEnChangeEditOpeDentem();
	virtual BOOL OnInitDialog();	
	afx_msg void OnEnKillfocusEditOpePitem();
	afx_msg void OnEnKillfocusEditOpeDentem();
	afx_msg void OnEnKillfocusEditOpeDentime();
	afx_msg void OnEnKillfocusEditOpeDeningtem();
	afx_msg void OnEnKillfocusEditOpeDeningtime();
	afx_msg void OnEnKillfocusEditOpeAnntem();
	afx_msg void OnEnKillfocusEditOpeAnntime();
	afx_msg void OnEnKillfocusEditOpeAnncycle();
	afx_msg void OnEnKillfocusEditOpeExttem();
	afx_msg void OnEnKillfocusEditOpeExttime();
	afx_msg void OnEnKillfocusEditOpeHoldtem();
	afx_msg void OnEnKillfocusEditOpeHoldtime();
	afx_msg void OnEnKillfocusEditOpeKp();
	afx_msg void OnEnKillfocusEditOpeKi();
	afx_msg void OnEnKillfocusEditOpeKd();
	afx_msg void OnEnKillfocusEditOpeKl();
	afx_msg void OnBnClickedBtnOpeSavepara();
	afx_msg void OnBnClickedBtnOpeMore();
	void OperReadStatus();
	CString m_operdlg_sEditpel;
	CString m_operdlg_sEditpiread;
	CString m_operdlg_sEditcycleread;
//	CEdit m_operdlg_sEditpwm;
//	CString m_operdlg_sEditpwm;
	CString m_operdlg_sEditvanstatus;
	CString m_operdlg_sEditcurstatus;
	afx_msg void OnBnClickedBtnOpeReadstart();
//	CString m_operdlg_sEditpwm2;
//	CString m_operdlg_sEditpwm1;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnOpeStart();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnOpeLedset();
	afx_msg void OnBnClickedBtnOpeLedread();
	afx_msg void OnBnClickedBtnOpeFanon();
	afx_msg void OnBnClickedBtnOpeFanoff();
	afx_msg void OnBnClickedBtnOpeLedon();
	afx_msg void OnBnClickedBtnOpeLenoff();
	afx_msg void OnBnClickedBtnOpeKpset();
	afx_msg void OnBnClickedBtnOpeKdset();
	afx_msg void OnBnClickedBtnOpeKiset();
	afx_msg void OnBnClickedBtnOpeKlset();
	afx_msg void OnBnClickedBtnOpePidread();
	afx_msg void OnBnClickedBtnOpePidzoneset();
	afx_msg void OnBnClickedBtnOpeStop();
	afx_msg void OnBnClickedBtnOpeFanclosetemp();
	afx_msg void OnBnClickedBtnOpeFanread();
	afx_msg void OnBnClickedBtnOpeOversend();
	afx_msg void OnBnClickedBtnOpeOverread();
};

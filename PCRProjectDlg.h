
// PCRProjectDlg.h : header file
//
//**************************************************************
//Message definition to Resource.h
//**************************************************************
#define WM_RegDlg_event		WM_USER + 101		
#define WM_GraDlg_event		WM_USER + 102	
#define WM_TrimDlg_event	WM_USER + 103
#define WM_ReadHID_event	WM_USER + 105
#define WM_OperDlg_event	WM_USER + 106
#define WM_ParsaveDlg_event		WM_USER + 107

//.......................................................................................

#if !defined(AFX_USBHIDIOCDLG_H__0B2AAA84_F5A9_11D3_9F47_0050048108EA__INCLUDED_)
#define AFX_USBHIDIOCDLG_H__0B2AAA84_F5A9_11D3_9F47_0050048108EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//....................................................................

//#pragma once
#include "GraDlg.h"
#include "RegDlg.h"
#include "TrimDlg.h"
#include "OperationDlg.h"
#include "ParsaveDlg.h"
#include "TrimReader.h"


//****************************************************
//Global variable definition
//****************************************************
#define TxNum			64		// the number of the buffer for sent data to COMX; Zhimin changed to 64 for HID
#define RxNum			64		// the number of the buffer for received data from COMX; Zhimin changed to 64 for HID
#define dNum			29			// ÿ���򴮿ڷ��͵��ֽ���
#define ONCOMNUM		59		// ��λ�����ض����ֽڻ����COM���ж�

#define HIDREPORTNUM	65		//	HID ÿ��report byte����, 1 based
//#define HIDREPORTNUM	130		//	HID ÿ��report byte����

#define VEDIOPITCHTIME	264000			// vedio��ʾʱÿ֡��ʾ�ļ��ʱ��

#define GraCmd			0x02			// ͼ�� command 
#define ReadCmd			0x04			// Read command
#define TempCmd			0x10			// peltier Control(temperature) command
#define PidCmd			0x11			// PID command
#define PidReadCfgCmd	0x12			// ��ȡPID configure��Ϣ command
#define PidReadCycCmd	0x13			// ��ȡPID cycle�Ȳ�����Ϣ command
#define CycCmd			0x14			// ��ȡѭ����Ϣ command

#define HIDBUFSIZE		12

#define PAGENUM			2				// 1: ֻ��ʾoperation dialog
										// 2: ��ʾ����Ի���

//*****************************************************************
//External variable definition
//*****************************************************************
extern BYTE TxData[TxNum];		// the buffer of sent data to COMX
extern BYTE RxData[RxNum];		// the buffer of received data from COMX

extern CString RegRecStr;				//���������ַ���buffer ʮ������
extern CString Dec_RegRecStr;			//���������ַ���buffer ʮ����
extern CString Valid_RegRecStr;			//��Ч���������ַ���buffer ʮ������
extern CString Valid_Dec_RegRecStr;		//��Ч���������ַ���buffer ʮ����


// CPCRProjectDlg dialog
class CPCRProjectDlg : public CDialogEx
{
// Construction
public:
	CPCRProjectDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PCRPROJECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();

// Implementation
protected:
	HICON m_hIcon;
	CTrimReader m_TrimReader;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnRegDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGraDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrimDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReadHID(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOperDlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnParsaveDlg(WPARAM wParam, LPARAM iParam);
	DECLARE_MESSAGE_MAP()

	BOOL DeviceNameMatch(LPARAM lParam);
	bool FindTheHID();
	LRESULT Main_OnDeviceChange(WPARAM wParam, LPARAM lParam);
	void CloseHandles();
	void DisplayInputReport();
	void DisplayReceivedData(char ReceivedByte);
	void GetDeviceCapabilities();
	void PrepareForOverlappedTransfer();
	void ReadAndWriteToDevice();
	void ReadHIDInputReport();
	void RegisterForDeviceNotifications();
	void WriteHIDOutputReport();
	void SendHIDRead();

public:
//	CMscomm1 m_mscomm;
	DECLARE_EVENTSINK_MAP()
	void OnCommMscomm1();
	CTabCtrl m_tab;
	CRegDlg m_RegDlg;
	CGraDlg m_GraDlg;
	CTrimDlg m_TrimDlg;
//	COperationDlg m_OperDlg;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	void GetCom();
	CString m_StaticOpenComm;
	void CommSend (int num);
	afx_msg void OnBnClickedBtnOpencomm();
	afx_msg void OnBnClickedBtnOpenhid();
//	CString m_strBytesReceived;
//	CEdit m_BytesReceived;
	CListBox m_BytesReceived;
	CString m_strBytesReceived;
	afx_msg void OnBnClickedBtnSendhid();
	afx_msg void OnBnClickedBtnReadhid();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	void GetFWVersion();
};

#endif // !defined(AFX_USBHIDIOCDLG_H__0B2AAA84_F5A9_11D3_9F47_0050048108EA__INCLUDED_)
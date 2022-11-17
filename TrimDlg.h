#pragma once
#include "afxwin.h"

//***************************************************************
//Constant definition
//***************************************************************
#define UM_TRIMPROCESS WM_USER+205	//Trim dialog自定义消息序号

#define sendtrimmsg		1		// ReadRow button
#define sendeeprommsg	2		// Read EEPROM
#define trimdatanum		200		// graphic dialog transmitted data number

extern BYTE TrimBuf[trimdatanum];	// trim dialog transmitted data buffer

// CTrimDlg dialog

class CTrimDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrimDlg)

public:
	CTrimDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrimDlg();

// Dialog Data
	enum { IDD = IDD_TRIM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnReread();
	void TrimCalMainMsg();
	afx_msg LRESULT OnTrimProcess(WPARAM wParam, LPARAM lParam);
	CComboBox m_ComboxRampgen;
	virtual BOOL OnInitDialog();
	CComboBox m_ComboxRange;
	CComboBox m_ComboxAmux;
	CComboBox m_ComboxIpix;
	CComboBox m_ComboxSwitch;
	CComboBox m_ComboxTestADC;
	CComboBox m_ComboxTX;
	CComboBox m_ComboxV15;
	CComboBox m_ComboxV20;
	CComboBox m_ComboxV24;
	CEdit m_EditRecData;
	CEdit m_EditData;
	afx_msg void OnCbnSelchangeComboRampgen();
	afx_msg void OnCbnSelchangeComboRange();
	afx_msg void OnCbnSelchangeComboV24();
	afx_msg void OnCbnSelchangeComboV20();
	afx_msg void OnCbnSelchangeComboV15();
	afx_msg void OnCbnSelchangeComboIpix();
	afx_msg void OnCbnSelchangeComboSwitch();
	afx_msg void OnCbnSelchangeComboTxbin();
	afx_msg void OnCbnSelchangeComboAmuxsel();
	afx_msg void OnCbnSelchangeComboTestadc();
	afx_msg void OnBnClickedBtnSendcmd();
	afx_msg void OnBnClickedBtnItgtim();
	void ResetTrim(void);
	void SetRange(char range);
	void SetV20(char v20);
	void SetV15(char v15);
	void SetRampgen(char rampgen);
	void SetTXbin(char txbin);
	void ResetTxBin(void);
	afx_msg void OnBnClickedTrimBtnPcrtemp1();
	afx_msg void OnBnClickedTrimBtnPcrtemp2();
	afx_msg void OnBnClickedBtnShowdata();
	int m_radio_trimdlg_isensor;
	BYTE SensorRadioSel(BYTE selBuf);
	afx_msg void OnClickedRadioTrimSensor1();
	afx_msg void OnBnClickedRadioTrimSensor2();
	afx_msg void OnBnClickedRadioTrimSensor3();
	afx_msg void OnBnClickedRadioTrimSensor4();

	void SelSensor(BYTE);
	void SelSensor4Read(BYTE);
	void SetV20(char V20, char i);
	afx_msg void OnBnClickedButton3();
	void SetIntTime(int ch);
	afx_msg void OnBnClickedBtnItgtim2();

	void SetGainMode(int);
	void SetGainMode(int, char);
	void GetTemp(void);
	afx_msg void OnBnClickedPwrsave();
	void GetOSCStat(void);
	CComboBox m_ComboxTX2;
	afx_msg void OnSelchangeComboTxbin2();
	afx_msg void OnBnClickedBtnItgtim3();
	afx_msg void OnBnClickedBtnItgtim4();

	CComboBox m_ComboxTX3;
	CComboBox m_ComboxTX4;
	afx_msg void OnCbnSelchangeComboTxbin3();
	afx_msg void OnCbnSelchangeComboTxbin4();

//	Remap related functions and variables

	void SetChannelMap(BYTE channel_index, BYTE chip_index, BYTE led_index, BYTE frame_size, float int_time);
	afx_msg void OnBnClickedBtnChanset();
	void UpdateRemapTable();

	void EnableRemap(BYTE);

	int chan, chip, led;
	int chan_1, chip_1, led_1;	
	BOOL remap_en;

	void EEPROMWrite(int chan, int total_packets, int packet_index, BYTE* packet_data, int packet_size);
	void EEPROMRead(int chan);

	void CommitTxBin();

	afx_msg void OnBnClickedButtonSavconf();
	void Loadconf();
	void ParseJsonFromFile(CString filename);
	afx_msg void OnBnClickedButtonLoadconf();
	afx_msg void OnBnClickedButtonTrimtest();
};

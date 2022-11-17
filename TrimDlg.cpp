// TrimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "TrimDlg.h"
#include "IntTimeDlg.h"
#include "KineticDlg.h"
#include "ChanSetDlg.h"
#include "afxdialogex.h"

#include <fstream>
#include <afx.h>
#include <vector>
#include "lib\json\include\json.h"	

#define CONFIGFILE "Config.json"

//*****************************************************************
//Constant definition
//*****************************************************************


//***************************************************************
//Global variable definition
//***************************************************************
int TrimFlag;					// register dialog flag

BYTE TrimBuf[trimdatanum];		// trim dialog transmitted data buffer

// BYTE TimCom;						//command下拉框的取值buffer
// BYTE TimType;					//type下拉框的取值buffer

BYTE SensorRadioValue;			// sensor radio select value

//*****************************************************************
//TrimDlg global function definition
//*****************************************************************


//*****************************************************************
//External variable definition
//*****************************************************************
extern CTrimReader		*g_pTrimReader;
extern CGraDlg			*g_pGraDlg;


//****************************************************
//External function 
//****************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
extern int ChangeNum (CString str, int length);							//十六进制字符串转十进制整型
extern  char* EditDataCvtChar (CString strCnv,  char * charRec);				//编辑框取值转字符变量

int 	gain_mode = 1; 			// zd add, 0 high gain, 1 low gain
extern int g_FrameSize;			// These are mirrors of the GUI value in GraDlg.

float 	int_time = 1.0; 		// zd add, in ms
float   int_time2 = 1.0;
float	int_time3 = 1.0;
float	int_time4 = 1.0;

int tx_bin = 0x0f;				// Apply to 12X12 mode only
int tx_bin2 = 0x0f;
int tx_bin3 = 0x0f;				// 
int tx_bin4 = 0x0f;

extern 	BOOL g_DeviceDetected;

extern unsigned int videoElapseTime;
extern unsigned int autoRepeatMaxCount;

extern 	CString RegRecStr;

int g_OSCStat = 1;		// 1: active mode

float TempC_sum = 0;
#define T_REPEAT 1

float g_FrameTemp;

extern int LAST_PCR;

BOOL g_EnableRemap = false;

extern BOOL g_ChannelMask[];

// CTrimDlg dialog

IMPLEMENT_DYNAMIC(CTrimDlg, CDialog)

CTrimDlg::CTrimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrimDlg::IDD, pParent)
{

	m_radio_trimdlg_isensor = 0;

	chan = 0; 
	chip = 0; 
	led = 0;
	chan_1 = 1; 
	chip_1 = 0; 
	led_1 = 1;	
	remap_en = false;
}

CTrimDlg::~CTrimDlg()
{
}

void CTrimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_Rampgen, m_ComboxRampgen);
	//	DDX_Control(pDX, IDC_COMBO_Rampgen2, m_ComboxRange);
	DDX_Control(pDX, IDC_COMBO_RANGE, m_ComboxRange);
	DDX_Control(pDX, IDC_COMBO_AmuxSel, m_ComboxAmux);
	//  DDX_Control(pDX, IDC_COMBO_CMD, m_ComboxcOM);
	DDX_Control(pDX, IDC_COMBO_IPIX, m_ComboxIpix);
	DDX_Control(pDX, IDC_COMBO_Switch, m_ComboxSwitch);
	DDX_Control(pDX, IDC_COMBO_TestADC, m_ComboxTestADC);
	DDX_Control(pDX, IDC_COMBO_TxBin, m_ComboxTX);
	//  DDX_Control(pDX, IDC_COMBO_Type, m_ComboxType);
	DDX_Control(pDX, IDC_COMBO_V15, m_ComboxV15);
	DDX_Control(pDX, IDC_COMBO_V20, m_ComboxV20);
	DDX_Control(pDX, IDC_COMBO_V24, m_ComboxV24);
	DDX_Control(pDX, IDC_EDIT_REDATA, m_EditRecData);
	DDX_Control(pDX, IDC_EDIT_SENDDATA, m_EditData);
	DDX_Radio(pDX, IDC_RADIO_TRIM_SENSOR1, m_radio_trimdlg_isensor);
	DDX_Control(pDX, IDC_COMBO_Txbin2, m_ComboxTX2);
	DDX_Control(pDX, IDC_COMBO_TxBin3, m_ComboxTX3);
	DDX_Control(pDX, IDC_COMBO_Txbin4, m_ComboxTX4);
}


BEGIN_MESSAGE_MAP(CTrimDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_REREAD, &CTrimDlg::OnBnClickedBtnReread)
	ON_CBN_SELCHANGE(IDC_COMBO_Rampgen, &CTrimDlg::OnCbnSelchangeComboRampgen)
	ON_CBN_SELCHANGE(IDC_COMBO_RANGE, &CTrimDlg::OnCbnSelchangeComboRange)
	ON_CBN_SELCHANGE(IDC_COMBO_V24, &CTrimDlg::OnCbnSelchangeComboV24)
	ON_CBN_SELCHANGE(IDC_COMBO_V20, &CTrimDlg::OnCbnSelchangeComboV20)
	ON_CBN_SELCHANGE(IDC_COMBO_V15, &CTrimDlg::OnCbnSelchangeComboV15)
	ON_CBN_SELCHANGE(IDC_COMBO_IPIX, &CTrimDlg::OnCbnSelchangeComboIpix)
	ON_CBN_SELCHANGE(IDC_COMBO_Switch, &CTrimDlg::OnCbnSelchangeComboSwitch)
	ON_CBN_SELCHANGE(IDC_COMBO_TxBin, &CTrimDlg::OnCbnSelchangeComboTxbin)
	ON_CBN_SELCHANGE(IDC_COMBO_Txbin2, &CTrimDlg::OnSelchangeComboTxbin2)	
	ON_CBN_SELCHANGE(IDC_COMBO_AmuxSel, &CTrimDlg::OnCbnSelchangeComboAmuxsel)
	ON_CBN_SELCHANGE(IDC_COMBO_TestADC, &CTrimDlg::OnCbnSelchangeComboTestadc)
	ON_BN_CLICKED(IDC_BTN_SENDCMD, &CTrimDlg::OnBnClickedBtnSendcmd)
//	ON_CBN_KILLFOCUS(IDC_COMBO_CMD, &CTrimDlg::OnKillfocusComboCmd)
//	ON_CBN_KILLFOCUS(IDC_COMBO_Type, &CTrimDlg::OnKillfocusComboType)
	ON_BN_CLICKED(IDC_BTN_ITGTIM, &CTrimDlg::OnBnClickedBtnItgtim)
	ON_BN_CLICKED(IDC_TRIM_BTN_PCRTEMP1, &CTrimDlg::OnBnClickedTrimBtnPcrtemp1)
	ON_BN_CLICKED(IDC_TRIM_BTN_PCRTEMP2, &CTrimDlg::OnBnClickedTrimBtnPcrtemp2)
	ON_MESSAGE(UM_TRIMPROCESS,OnTrimProcess)
	ON_BN_CLICKED(IDC_BTN_SHOWDATA, &CTrimDlg::OnBnClickedBtnShowdata)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR1, &CTrimDlg::OnClickedRadioTrimSensor1)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR2, &CTrimDlg::OnBnClickedRadioTrimSensor2)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR3, &CTrimDlg::OnBnClickedRadioTrimSensor3)
	ON_BN_CLICKED(IDC_RADIO_TRIM_SENSOR4, &CTrimDlg::OnBnClickedRadioTrimSensor4)
	ON_BN_CLICKED(IDC_BUTTON3, &CTrimDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_ITGTIM2, &CTrimDlg::OnBnClickedBtnItgtim2)
	ON_BN_CLICKED(IDC_CHECK_PWRSAVE, &CTrimDlg::OnBnClickedPwrsave)

	ON_BN_CLICKED(IDC_BTN_ITGTIM3, &CTrimDlg::OnBnClickedBtnItgtim3)
	ON_BN_CLICKED(IDC_BTN_ITGTIM4, &CTrimDlg::OnBnClickedBtnItgtim4)

	ON_CBN_SELCHANGE(IDC_COMBO_TxBin3, &CTrimDlg::OnCbnSelchangeComboTxbin3)
	ON_CBN_SELCHANGE(IDC_COMBO_Txbin4, &CTrimDlg::OnCbnSelchangeComboTxbin4)
	ON_BN_CLICKED(IDC_BTN_CHANSET, &CTrimDlg::OnBnClickedBtnChanset)
	ON_BN_CLICKED(IDC_BUTTON_SAVCONF, &CTrimDlg::OnBnClickedButtonSavconf)
	ON_BN_CLICKED(IDC_BUTTON_LOADCONF, &CTrimDlg::OnBnClickedButtonLoadconf)
	ON_BN_CLICKED(IDC_BUTTON_TRIMTEST, &CTrimDlg::OnBnClickedButtonTrimtest)
END_MESSAGE_MAP()


// CTrimDlg message handlers

BYTE CTrimDlg::SensorRadioSel(BYTE selBuf)
{
	UpdateData(TRUE);
	switch(m_radio_trimdlg_isensor)
	{
	case 0:
		selBuf = 0;
		break;
	case 1:
		selBuf = 1;
		break;
	case 2:
		selBuf = 2;
		break;
	case 3:
		selBuf = 3;
		break;
	default:
		break;
	}

	return selBuf;
}

// Repurposed to Reset Trim

void CTrimDlg::OnBnClickedBtnReread()
{
	// TODO: Add your control notification handler code here

/*	// Only ADC conversion and SPI read	
	TrimFlag = sendtrimmsg; 
	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x02;			//command
	TxData[2] = 0x01;			//data length
	TxData[3] = 0x05;			//data type, dat edit first byte
	TxData[4] = 0x00;			//real data, data edit second byte
	TxData[5] = 0x08;			//real data, data edit third byte
	TxData[6] = 0x17;			//check sum
	TxData[7] = 0x17;			//back code
	TxData[8] = 0x17;			//back code

	//Call maindlg message
	TrimCalMainMsg();
*/
	// TODO: Add your control notification handler code here
	ResetTrim();
}

void CTrimDlg::TrimCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_TrimDlg_event,a,b);
}


BOOL CTrimDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	//Rampgen add string
	CString sRamp;

	for (unsigned int i=0; i<256; i++) // this is strange
	{
		sRamp.Format("%.2X",i);
		m_ComboxRampgen.AddString(sRamp);
	}

	//Range add string
	m_ComboxRange.AddString("0x00");
	m_ComboxRange.AddString("0x01");
	m_ComboxRange.AddString("0x02");
	m_ComboxRange.AddString("0x03");
	m_ComboxRange.AddString("0x04");
	m_ComboxRange.AddString("0x05");
	m_ComboxRange.AddString("0x06");
	m_ComboxRange.AddString("0x07");
	m_ComboxRange.AddString("0x08");
	m_ComboxRange.AddString("0x09");
	m_ComboxRange.AddString("0x0a");
	m_ComboxRange.AddString("0x0b");
	m_ComboxRange.AddString("0x0c");
	m_ComboxRange.AddString("0x0d");
	m_ComboxRange.AddString("0x0e");
	m_ComboxRange.AddString("0x0f");

	//V24 add string
	m_ComboxV24.AddString("0x00");
	m_ComboxV24.AddString("0x01");
	m_ComboxV24.AddString("0x02");
	m_ComboxV24.AddString("0x03");
	m_ComboxV24.AddString("0x04");
	m_ComboxV24.AddString("0x05");
	m_ComboxV24.AddString("0x06");
	m_ComboxV24.AddString("0x07");
	m_ComboxV24.AddString("0x08");
	m_ComboxV24.AddString("0x09");
	m_ComboxV24.AddString("0x0a");
	m_ComboxV24.AddString("0x0b");
	m_ComboxV24.AddString("0x0c");
	m_ComboxV24.AddString("0x0d");
	m_ComboxV24.AddString("0x0e");
	m_ComboxV24.AddString("0x0f");

	//V20 add string
	m_ComboxV20.AddString("0x00");
	m_ComboxV20.AddString("0x01");
	m_ComboxV20.AddString("0x02");
	m_ComboxV20.AddString("0x03");
	m_ComboxV20.AddString("0x04");
	m_ComboxV20.AddString("0x05");
	m_ComboxV20.AddString("0x06");
	m_ComboxV20.AddString("0x07");
	m_ComboxV20.AddString("0x08");
	m_ComboxV20.AddString("0x09");
	m_ComboxV20.AddString("0x0a");
	m_ComboxV20.AddString("0x0b");
	m_ComboxV20.AddString("0x0c");
	m_ComboxV20.AddString("0x0d");
	m_ComboxV20.AddString("0x0e");
	m_ComboxV20.AddString("0x0f");

	//V15 add string
	m_ComboxV15.AddString("0x00");
	m_ComboxV15.AddString("0x01");
	m_ComboxV15.AddString("0x02");
	m_ComboxV15.AddString("0x03");
	m_ComboxV15.AddString("0x04");
	m_ComboxV15.AddString("0x05");
	m_ComboxV15.AddString("0x06");
	m_ComboxV15.AddString("0x07");
	m_ComboxV15.AddString("0x08");
	m_ComboxV15.AddString("0x09");
	m_ComboxV15.AddString("0x0a");
	m_ComboxV15.AddString("0x0b");
	m_ComboxV15.AddString("0x0c");
	m_ComboxV15.AddString("0x0d");
	m_ComboxV15.AddString("0x0e");
	m_ComboxV15.AddString("0x0f");

	//Ipix add string
	m_ComboxIpix.AddString("0x00");
	m_ComboxIpix.AddString("0x01");
	m_ComboxIpix.AddString("0x02");
	m_ComboxIpix.AddString("0x03");
	m_ComboxIpix.AddString("0x04");
	m_ComboxIpix.AddString("0x05");
	m_ComboxIpix.AddString("0x06");
	m_ComboxIpix.AddString("0x07");
	m_ComboxIpix.AddString("0x08");
	m_ComboxIpix.AddString("0x09");
	m_ComboxIpix.AddString("0x0a");
	m_ComboxIpix.AddString("0x0b");
	m_ComboxIpix.AddString("0x0c");
	m_ComboxIpix.AddString("0x0d");
	m_ComboxIpix.AddString("0x0e");
	m_ComboxIpix.AddString("0x0f");

	//Switch add string
	m_ComboxSwitch.AddString("0 high gain");
	m_ComboxSwitch.AddString("1 low gain");

	//TX add string
	m_ComboxTX.AddString("0x00");
	m_ComboxTX.AddString("0x01");
	m_ComboxTX.AddString("0x02");
	m_ComboxTX.AddString("0x03");
	m_ComboxTX.AddString("0x04");
	m_ComboxTX.AddString("0x05");
	m_ComboxTX.AddString("0x06");
	m_ComboxTX.AddString("0x07");
	m_ComboxTX.AddString("0x08");
	m_ComboxTX.AddString("0x09");
	m_ComboxTX.AddString("0x0a");
	m_ComboxTX.AddString("0x0b");
	m_ComboxTX.AddString("0x0c");
	m_ComboxTX.AddString("0x0d");
	m_ComboxTX.AddString("0x0e");
	m_ComboxTX.AddString("0x0f");

	m_ComboxTX2.AddString("0x00");
	m_ComboxTX2.AddString("0x01");
	m_ComboxTX2.AddString("0x02");
	m_ComboxTX2.AddString("0x03");
	m_ComboxTX2.AddString("0x04");
	m_ComboxTX2.AddString("0x05");
	m_ComboxTX2.AddString("0x06");
	m_ComboxTX2.AddString("0x07");
	m_ComboxTX2.AddString("0x08");
	m_ComboxTX2.AddString("0x09");
	m_ComboxTX2.AddString("0x0a");
	m_ComboxTX2.AddString("0x0b");
	m_ComboxTX2.AddString("0x0c");
	m_ComboxTX2.AddString("0x0d");
	m_ComboxTX2.AddString("0x0e");
	m_ComboxTX2.AddString("0x0f");

	m_ComboxTX3.AddString("0x00");
	m_ComboxTX3.AddString("0x01");
	m_ComboxTX3.AddString("0x02");
	m_ComboxTX3.AddString("0x03");
	m_ComboxTX3.AddString("0x04");
	m_ComboxTX3.AddString("0x05");
	m_ComboxTX3.AddString("0x06");
	m_ComboxTX3.AddString("0x07");
	m_ComboxTX3.AddString("0x08");
	m_ComboxTX3.AddString("0x09");
	m_ComboxTX3.AddString("0x0a");
	m_ComboxTX3.AddString("0x0b");
	m_ComboxTX3.AddString("0x0c");
	m_ComboxTX3.AddString("0x0d");
	m_ComboxTX3.AddString("0x0e");
	m_ComboxTX3.AddString("0x0f");

	m_ComboxTX4.AddString("0x00");
	m_ComboxTX4.AddString("0x01");
	m_ComboxTX4.AddString("0x02");
	m_ComboxTX4.AddString("0x03");
	m_ComboxTX4.AddString("0x04");
	m_ComboxTX4.AddString("0x05");
	m_ComboxTX4.AddString("0x06");
	m_ComboxTX4.AddString("0x07");
	m_ComboxTX4.AddString("0x08");
	m_ComboxTX4.AddString("0x09");
	m_ComboxTX4.AddString("0x0a");
	m_ComboxTX4.AddString("0x0b");
	m_ComboxTX4.AddString("0x0c");
	m_ComboxTX4.AddString("0x0d");
	m_ComboxTX4.AddString("0x0e");
	m_ComboxTX4.AddString("0x0f");

	//Amux add string
	m_ComboxAmux.AddString("Vbgr");
	m_ComboxAmux.AddString("Vcm");
	m_ComboxAmux.AddString("V24");
	m_ComboxAmux.AddString("V15");
	m_ComboxAmux.AddString("V20");

	//Test ADC add string
	m_ComboxTestADC.AddString("TestADC");
	m_ComboxTestADC.AddString("None");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BYTE EepromBuff[NUM_EPKT][EPKT_SZ + 1];

LRESULT CTrimDlg::OnTrimProcess(WPARAM wParam, LPARAM lParam)
{
	BYTE type;
	type = RxData[4];	//从接收数据中取出type分支
	int index = RxData[7];		// For 2D EEPROM read command

	CString sTemp1,sTemp2;
	sTemp1.Empty();
	sTemp2.Empty();

	CString sTem;
	sTem.Empty();
	unsigned char cTem[4];
	float * fTem, TempC;

	switch(type)
	{
	case 0x11:		// PCR temp1
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;

			TempC_sum += (*fTem);

//			TempC = 27*(*fTem) - 31.5;			// To do: use trim data from Chan 1

//			sTem.Format("%3.2f",TempC);

//			sTemp1 = sTem;
//			SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP1,sTemp1);
			//清除数据传输buffer
//			memset(RxData,0,sizeof(RxData));
			break;
		}
	case 0x12:		// PCR temp2
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTemp2 = sTem;
			SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP2,sTemp2);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));
			break;
		}
	case 0x2d:		// EEPROM data
	{
		BYTE eeprom_parity = 0;

		for (int i = 0; i < EPKT_SZ + 1; i++) {		// 
			EepromBuff[index][i] = RxData[8 + i];

			if (i < EPKT_SZ) {
				eeprom_parity += RxData[8 + i];
			}
			else {
				if (eeprom_parity != RxData[8 + i]) {
					MessageBox(_T("Packet parity error!"));
				}
			}
		}

		memset(RxData, 0, sizeof(RxData));
		break;
	}
	case 0x0e:		// OSC status
		{
			g_OSCStat = RxData[5];

			CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_PWRSAVE);
			pCheckbox->SetCheck(!g_OSCStat);
			break;
		}
	default: 
		break;
	}
	return 0;
}

void CTrimDlg::ResetTrim(void)
{
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	TrimFlag = sendtrimmsg; 

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x0F;		//data type, dat edit first byte
	TxData[4] = 0x00;		//real data, data edit second byte
	TxData[5] = 0x00;		//real data, data edit third byte
	TxData[6] = 0x13;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();

	// zd add initalize

	m_ComboxV20.SetCurSel(8);
	m_ComboxV15.SetCurSel(8);
//	m_ComboxRampgen.SetCurSel((int)rg_trim);
	m_ComboxRange.SetCurSel(0x0f);
	m_ComboxV24.SetCurSel(8);
	m_ComboxIpix.SetCurSel(8);
	m_ComboxTX.SetCurSel(0x0f);
	m_ComboxTX2.SetCurSel(0x0f);
	m_ComboxSwitch.SetCurSel(0);
	m_ComboxAmux.SetCurSel(0);
	m_ComboxTestADC.SetCurSel(0);

	int i;

	if(g_pTrimReader) {
		for(i=0; i<LAST_PCR; i++) {
			SelSensor(i+1);
			SetRampgen((char)g_pTrimReader->Node[i].rampgen); 	
			SetRange((char)g_pTrimReader->Node[i].range);
		}
		m_ComboxRampgen.SetCurSel((int)g_pTrimReader->Node[0].rampgen); 			// To do: how to handle this? depend on SensorSel.
	}

	SetIntTime(1);
	SetIntTime(2);
	SetIntTime(3);
	SetIntTime(4);

	if (!g_FrameSize) {
		CommitTxBin();
	}
	else {
		ResetTxBin();
	}

	for(i=0; i<LAST_PCR; i++) {
		SetGainMode(gain_mode, i+1);		// restore gui version

		SelSensor(i+1);

		if (g_pTrimReader) {
			if (!gain_mode)
				SetV20(g_pTrimReader->Node[i].auto_v20[1], i + 1);
			else
				SetV20(g_pTrimReader->Node[i].auto_v20[0], i + 1);

			SetV15(g_pTrimReader->Node[i].auto_v15);
		}
	}

	g_pGraDlg->SetChannelMask();			// for Autosnap;

	GetOSCStat();
	OnBnClickedTrimBtnPcrtemp1();
}

void CTrimDlg::CommitTxBin(void)
{
	m_ComboxTX.SetCurSel(tx_bin);
	m_ComboxTX2.SetCurSel(tx_bin2);
	m_ComboxTX3.SetCurSel(tx_bin3);
	m_ComboxTX4.SetCurSel(tx_bin4);

	SelSensor(1);
	SetTXbin(tx_bin);

	SelSensor(2);
	SetTXbin(tx_bin2);

	SelSensor(3);
	SetTXbin(tx_bin3);

	SelSensor(4);
	SetTXbin(tx_bin4);
}


void CTrimDlg::OnCbnSelchangeComboRampgen()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRampgen.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxRampgen.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);		//将获取的字符串转成16进制字符

	SetRampgen(dataEdit[0]);
}


void CTrimDlg::OnCbnSelchangeComboRange()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRange.GetCurSel();			//获取combox中被选中item的行号

	CString strText;
	m_ComboxRange.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	SetRange(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboV24()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxV24.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxV24.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnCbnSelchangeComboV20()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxV20.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxV20.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);

	SetV20(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboV15()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	int nIndex = m_ComboxV15.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxV15.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SelSensor(1);
	SetV15(dataEdit[1]);
	SelSensor(2);
	SetV15(dataEdit[1]);
	SelSensor(3);
	SetV15(dataEdit[1]);
	SelSensor(4);
	SetV15(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboIpix()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxIpix.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxIpix.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x06;		//data type, date edit first byte
	TxData[4] = dataEdit[1];	//real data, date edit second byte
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}

// Note: Change gain mode here may cause V20 not to be set up correctly for DARK MANAGEMENT. 
// This is accepted since this feature is for internal debug use only

void CTrimDlg::OnCbnSelchangeComboSwitch()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	int nIndex = m_ComboxSwitch.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxSwitch.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中

	if (strText == "0 high gain")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
		TrimCalMainMsg();
		gain_mode = 0;
	}
	else	
	{	
		if (strText == "1 low gain")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
			//0x01 means send vedio data
			//0x00 means stop vedio data
			TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
			if (TxData[5]==0x17)
				TxData[5]=0x18;
			else
				TxData[5]=TxData[5];
			TxData[6] = 0x17;		//back code
			TxData[7] = 0x17;		//back code

			//Call maindlg message
			TrimCalMainMsg();
			gain_mode = 1;
		}
		else
			MessageBox("please check the right item");
	}	
}


void CTrimDlg::OnCbnSelchangeComboTxbin()
{
	// TODO: Add your control notification handler code here



	int nIndex = m_ComboxTX.GetCurSel();		//

	CString strText;
	m_ComboxTX.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	tx_bin = dataEdit[1];

	if (!g_DeviceDetected) {
		return;
	}

	SelSensor(1);
	SetTXbin(dataEdit[1]);


}

void CTrimDlg::OnSelchangeComboTxbin2()
{
	int nIndex = m_ComboxTX2.GetCurSel();		//

	CString strText;
	m_ComboxTX2.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	tx_bin2 = dataEdit[1];

	if (!g_DeviceDetected) {
		return;
	}

	SelSensor(2);
	SetTXbin(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboTxbin3()
{
	int nIndex = m_ComboxTX3.GetCurSel();		//

	CString strText;
	m_ComboxTX3.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	tx_bin3 = dataEdit[1];

	if (!g_DeviceDetected) {
		return;
	}

//	SelSensor(1);
//	SetTXbin(dataEdit[1]);
//	SelSensor(2);
//	SetTXbin(dataEdit[1]);
	SelSensor(3);
	SetTXbin(dataEdit[1]);
//	SelSensor(4);
//	SetTXbin(dataEdit[1]);
}


void CTrimDlg::OnCbnSelchangeComboTxbin4()
{
	int nIndex = m_ComboxTX4.GetCurSel();		//

	CString strText;
	m_ComboxTX4.GetLBText(nIndex,strText);		//

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);
	tx_bin4 = dataEdit[1];

	if (!g_DeviceDetected) {
		return;
	}

//	SelSensor(1);
//	SetTXbin(dataEdit[1]);
//	SelSensor(2);
//	SetTXbin(dataEdit[1]);
//	SelSensor(3);
//	SetTXbin(dataEdit[1]);
	SelSensor(4);
	SetTXbin(dataEdit[1]);
}

void CTrimDlg::OnCbnSelchangeComboAmuxsel()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	int nIndex = m_ComboxAmux.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxAmux.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer


	if (strText == "Vbgr")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x09;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
		TrimCalMainMsg();
	}	
	else
	{
		if (strText == "Vcm")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x09;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
			//0x01 means send vedio data
			//0x00 means stop vedio data
			TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
			if (TxData[5]==0x17)
				TxData[5]=0x18;
			else
				TxData[5]=TxData[5];
			TxData[6] = 0x17;		//back code
			TxData[7] = 0x17;		//back code

			//Call maindlg message
			TrimCalMainMsg();
		}
		else
		{
			if (strText == "V24")
			{
				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x01;		//command
				TxData[2] = 0x02;		//data length
				TxData[3] = 0x09;		//data type, date edit first byte
				TxData[4] = 0x02;		//real data, date edit second byte
				//0x01 means send vedio data
				//0x00 means stop vedio data
				TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
				if (TxData[5]==0x17)
					TxData[5]=0x18;
				else
					TxData[5]=TxData[5];
				TxData[6] = 0x17;		//back code
				TxData[7] = 0x17;		//back code

				//Call maindlg message
				TrimCalMainMsg();
			}
			else
			{
				if (strText == "V15")
				{
					TxData[0] = 0xaa;		//preamble code
					TxData[1] = 0x01;		//command
					TxData[2] = 0x02;		//data length
					TxData[3] = 0x09;		//data type, date edit first byte
					TxData[4] = 0x03;		//real data, date edit second byte
					//0x01 means send vedio data
					//0x00 means stop vedio data
					TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
					if (TxData[5]==0x17)
						TxData[5]=0x18;
					else
						TxData[5]=TxData[5];
					TxData[6] = 0x17;		//back code
					TxData[7] = 0x17;		//back code

					//Call maindlg message
					TrimCalMainMsg();
				}
				else
				{
					if (strText == "V20")
					{
						TxData[0] = 0xaa;		//preamble code
						TxData[1] = 0x01;		//command
						TxData[2] = 0x02;		//data length
						TxData[3] = 0x09;		//data type, date edit first byte
						TxData[4] = 0x04;		//real data, date edit second byte
						//0x01 means send vedio data
						//0x00 means stop vedio data
						TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
						if (TxData[5]==0x17)
							TxData[5]=0x18;
						else
							TxData[5]=TxData[5];
						TxData[6] = 0x17;		//back code
						TxData[7] = 0x17;		//back code

						//Call maindlg message
						TrimCalMainMsg();
					}
					else
						MessageBox("please check right item");
				}
			}
		}

	}
}


void CTrimDlg::OnCbnSelchangeComboTestadc()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	int nIndex = m_ComboxTestADC.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxTestADC.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中


	if (strText == "TestADC")
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x0a;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
		TrimCalMainMsg();
	}
	else	
	{	
		if (strText == "None")
		{
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x0a;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
			//0x01 means send vedio data
			//0x00 means stop vedio data
			TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
			if (TxData[5]==0x17)
				TxData[5]=0x18;
			else
				TxData[5]=TxData[5];
			TxData[6] = 0x17;		//back code
			TxData[7] = 0x17;		//back code

			//Call maindlg message
			TrimCalMainMsg();
		}
		else
			MessageBox("please check the right item");
	}
}

// For the protocol debug interface

void CTrimDlg::OnBnClickedBtnSendcmd()
{
	CString sData, sCmd, sType;
	char* cData ="";
	char* cCmd = "";
	char* cType = "";
	int d_Num;

	TrimFlag = sendtrimmsg;

	memset(TxData,0,sizeof(TxData));

	GetDlgItemText(IDC_EDIT_CMD,sCmd);
	d_Num = (sCmd.GetLength())/2;
	cCmd = EditDataCvtChar(sCmd,cCmd);

	GetDlgItemText(IDC_EDIT_TYPE,sType);
	d_Num = (sType.GetLength())/2;
	cType = EditDataCvtChar(sType,cType);

	GetDlgItemText(IDC_EDIT_Data,sData);
	d_Num = (sData.GetLength())/2;

	cData = EditDataCvtChar(sData,cData);

	TxData[0] = 0xaa;
	TxData[1]= cCmd[0];
	TxData[2]= d_Num+1;
	TxData[3]= cType[0];
	for(int i=0; i<d_Num; i++)
	{
		TxData[i+4] = cData[i];
	}
	for(int j=1; j<(d_Num+4); j++)
	{
		TxData[d_Num+4] += TxData[j];
	}
	TxData[d_Num+5] = 0x17;
	TxData[d_Num+6] = 0x17;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::OnBnClickedBtnShowdata()
{
	// TODO: 
	SetDlgItemText(IDC_EDIT_REDATA,RegRecStr);
}

void CTrimDlg::OnBnClickedBtnItgtim()
{
	IntTimeDlg intDlg;
	intDlg.m_IntTime = int_time;

	if (intDlg.DoModal() == IDOK)
	{
		int_time = intDlg.m_IntTime;
	}

	SetIntTime(1);
}

void CTrimDlg::OnBnClickedBtnItgtim2()
{
	IntTimeDlg intDlg;
	intDlg.m_IntTime = int_time2;

	if (intDlg.DoModal() == IDOK)
	{
		int_time2 = intDlg.m_IntTime;
	}

	SetIntTime(2);
}

void CTrimDlg::OnBnClickedBtnItgtim3()
{
	IntTimeDlg intDlg;
	intDlg.m_IntTime = int_time3;

	if (intDlg.DoModal() == IDOK)
	{
		int_time3 = intDlg.m_IntTime;
	}

	SetIntTime(3);
}

void CTrimDlg::OnBnClickedBtnItgtim4()
{
	IntTimeDlg intDlg;
	intDlg.m_IntTime = int_time4;

	if (intDlg.DoModal() == IDOK)
	{
		int_time4 = intDlg.m_IntTime;
	}

	SetIntTime(4);
}

void CTrimDlg::SetIntTime(int ch)
{
	CString kstg;

	switch(ch) {
	case 1:
		kstg.Format("%4.2f", int_time);
		SetDlgItemText(IDC_EDIT_IGTIME,kstg);	
		break;
	case 2:
		kstg.Format("%4.2f", int_time2);
		SetDlgItemText(IDC_EDIT_IGTIME2,kstg);	
		break;
	case 3:
		kstg.Format("%4.2f", int_time3);
		SetDlgItemText(IDC_EDIT_IGTIME3,kstg);	
		break;
	case 4:
		kstg.Format("%4.2f", int_time4);
		SetDlgItemText(IDC_EDIT_IGTIME4,kstg);	
		break;	
	}

	// Commit to HW 

	if (!g_DeviceDetected)
		return;

	if(g_EnableRemap) 
		UpdateRemapTable();

	SelSensor(ch);

	unsigned char * hData; 

	switch(ch) {
	case 1:
		hData  = (unsigned char *) & int_time;
		break;
	case 2:
		hData  = (unsigned char *) & int_time2;
		break;
	case 3:
		hData  = (unsigned char *) & int_time3;
		break;
	case 4:
		hData  = (unsigned char *) & int_time4;
		break;	}

	TrimFlag = sendtrimmsg;

	TrimBuf[0] = hData[0];	//存到窗口传递buffer
	TrimBuf[1] = hData[1];
	TrimBuf[2] = hData[2];
	TrimBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x20;		//data type, date edit first byte
	TxData[4] = TrimBuf[0];		//real data, date edit second byte
	TxData[5] = TrimBuf[1];
	TxData[6] = TrimBuf[2];
	TxData[7] = TrimBuf[3];
	TxData[8] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7];		//check sum
	if (TxData[8]==0x17)
		TxData[8]=0x18;
//	else
//		TxData[8]=TxData[8];
	TxData[9] = 0x17;		//back code
	TxData[10] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetRange(char range)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x02;		//data type, date edit first byte
	TxData[4] = range;	//real data, date edit second byte
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
//	else
//		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetV20(char v20)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte
	TxData[4] = v20;		//real data, date edit second byte
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
//	else
//		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetV15(char v15)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x05;		//data type, date edit first byte
	TxData[4] = v15;	//real data, date edit second byte
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
//	else
//		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::SetRampgen(char rampgen)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = rampgen;	//real data, date edit second byte
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
//	else
//		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetTXbin(char txbin)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = txbin;	//real data, date edit second byte
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
//	else
//		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimFlag = sendtrimmsg;

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::ResetTxBin(void)
{
//	m_ComboxTX.SetCurSel(8);
//	m_ComboxTX2.SetCurSel(8);
//	m_ComboxTX3.SetCurSel(8);
//	m_ComboxTX4.SetCurSel(8);
}

/////// Zhimin Ding added: read temperature 4 times and then average;


void CTrimDlg::OnBnClickedTrimBtnPcrtemp1()
{
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached", "ULVision", MB_ICONWARNING);
		return;
	}
	
	TempC_sum = 0;
	
	for(int i=0; i<T_REPEAT; i++)	{
		SelSensor(1); 
		GetTemp();
	}

	float TempC;
	CString sTemp, sTem;

	sTemp.Empty();
	sTem.Empty();

	if(g_pTrimReader) 
		TempC = (float)(g_pTrimReader->Node[0].tempcal[0] * (TempC_sum / T_REPEAT) + g_pTrimReader->Node[0].tempcal[1]);
	else 
		TempC = TempC_sum / T_REPEAT;

	sTem.Format("%.2f", TempC);

	sTemp = sTem;

	SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP1,sTemp);
	//清除数据传输buffer
	memset(RxData,0,sizeof(RxData));

	g_FrameTemp = TempC;
}	
	
void CTrimDlg::GetTemp()
{
	// TODO: Add your control notification handler code here
	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x11;		//data type
	TxData[4] = 0x00;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedTrimBtnPcrtemp2()
{
	// TODO: Add your control notification handler code here
	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x12;		//data type
	TxData[4] = 0x01;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::OnClickedRadioTrimSensor1()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x00;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::OnBnClickedRadioTrimSensor2()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x01;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedRadioTrimSensor3()
{
	// TODO: Add your control notification handler code here
	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x02;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::OnBnClickedRadioTrimSensor4()
{
	// TODO: Add your control notification handler code here

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = 0x03;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}


void CTrimDlg::SelSensor(BYTE i)
{
	// TODO: Add your control notification handler code here

	if(i < 1 || i > 4) return;

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = i - 1;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

// Not used. Just use selsensor for channel select.
void CTrimDlg::SelSensor4Read(BYTE i)
{
	// TODO: Add your control notification handler code here

	if(i < 1 || i > 4) return;

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x26;		//data type
	TxData[4] = i - 1;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

void CTrimDlg::SetV20(char v20, char i)
{
	SelSensor(i);
	SetV20(v20);
}

// Auto repeat properties

void CTrimDlg::OnBnClickedButton3()
{
	CKineticDlg kinDlg;

	kinDlg.m_Interval = videoElapseTime;
	kinDlg.m_Maxcnt = autoRepeatMaxCount;

	if (kinDlg.DoModal() == IDOK)
	{
		videoElapseTime = kinDlg.m_Interval;
		autoRepeatMaxCount = kinDlg.m_Maxcnt;
	}
}

void CTrimDlg::SetGainMode(int gain)	// gain - 1: low gain; 0: high gain
{
	TrimFlag = sendtrimmsg;

	if (!gain)	// high gain
	{
		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x07;		//data type, date edit first byte
		TxData[4] = 0x00;	//real data, date edit second byte
		//0x01 means send vedio data
		//0x00 means stop vedio data
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		//Call maindlg message
		TrimCalMainMsg();
		gain_mode = 0;
	}
	else if(gain)	// low gain
	{	
			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x01;		//command
			TxData[2] = 0x02;		//data length
			TxData[3] = 0x07;		//data type, date edit first byte
			TxData[4] = 0x01;	//real data, date edit second byte
			//0x01 means send vedio data
			//0x00 means stop vedio data
			TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
			if (TxData[5]==0x17)
				TxData[5]=0x18;
			else
				TxData[5]=TxData[5];
			TxData[6] = 0x17;		//back code
			TxData[7] = 0x17;		//back code

			//Call maindlg message
			TrimCalMainMsg();
			gain_mode = 1;
	}	
}

void CTrimDlg::SetGainMode(int gain, char i)	// gain - 0: low gain; 1: high gain
{
	SelSensor(i);
	SetGainMode(gain);
}


void CTrimDlg::OnBnClickedPwrsave()
{
	// TODO: Add your control notification handler code here

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_PWRSAVE);

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x0e;		//data type, date edit first byte
	TxData[4] = (pCheckbox->GetCheck())? 0:1;		//real data, date edit second byte

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();

	g_OSCStat = !(pCheckbox->GetCheck());
}

void CTrimDlg::GetOSCStat()
{
	// TODO: Add your control notification handler code here
	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x0e;		//data type
	TxData[4] = 0x00;		//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5];		//check sum
	if (TxData[6]==0x17)
		TxData[6]=0x18;
	else
		TxData[6]=TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
}

//////////////////////////////////////////////////
// New code to support Remap feature
//////////////////////////////////////////////////

void CTrimDlg::SetChannelMap(BYTE channel_index,  BYTE chip_index, BYTE led_index, BYTE frame_size, float int_time)
{
	BYTE pCmd = 0x01;
	BYTE pType = 0x2A;
	BYTE pData = 0x01;		// total channel to set is 1;
	
	unsigned char *hData;
	BYTE hBuf[4];
	
	hData = (unsigned char *)& int_time;

	hBuf[0] = hData[0];	//
	hBuf[1] = hData[1];
	hBuf[2] = hData[2];
	hBuf[3] = hData[3];

	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;				//preamble code
	TxData[1] = pCmd;				//command
	TxData[2] = 0x0C;				//data length
	TxData[3] = pType;				//data type, date edit first byte
	TxData[4] = pData;				// real data, # of channels to set
	TxData[5] = channel_index;		// first channel number
	TxData[6] = chip_index;
	TxData[7] = led_index;
	TxData[8] = frame_size;
	TxData[9] = hBuf[0];
	TxData[10] = hBuf[1];
	TxData[11] = hBuf[2];
	TxData[12] = hBuf[3];
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1] + TxData[2] + TxData[3] + TxData[4] + TxData[5] + TxData[6] + TxData[7] + TxData[8] + TxData[9]
		+ TxData[10] + TxData[11] + TxData[12] + TxData[13] + TxData[14];		//check sum
	if (TxData[15] == 0x17)
		TxData[15] = 0x18;
	else
		TxData[15] = TxData[15];
	TxData[16] = 0x17;				//back code
	TxData[17] = 0x17;				//back code

	TrimCalMainMsg();
}

void CTrimDlg::OnBnClickedBtnChanset()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	CChanSetDlg chanDlg;

	chanDlg.m_ChanIndex = chan;
	chanDlg.m_ChipIndex = chip;
	chanDlg.m_LedIndex = led;
	chanDlg.m_IntTime = int_time;

	chanDlg.m_ChanIndex1 = chan_1;
	chanDlg.m_ChipIndex1 = chip_1;
	chanDlg.m_LedIndex1 = led_1;
	chanDlg.m_IntTime1 = int_time2;

	chanDlg.m_RemapEn = remap_en;

	if (chanDlg.DoModal() == IDOK)
	{
		chan = chanDlg.m_ChanIndex;
		chip = chanDlg.m_ChipIndex;
		led = chanDlg.m_LedIndex;
//		int_time = chanDlg.m_IntTime;

		chan_1 = chanDlg.m_ChanIndex1;
		chip_1 = chanDlg.m_ChipIndex1;
		led_1 = chanDlg.m_LedIndex1;
//		int_time2 = chanDlg.m_IntTime1;

		remap_en = chanDlg.m_RemapEn;
	}

	SetChannelMap(chan, chip, led, g_FrameSize, int_time);
	SetChannelMap(chan_1, chip_1, led_1, g_FrameSize, int_time2);

//	SetChannelMap(0, 0, 0, 0, 10);
//	SetChannelMap(1, 0, 1, 0, 10);
//	remap_en = 1;

	EnableRemap(remap_en);
	g_EnableRemap = remap_en;
}

void CTrimDlg::EnableRemap(BYTE en)
{
	// TODO: Add your control notification handler code here
	TrimFlag = sendtrimmsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;		//data length
	TxData[3] = 0x29;		//data type
	TxData[4] = en;			//real data
	TxData[5] = 0x00;
	TxData[6] = TxData[1] + TxData[2] + TxData[3] + TxData[4] + TxData[5];		//check sum
	if (TxData[6] == 0x17)
		TxData[6] = 0x18;
	else
		TxData[6] = TxData[6];
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code
							
	TrimCalMainMsg();		//Call maindlg message
}

void CTrimDlg::UpdateRemapTable()
{
	SetChannelMap(chan, chip, led, g_FrameSize, int_time);
	SetChannelMap(chan_1, chip_1, led_1, g_FrameSize, int_time2);

	EnableRemap(remap_en);
	g_EnableRemap = remap_en;
}

//=======End Remap support==================

//=============== EEPROM write =======================

// only support 4 packets, 50 byte per packet

void CTrimDlg::EEPROMWrite(int chan, int total_packets, int packet_index, BYTE* packet_data, int packet_size)
{
	TrimFlag = sendtrimmsg;

	int len = packet_size + 4;
	BYTE check_sum = 0;
	int i;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = len;		//data length
	TxData[3] = 0x2d;		//data type, date edit first byte
	TxData[4] = (BYTE)chan;		//real data, date edit second byte
	TxData[5] = (BYTE)total_packets;
	TxData[6] = (BYTE)packet_index;

	for (i = 0; i < packet_size; i++) {
		TxData[7 + i] = packet_data[i];
	}

	for (i = 1; i < 7 + packet_size; i++) {
		check_sum += TxData[i];
	}

	TxData[7 + packet_size] = check_sum;		//check sum

	if (TxData[7 + packet_size] == 0x17) {
		TxData[7 + packet_size] = 0x18;
	}

	TxData[8 + packet_size] = 0x17;			//back code
	TxData[9 + packet_size] = 0x17;			//back code

	TrimCalMainMsg();						//Call maindlg message
}

void CTrimDlg::EEPROMRead(int chan)
{
	TrimFlag = sendeeprommsg;			// only the eeprom read use this message type

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x2d;		//data type
	TxData[4] = (BYTE)chan;		//real data
								//	TxData[5] = 0x00;
	TxData[5] = TxData[1] + TxData[2] + TxData[3] + TxData[4];		//check sum
	if (TxData[5] == 0x17)
		TxData[5] = 0x18;
	else
		TxData[5] = TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	TrimCalMainMsg();		//Call maindlg message
}


void CTrimDlg::OnBnClickedButtonSavconf()
{
	// TODO: Add your control notification handler code here

	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 }, szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };;

	GetModuleFileNameA(NULL, szFilePath, sizeof(szFilePath));

	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);

	CString sConfigFile(szDir);
	sConfigFile += CONFIGFILE;

	//construct json data
	Json::Value root;

	root["IntTimeCh1"] = Json::Value(int_time);
	root["IntTimeCh2"] = Json::Value(int_time2);
	root["IntTimeCh3"] = Json::Value(int_time3);
	root["IntTimeCh4"] = Json::Value(int_time4);

	root["GainMode"] = Json::Value(gain_mode);
	root["FrameSize"] = Json::Value(g_FrameSize);

	root["TxBin1"] = Json::Value(tx_bin);
	root["TxBin2"] = Json::Value(tx_bin2);
	root["TxBin3"] = Json::Value(tx_bin3);
	root["TxBin4"] = Json::Value(tx_bin4);

	root["ChanMask1"] = Json::Value(g_ChannelMask[0]);
	root["ChanMask2"] = Json::Value(g_ChannelMask[1]);
	root["ChanMask3"] = Json::Value(g_ChannelMask[2]);
	root["ChanMask4"] = Json::Value(g_ChannelMask[3]);

	root["PipeMode"] = Json::Value(g_pGraDlg->m_PipelineMode);

	root["KineCount"] = Json::Value(autoRepeatMaxCount);
	root["KineTime"] = Json::Value(videoElapseTime);
	root["NumChipsInstalled"] = Json::Value(LAST_PCR);

	if (g_pGraDlg) {
		root["ContrastSliderPos"] = Json::Value(g_pGraDlg->m_SliderPos);
		root["VFlip1"] = Json::Value(g_pGraDlg->m_flip1);
		root["VFlip2"] = Json::Value(g_pGraDlg->m_flip2);
		root["VFlip3"] = Json::Value(g_pGraDlg->m_flip3);
		root["VFlip4"] = Json::Value(g_pGraDlg->m_flip4);

		root["ShowRowNum"] = Json::Value(g_pGraDlg->m_row_number);
		root["ShowTimeStamp"] = Json::Value(g_pGraDlg->m_time_stamp);
		root["ShowCaptureSetting"] = Json::Value(g_pGraDlg->m_capture_setting);
		root["ShowFrameAverage"] = Json::Value(g_pGraDlg->m_frame_average);
	}

	// Write to json file

	Json::FastWriter json_writer;
	std::string strJsonResult = json_writer.write(root);

	//Save Config to file
	std::ofstream outfile;

	outfile.open(sConfigFile);
	outfile << strJsonResult;
	outfile.close();
}


void CTrimDlg::Loadconf()
{
	//get config file path
	char szFilePath[MAX_PATH] = { 0 }, szDrive[MAX_PATH] = { 0 }, szDir[MAX_PATH] = { 0 }, szFileName[MAX_PATH] = { 0 }, szExt[MAX_PATH] = { 0 };;
	GetModuleFileNameA(NULL, szFilePath, sizeof(szFilePath));

	_splitpath(szFilePath, szDrive, szDir, szFileName, szExt);
	CString sConfigFile(szDir);
	sConfigFile += CONFIGFILE;

	//wether the config file is existing or not
	DWORD dwAttrib = GetFileAttributes(sConfigFile);
	if (INVALID_FILE_ATTRIBUTES == dwAttrib || 1 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
	{//file not exist
		AfxMessageBox("Can't find the configuration json file");
		return;
	}

	//Parse the config file
	// CParaConfig paraConfig = 

	ParseJsonFromFile(sConfigFile);
}

// parse para from file
void CTrimDlg::ParseJsonFromFile(CString filename)
{
	// Json::Reader   
	Json::Reader reader;
	// Json::Value int, string, object, array...   
	Json::Value root;
	//	CParaConfig paraConfig;
	std::ifstream is;
	is.open(filename, std::ios::binary);

	if (reader.parse(is, root))
	{
		float	tempFloat = 1;
		int		tempInt = 0;
		BOOL	tempBool = false;

		tempFloat = root.get("IntTimeCh1", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
		int_time = tempFloat;

		tempFloat = root.get("IntTimeCh2", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
		int_time2 = tempFloat;

		tempFloat = root.get("IntTimeCh3", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
		int_time3 = tempFloat;

		tempFloat = root.get("IntTimeCh4", tempFloat).asFloat(); // Return the member named key if it exist, tempFloat otherwise.
		int_time4 = tempFloat;

		SetIntTime(1);
		SetIntTime(2);
		SetIntTime(3);
		SetIntTime(4);

		tempInt = root.get("GainMode", tempInt).asInt();
		int gm = tempInt;

		g_pGraDlg->SetGainModeGUI(gm);

		tempInt = root.get("TxBin1", tempInt).asInt();
		tx_bin = tempInt;

		tempInt = root.get("TxBin2", tempInt).asInt();
		tx_bin2 = tempInt;

		tempInt = root.get("TxBin3", tempInt).asInt();
		tx_bin3 = tempInt;

		tempInt = root.get("TxBin4", tempInt).asInt();
		tx_bin4 = tempInt;

		m_ComboxTX.SetCurSel(tx_bin);
		m_ComboxTX2.SetCurSel(tx_bin2);
		m_ComboxTX3.SetCurSel(tx_bin3);
		m_ComboxTX4.SetCurSel(tx_bin4);

		// Not needed as will do this when set frame size
/*		if (g_DeviceDetected) {
			if (!g_FrameSize) {
				CommitTxBin();
			}
			else {
				ResetTxBin();
			}
		}
*/

		tempInt = root.get("FrameSize", tempInt).asInt();
		int fs = tempInt;

		g_pGraDlg->SetFrameSizeGUI(fs);

		tempBool = g_ChannelMask[0];
		tempBool = root.get("ChanMask1", tempBool).asBool();
		g_ChannelMask[0] = tempBool;
		tempBool = false;
		tempBool = root.get("ChanMask2", tempBool).asBool();
		g_ChannelMask[1] = tempBool;
		tempBool = root.get("ChanMask3", tempBool).asBool();
		g_ChannelMask[2] = tempBool;
		tempBool = root.get("ChanMask4", tempBool).asBool();
		g_ChannelMask[3] = tempBool;
		g_pGraDlg->SetChannelMaskGUI();

		tempBool = false;
		tempBool = root.get("PipeMode", tempBool).asBool();
		g_pGraDlg->m_PipelineMode = tempBool;
		g_pGraDlg->SetPipelineModeGUI();

		tempInt = autoRepeatMaxCount;
		tempInt = root.get("KineCount", tempInt).asInt();
		autoRepeatMaxCount = tempInt;
		tempInt = videoElapseTime;
		tempInt = root.get("KineTime", tempInt).asInt();
		videoElapseTime = tempInt;

		tempInt = root.get("NumChipsInstalled", tempInt).asInt();
		LAST_PCR = tempInt;

		tempInt = root.get("ContrastSliderPos", tempInt).asInt();
		g_pGraDlg->m_SliderPos = tempInt;
		g_pGraDlg->SetContrast();

		tempBool = false;
		tempBool = root.get("VFlip1", tempBool).asBool();
		g_pGraDlg->m_flip1 = tempBool;

		tempBool = false;
		tempBool = root.get("VFlip2", tempBool).asBool();
		g_pGraDlg->m_flip2 = tempBool;

		tempBool = false;
		tempBool = root.get("VFlip3", tempBool).asBool();
		g_pGraDlg->m_flip3 = tempBool;

		tempBool = false;
		tempBool = root.get("VFlip4", tempBool).asBool();
		g_pGraDlg->m_flip4 = tempBool;

		tempBool = false;
		tempBool = root.get("ShowRowNum", tempBool).asBool();
		g_pGraDlg->m_row_number = tempBool;

		tempBool = false;
		tempBool = root.get("ShowTimeStamp", tempBool).asBool();
		g_pGraDlg->m_time_stamp = tempBool;

		tempBool = false;
		tempBool = root.get("ShowCaptureSetting", tempBool).asBool();
		g_pGraDlg->m_capture_setting = tempBool;

		tempBool = false;
		tempBool = root.get("ShowFrameAverage", tempBool).asBool();
		g_pGraDlg->m_frame_average = tempBool;
	}

	is.close();
}


void CTrimDlg::OnBnClickedButtonLoadconf()
{
	Loadconf();
}


void CTrimDlg::OnBnClickedButtonTrimtest()
{
	// TODO: Add your control notification handler code here
	int col, ecode;
	BYTE hb, lb;
	int result;
	CString str;

	GetDlgItemText(IDC_EDIT_HB, str);
	hb = (BYTE)_tcstol(str, NULL, 16);
	GetDlgItemText(IDC_EDIT_LB, str);
	lb = (BYTE)_tcstol(str, NULL, 16);
	GetDlgItemText(IDC_EDIT_COL, str);
	col = atoi(str);		// col 1 based

	g_pTrimReader->SetCalib2(1);
	result = g_pTrimReader->ADCCorrection(col - 1, hb, lb, 12, 1, 1, &ecode);
	g_pTrimReader->SetCalib2(0);
	str.Format("%d, 0x%x (%d)", result, result, ecode);

	SetDlgItemText(IDC_EDIT_RESULT, str);
}

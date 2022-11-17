// TrimDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "TrimDlg.h"
#include "IntTimeDlg.h"
#include "KineticDlg.h"
#include "afxdialogex.h"

//*****************************************************************
//Constant definition
//*****************************************************************


//***************************************************************
//Global variable definition
//***************************************************************
int TrimFlag;	// register dialog flag

BYTE TrimBuf[trimdatanum];	// trim dialog transmitted data buffer

BYTE TimCom;		//command下拉框的取值buffer
BYTE TimType;		//type下拉框的取值buffer

BYTE SensorRadioValue;		// sensor radio select value

//*****************************************************************
//TrimDlg global function definition
//*****************************************************************


//*****************************************************************
//External variable definition
//*****************************************************************


//****************************************************
//External function 
//****************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
extern int ChangeNum (CString str, int length);									//十六进制字符串转十进制整型
extern  char* EditDataCvtChar (CString strCnv,  char * charRec);	//编辑框取值转字符变量

int gain_mode = 0; // zd add, 0 high gain, 1 low gain
float int_time = 1.0; // zd add, in ms

extern BOOL g_DeviceDetected;

extern unsigned int videoElapseTime;
extern unsigned int autoRepeatMaxCount;

// CTrimDlg dialog

IMPLEMENT_DYNAMIC(CTrimDlg, CDialog)

CTrimDlg::CTrimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTrimDlg::IDD, pParent)
{

	m_radio_trimdlg_isensor = 0;
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

int myint = 13;

void CTrimDlg::OnBnClickedBtnReread()
{
	// TODO: Add your control notification handler code here

/*	TrimFlag = sendtrimmsg; 
	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x01;			//data length
	TxData[3] = 0x05;	//data type, dat edit first byte
	TxData[4] = 0x00;	//real data, data edit second byte
	TxData[5] = 0x08;	//real data, data edit third byte
	TxData[6] = 0x17;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();
*/
	// TODO: Add your control notification handler code here
	ResetTrim();

/*	IntTimeDlg intDlg;

	intDlg.m_IntTime = myint;

	if (intDlg.DoModal() == IDOK)
	{
	myint = intDlg.m_IntTime;
	}
*/
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


LRESULT CTrimDlg::OnTrimProcess(WPARAM wParam, LPARAM lParam)
{
	BYTE type;
	type = RxData[4];	//从接收数据中取出type分支

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

			TempC = 27*(*fTem) - 31.5;

			sTem.Format("%3.2f",TempC);

			sTemp1 = sTem;
			SetDlgItemTextA(IDC_TRIM_EDIT_PCRTEMP1,sTemp1);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));
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
	default: 
		break;
	}
	return 0;
}


#define B15

#if defined(A11)

	const unsigned char rg_trim = 0xb8;

#elif defined(A12)

	const unsigned char rg_trim = 0xc8;

#elif defined(B14)

	const unsigned char rg_trim = 0xB0;

#elif defined(B15)

	const unsigned char rg_trim = 0xDA;

#endif

void CTrimDlg::ResetTrim(void)
{
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	TrimFlag = sendtrimmsg; 

	TxData[0] = 0xaa;			//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x03;			//data length
	TxData[3] = 0x0F;	//data type, dat edit first byte
	TxData[4] = 0x00;	//real data, data edit second byte
	TxData[5] = 0x00;	//real data, data edit third byte
	TxData[6] = 0x13;		//check sum
	TxData[7] = 0x17;		//back code
	TxData[8] = 0x17;		//back code

	//Call maindlg message
	TrimCalMainMsg();

	// zd add initalize

	SelSensor(1);
	SetRampgen(0xD8);
	SetRange(0x0f);
	SelSensor(2);
	SetRampgen(0xB0);
	SetRange(0x0f);

	SelSensor(3);
	SetRampgen(0xD4);
	SetRange(0x0f);
	SelSensor(4);
	SetRampgen(0xC8);
	SetRange(0x0f);

	m_ComboxV20.SetCurSel(8);
	m_ComboxV15.SetCurSel(8);
	m_ComboxRampgen.SetCurSel((int)rg_trim);
	m_ComboxRange.SetCurSel(0x0f);
	m_ComboxV24.SetCurSel(8);
	m_ComboxIpix.SetCurSel(8);
	m_ComboxTX.SetCurSel(8);
	m_ComboxSwitch.SetCurSel(0);
	m_ComboxAmux.SetCurSel(0);
	m_ComboxTestADC.SetCurSel(0);

//	SetDlgItemText(IDC_EDIT_IGTIME,"1");		//从编辑框获取数值字符串

	int_time = 1;

	CString kstg;
//	float kfl;

	kstg.Format("%4.3f", int_time);
	SetDlgItemText(IDC_EDIT_IGTIME,kstg);	


//=======================
}



void CTrimDlg::OnCbnSelchangeComboRampgen()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRampgen.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxRampgen.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";
	dataEdit = EditDataCvtChar(strText, dataEdit);	//将获取的字符串转成16进制字符

	SetRampgen(dataEdit[0]);
}


void CTrimDlg::OnCbnSelchangeComboRange()
{
	// TODO: Add your control notification handler code here

	int nIndex = m_ComboxRange.GetCurSel();		//获取combox中被选中item的行号

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

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}


	int nIndex = m_ComboxTX.GetCurSel();		//获取combox中被选中item的行号

	CString strText;
	m_ComboxTX.GetLBText(nIndex,strText);		//将选中的item的内容以字符串的格式放入buffer中
	//strText是buffer

	char* dataEdit="";

	dataEdit = EditDataCvtChar(strText, dataEdit);

	SelSensor(1);
	SetTXbin(dataEdit[1]);
	SelSensor(2);
	SetTXbin(dataEdit[1]);
	SelSensor(3);
	SetTXbin(dataEdit[1]);
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


void CTrimDlg::OnBnClickedBtnSendcmd()
{
	// TODO: Add your control notification handler code here

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


void CTrimDlg::OnBnClickedBtnItgtim()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	TrimFlag = sendtrimmsg;

	IntTimeDlg intDlg;

	intDlg.m_IntTime = int_time;

	if (intDlg.DoModal() == IDOK)
	{
	int_time = intDlg.m_IntTime;
	}

	//取Intergrate Time编辑框的值传给主对话框
	CString kstg;
//	float kfl;

	kstg.Format("%4.3f", int_time);
	SetDlgItemText(IDC_EDIT_IGTIME,kstg);	

//	GetDlgItemText(IDC_EDIT_IGTIME,kstg);		//从编辑框获取数值字符串

//	kfl = (float)atof(kstg);		//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & int_time;	//将浮点数据转化为十六进制数据


/*	CString kstg;
	kstg.Empty();
	float kfl = 0;

	GetDlgItemText(IDC_TRIM_EDIT_IntergrateTime,kstg);
	kfl = (float)atof(kstg);	//将字符串转成浮点型数据

	unsigned char * hData = (unsigned char *) & kfl;	//将浮点数据转化为十六进制数据
*/
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
	//0x01 means send vedio data
	//0x00 means stop vedio data
	TxData[8] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7];		//check sum
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[5]=TxData[5];
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

void CTrimDlg::SetV20(char v20)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte
	TxData[4] = v20;		//real data, date edit second byte
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

void CTrimDlg::SetV15(char v15)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x05;		//data type, date edit first byte
	TxData[4] = v15;	//real data, date edit second byte
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


void CTrimDlg::SetRampgen(char rampgen)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x01;		//data type, date edit first byte
	TxData[4] = rampgen;	//real data, date edit second byte
	//0x01 means send video data
	//0x00 means stop video data
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

void CTrimDlg::SetTXbin(char txbin)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x08;		//data type, date edit first byte
	TxData[4] = txbin;	//real data, date edit second byte
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

void CTrimDlg::ResetTxBin(void)
{
		m_ComboxTX.SetCurSel(8);
}


void CTrimDlg::OnBnClickedTrimBtnPcrtemp1()
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

extern CString RegRecStr;

void CTrimDlg::OnBnClickedBtnShowdata()
{
	// TODO: 在此添加控件通知处理程序代码

			SetDlgItemText(IDC_EDIT_REDATA,RegRecStr);

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

void CTrimDlg::SetV20(char v20, char i)
{
	SelSensor(i);
	SetV20(v20);
}


void CTrimDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	CKineticDlg kinDlg;

	kinDlg.m_Interval = videoElapseTime;
	kinDlg.m_Maxcnt = autoRepeatMaxCount;

	if (kinDlg.DoModal() == IDOK)
	{
		videoElapseTime = kinDlg.m_Interval;
		autoRepeatMaxCount = kinDlg.m_Maxcnt;
	}

}

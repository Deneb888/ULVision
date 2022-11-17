// GraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "GraDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <afx.h>

#include "ExcelFormat.h"

using namespace ExcelFormat;
//using namespace std;

//***************************************************************
//Global variable definition
//***************************************************************

int GraFlag = 0;				// graphic dialog flag
bool Gra_pageFlag = false;			// graphic dialog 画页循环标志
bool Gra_videoFlag = false;			// graphic dialog video循环标志
BYTE GraBuf[gradatanum];			// graphic dialog transmitted data buffer
int RecdataFlag = 0;				// receive data recode flag

byte PCRType = 0;				// PCR select type
int PCRCycCnt = 0;				// PCR 依次发送计数
int PCRNum = 0;					// 各PCR序号

BYTE PCRTypeFilterClass = 0;			// Graphic command返回type 类别区分，如画行、页、vedio（返回type byte的低4位）
BYTE PCRTypeFilterNum = 0;			// Graphic command返回type 各PCR区分（返回type byte的高4位）

int Page12_24Flag = 0;				// 12行/24行画页标志(0:12行，1:24行)
int Vedio12_24Flag = 0;				// 12行/24行Vedio标志(0:12行，1:24行)

int xOffset = 0;				// 各PCR画图偏移量
int yOffset = 0;				// 各PCR画图偏移量

CBitmap bBmp;					// 后台画图BMP
CBitmap *oldBMP;				// 原BMP指针
CDC bgDC;					// 后台DC
CDC * dispalyDC;				// 前台显示DC
CRect displayRect;				// 显示用rect

CBitmap bBmp_Row;				// 后台画行BMP
CBitmap *oldBMP_Row;				// 原BMP指针
CDC bgDC_Row;					// 后台DC
CDC *RowDC;					// 前台显示DC

int OnPaint_Flag = 0;				// OnPaint绘制标志
BOOL DrawPageStart = FALSE;			// 判断是不是第一次按绘图按钮(Capture)
BOOL DrawRowStart = FALSE;			// 判断是不是第一次按画行按钮

CString sPCRdata;
CString sADCRecdata;

//CFile m_recdataFile(_T("PRCdata.txt"),CFile::modeCreate|CFile::modeWrite);				// receive data file
//CFile m_adcdataFile(_T("KineticFrameData.txt"), CFile::modeCreate|CFile::modeWrite);			// ADC data file
//CFile m_recdataFile;				// receive data file

CFile m_adcdataFile;				// ADC data file
bool file_open = false;

CTime sysTime_Gra;				// system time

POINT pt;					// 编辑框显示位置指针
int typePixelSize = 0;				// 12行和24行区分标志

//*****************************************************************
//Own function
//*****************************************************************


//*****************************************************************
//External function
//*****************************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);		//ASIC convert to HEX
extern int ChangeNum (CString str, int length);								//字符串转10进制

extern float 	int_time; 		// zd add, in ms
extern int 	gain_mode;  		// zd add, 0 high gain, 1 low gain; this is the CURRENT gain mode that is commited to the HW
int 		adc_result[24];    	// zd add, buffer for one row of data
int 		rn;			// zd add, This is for HDR, may not be needed.

extern BOOL 	g_DeviceDetected;

//=====HDR Support==========

int pdata_0[24][24];	// first pass
int pdata_1[24][24];	// second pass

int hdr_phase = 0;
int contrast = 16;

int frame_average[5]; 	// index is '1' based.

//==========================

extern CTrimDlg *g_pTrimDlg;
extern CTrimReader *g_pTrimReader;

// Exel file export feature

CString sXlsDataFileName; 	// shared in DisplayAVG

BasicExcel 				xls;
BasicExcelWorksheet* 	xls_sheet;
int 					xls_row = 0;
int 					xls_col = 0;

unsigned int videoElapseTime = 1;
unsigned int autoRepeatCounter = 0;
unsigned int autoRepeatMaxCount = 5;

unsigned int pdata_avg1[24][24];	// average pic from auto repeat
unsigned int pdata_avg2[24][24];	// average pic from auto repeat
unsigned int pdata_avg3[24][24];	// average pic from auto repeat
unsigned int pdata_avg4[24][24];	// average pic from auto repeat

#define ADC_CORRECT
#define DARK_MANAGE

#define LAST_PCR 2			// Number of sensors installed

BOOL g_ChannelMask[] = {1, 1, 0, 0};

#define DARK_LEVEL 100

#define MAX_CHAN 4
// #define RUSU
#define Lucentix

struct FrameData {
	unsigned int fdata[24][24];
	unsigned int avgdata[24][24];
};

FrameData	frame[4];

BOOL PollingGraTimerFlag = FALSE;		// GraDlg polling timer（用来查询接收polling停止命令）是否开启flag
BOOL PollingGraMode = FALSE;			// 画图时判断是否为polling模式

//------------------

// CGraDlg dialog

IMPLEMENT_DYNAMIC(CGraDlg, CDialogEx)

CGraDlg::CGraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGraDlg::IDD, pParent)
	, m_GainMode(0)
	, m_FrameSize(0)
	, m_OrigOut(FALSE)
{

	m_EditReadRow = _T("");
	m_ShowAllDataInt = 0;
	m_ReadHexInt = 0;
	m_PixelData = _T("");
	m_ADCRecdata = _T("");

//	m_GainMode = 0;
//	m_FrameSize = 0;
	m_PixOut = true;
//	m_OrigOut = true;

	int i, j;

	for(i=0; i<24; i++) {
		adc_result[i] = 0;
		for(j=0; j<24; j++) {
			frame[0].fdata[i][j] = 0;
			frame[1].fdata[i][j] = 0;
			frame[2].fdata[i][j] = 0;
			frame[3].fdata[i][j] = 0;

			frame[0].avgdata[i][j] = 0;
			frame[1].avgdata[i][j] = 0;
			frame[2].avgdata[i][j] = 0;
			frame[3].avgdata[i][j] = 0;
		}
	}

	for(i=0; i<4; i++) {
		frame_average[i] = 0;
	}
}

CGraDlg::~CGraDlg()
{
}

void CGraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_READROW, m_EditReadRow);
	DDX_Control(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllData);
	DDX_Radio(pDX, IDC_RADIO_SHOWALLDATA, m_ShowAllDataInt);
	DDX_Control(pDX, IDC_RADIO_READHEX, m_ReadHex);
	DDX_Radio(pDX, IDC_RADIO_READHEX, m_ReadHexInt);
	DDX_Control(pDX, IDC_RADIO_READDEC, m_ReadDec);
	DDX_Control(pDX, IDC_RADIO_ADCDATA, m_ADCData);
	DDX_Control(pDX, IDC_RADIO_SHOWVALIDDATA, m_ShowValidData);
	DDX_Text(pDX, IDC_EDIT_RecData, m_PixelData);
	DDX_Text(pDX, IDC_EDIT_ADCDATA, m_ADCRecdata);
	DDX_Radio(pDX, IDC_RADIOLOWGAIN, m_GainMode);
	DDX_Radio(pDX, IDC_RADIO_12, m_FrameSize);
	DDX_Check(pDX, IDC_CHECK_PIXOUT, m_PixOut);
	DDX_Check(pDX, IDC_CHECK_ORIGOUT, m_OrigOut);
}


BEGIN_MESSAGE_MAP(CGraDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_READROW, &CGraDlg::OnClickedBtnReadrow)
	ON_MESSAGE(UM_GRAPROCESS,OnGraProcess)
	ON_BN_CLICKED(IDC_BTN_DPROW24, &CGraDlg::OnBnClickedBtnDprow24)
	ON_BN_CLICKED(IDC_BTN_DPPAGE12, &CGraDlg::OnBnClickedBtnDppage12)
	ON_BN_CLICKED(IDC_BTN_DPPAGE24, &CGraDlg::OnBnClickedBtnDppage24)
	ON_BN_CLICKED(IDC_BTN_DPVEDIO, &CGraDlg::OnBnClickedBtnDpvedio)
	ON_BN_CLICKED(IDC_BTN_STOPVIDEO, &CGraDlg::OnBnClickedBtnStopvideo)
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CGraDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_ADCCONVERT, &CGraDlg::OnBnClickedBtnAdcconvert)
	ON_BN_CLICKED(IDC_BTN_CLEARADC, &CGraDlg::OnBnClickedBtnClearadc)
	ON_BN_CLICKED(IDC_BUTTON1, &CGraDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CGraDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BTN_DPVIDEO24, &CGraDlg::OnBnClickedBtnDpvideo24)
	ON_BN_CLICKED(IDC_RADIO_ADCDATA, &CGraDlg::OnBnClickedRadioAdcdata)
//	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER1, &CGraDlg::OnThumbposchangingSlider1)
//	ON_WM_HSCROLL()
//	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CGraDlg::OnNMCustomdrawSlider1)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_RADIOLOWGAIN, &CGraDlg::OnClickedRadiolowgain)
//ON_UPDATE_COMMAND_UI(IDC_RADIOHDR, &CGraDlg::OnUpdateRadiohdr)
//ON_UPDATE_COMMAND_UI(IDC_RADIOHIGHGAIN, &CGraDlg::OnUpdateRadiohighgain)
ON_COMMAND(IDC_RADIOHIGHGAIN, &CGraDlg::OnRadiohighgain)
ON_COMMAND(IDC_RADIOHDR, &CGraDlg::OnRadiohdr)
ON_BN_CLICKED(IDC_RADIO_12, &CGraDlg::OnClickedRadio12)
ON_COMMAND(IDC_RADIO_24, &CGraDlg::OnRadio24)
ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CGraDlg::OnBnClickedButtonCapture)
ON_BN_CLICKED(IDC_CHECK_PIXOUT, &CGraDlg::OnClickedCheckPixout)
ON_BN_CLICKED(IDC_CHECK_ORIGOUT, &CGraDlg::OnClickedCheckOrigout)
ON_WM_PAINT()
ON_EN_CHANGE(IDC_EDIT_ADCDATA, &CGraDlg::OnEnChangeEditAdcdata)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BTN_COPY, &CGraDlg::OnBnClickedBtnCopy)
ON_BN_CLICKED(IDC_CHECK_IMGEN, &CGraDlg::OnBnClickedCheckImgen)
ON_BN_CLICKED(IDC_CHECK_CH1, &CGraDlg::OnBnClickedCheckCh1)
ON_BN_CLICKED(IDC_CHECK_CH2, &CGraDlg::OnBnClickedCheckCh2)
ON_BN_CLICKED(IDC_BUTTON_SAVEIMAGE, &CGraDlg::OnBnClickedButtonSaveimage)
ON_BN_CLICKED(IDC_LED_INDV_EN, &CGraDlg::OnBnClickedLedIndvEn)
ON_BN_CLICKED(IDC_LED1_EN, &CGraDlg::OnBnClickedLed1En)
ON_BN_CLICKED(IDC_LED_SWITCH, &CGraDlg::OnBnClickedLedSwitch)
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//调用主对话框对应消息处理函数
void CGraDlg::GraCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_GraDlg_event,a,b);
}


void CGraDlg::MakeGraPacket(byte pCmd, byte pType, byte pData)
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = pCmd;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = pType;		//data type, date edit first byte
	TxData[4] = pData;		//real data
	TxData[5] = 0x00;		//预留位
	TxData[6] = 0x00;
	TxData[7] = 0x00;
	TxData[8] = 0x00;
	TxData[9] = 0x00;
	TxData[10] = 0x00;
	TxData[11] = 0x00;
	TxData[12] = 0x00;
	TxData[13] = 0x00;
	TxData[14] = 0x00;
	TxData[15] = TxData[1]+TxData[2]+TxData[3]+TxData[4]+TxData[5]+TxData[6]+TxData[7]+TxData[8]+TxData[9]
	+TxData[10]+TxData[11]+TxData[12]+TxData[13]+TxData[14];		//check sum
	if (TxData[15]==0x17)
		TxData[15]=0x18;
	else
		TxData[15]=TxData[15];
	TxData[16] = 0x17;		//back code
	TxData[17] = 0x17;		//back code
}

	
int pixelNum=0;		// This should probably be a member variable of the class

LRESULT CGraDlg::OnGraProcess(WPARAM wParam, LPARAM lParam)
{
	//数据处理及显示

//	int pixelNum=0;

	// 根据返回的type区分是显示12列数据还是24列数据
	switch(PCRTypeFilterClass)
	{
	case dprow12:		// 12列画行
	case dppage12:		// 12列画页
	case dpvideo12:		// 12列画vedio
	case 0x0c:			// 积分时间超过10ms
		pixelNum = 12;
		break;

	case dprow24:		// 24列画行
	case dppage24:		// 24列画页
	case dpvideo24:		// 24列画vedio
		pixelNum = 24;
		break;

	default: break;
	}

	WORD * DecData = new WORD [pixelNum];
	int NumData = 0;
	int iDecVal = 0;

	CString TmHex;
	TmHex.Empty();

	CString sDecData;	// 最后显示的十进制字符串
	sDecData.Empty();

	char sDecVal[10];
	BYTE lByte = 0x00;

	CString sRownum;
	sRownum.Empty();
	sRownum.Format(" %d",RxData[5]);

	for (NumData=0; NumData<pixelNum; NumData++)			//将每两个byte整合成一个word
	{
		lByte=RxData[NumData*2+6];				//取出低4位byte
		lByte = lByte<<4;						//将低4位byte左移4位
		DecData[NumData] = RxData[NumData*2+7];		//将高8位byte赋值给word变量
		DecData[NumData] <<= 8;						//word buffer左移8位，将高8位byte数据放置到高8位
		DecData[NumData] |= lByte;				//将低4位byte放到word buffer低8位
		DecData[NumData] >>= 4;						//将word buffer整体右移4位，变成有效12位数据
	}

	for (NumData=0; NumData<pixelNum; NumData++)			//将一行中的每个Pixel十六进制有效数据转成十进制
	{
		TmHex.Format("%2X",DecData[NumData]);		//将每个word buffer转成十六进制字符串
		iDecVal = ChangeNum(TmHex, TmHex.GetLength());		//将每个十六进制字符串转成有效十进制数
		gcvt(iDecVal,4,sDecVal);		//将十进制浮点数转换成字符串
		//第二个参数代表十进制数有多少位
		//第三个参数，必须为char *, 如上面对sDecVal的定义

		sDecData += sDecVal;				//将char* 赋值给CString，用来显示
		sDecData += "  ";					//每个数据间加空格
	}

	sDecData += sRownum;
	delete[] DecData;

	// 当返回命令为0x14时，只显示下述字符串
	if (RxData[2] == 0x14)
	{
		sDecData = "";
		sDecData.Format("the cycle number is %d",RxData[3]);
	}

//	int ioffset = 0; // ilbc, iresult;
//	int lbc = 0;
	int result = 0;
//	int TemInt = 0;

	char fstrbuf[100];

//	CString TemHex;
//	TemHex.Empty();

	CString sADCData;	// 最后显示的ADC数据字符串
	sADCData.Empty();

	if(RxData[5] == 0) {
		sADCData += "\r\nChip #";
		itoa (PCRNum,fstrbuf,10);		//将结果转成字符串显示
		sADCData += fstrbuf;

		if(file_open) {
			sADCData += " Frame set #";
			itoa (autoRepeatCounter,fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;

			sprintf(fstrbuf, "Chip #%d, Frame set #%d", PCRNum, autoRepeatCounter);

			xls_col = 0;
			xls_sheet->Cell(xls_row, 0)->Set(fstrbuf);   //("Frame set ...");
			xls_row++;;
		}

		sADCData += "\r\n";
	}

	for (NumData=0; NumData<pixelNum; NumData++)
	{

		result = ADCCorrection(NumData, RxData[NumData*2+7], RxData[NumData*2+6]);

		adc_result[NumData] = result; // zd add

		itoa (result,fstrbuf,10);		//将结果转成字符串显示
		sADCData += fstrbuf;
		sADCData += " ";

		if(file_open) xls_sheet->Cell(xls_row, /*xls_col*/ NumData)->SetDouble(adc_result[NumData]);

		frame_average[PCRNum-1] += result; 

	//=========Zhimin added, save adc result data to create an average frame====================

		unsigned int rn = RxData[5];
		unsigned int cn = NumData;

		if(/*file_open*/ autoRepeatCounter) {
			switch(PCRNum) {
				case(1): 
					pdata_avg1[rn][cn] += result;
					break;
				case(2):
					pdata_avg2[rn][cn] += result;
					break;
				case(3):
					pdata_avg3[rn][cn] += result;
					break;
				case(4):
					pdata_avg4[rn][cn] += result;
					break;
				default:
					break;
			}
		frame[PCRNum-1].avgdata[rn][cn] += result;
		}

		frame[PCRNum-1].fdata[rn][cn] = result;

	//------------------------------------------------------------------------------------------
	}

	// 编辑框数据清零
/*	if (pixelNum == 12)
	{
		if (RecdataFlag == 1)
		{
			m_PixelData.Empty();
			m_ADCRecdata.Empty();
			RecdataFlag = 0;
		}
		else
			RecdataFlag++;
	}
	else
	{
		if (RecdataFlag == 2)
		{
			m_PixelData.Empty();
			m_ADCRecdata.Empty();
			RecdataFlag = 0;
		}
		else
			RecdataFlag++;
	}
*/

	sADCData += sRownum;

	if(RxData[5] == 11 && pixelNum == 12) {
		itoa (frame_average[PCRNum-1] / 144, fstrbuf, 10);
		sADCData += " Frame average";
		sADCData += fstrbuf;

		if(file_open) {
			xls_sheet->Cell(xls_row, 12)->Set("Average");			
			xls_sheet->Cell(xls_row, 13)->SetDouble(frame_average[PCRNum-1] / 144); 
		}

		frame_average[PCRNum-1] = 0;
	}
	else if(RxData[5] == 23 && pixelNum == 24) {
		itoa (frame_average[PCRNum-1] / 576, fstrbuf, 10);
		sADCData += " Frame average";
		sADCData += fstrbuf;

		if(file_open) {
			xls_sheet->Cell(xls_row, 24)->Set("Average");			
			xls_sheet->Cell(xls_row, 25)->SetDouble(frame_average[PCRNum-1] / 576); 
		}

		frame_average[PCRNum-1] = 0;
	}

	// 当返回命令为0x14时，只显示下述字符串
	if (RxData[2] == 0x14)
	{
		sADCData = "";
		sADCData.Format("the cycle number is %d",RxData[3]);
	}

	if (m_OrigOut && m_ReadHex.GetCheck())			//以十六进制
	{
		if (m_ShowAllData.GetCheck())	//显示所有数据
		{
			//			m_PixelData = RegRecStr + m_PixelData;		//最新数据在编辑框第一行显示
			m_PixelData += (RegRecStr+"\r\n");					//最新数据在编辑框最后一行显示
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			sPCRdata = (RegRecStr+"\r\n");

			//编辑框垂直滚动到底端
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			RegRecStr = "";
		}

		if (m_ShowValidData.GetCheck())	//显示有效数据
		{
			//			m_PixelData = Valid_RegRecStr + m_PixelData;	//最新数据在编辑框第一行显示
			m_PixelData += (Valid_RegRecStr+"\r\n");					//最新数据在编辑框最后一行显示
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			sPCRdata = (Valid_RegRecStr+"\r\n");

			//编辑框垂直滚动到底端
//			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			Valid_RegRecStr = "";
		}
	}

	if (m_OrigOut && m_ReadDec.GetCheck())			//以十进制显示
	{
		//		m_PixelData = sTest + m_PixelData +"\r\n";			//每行数据将加回车
		//最新数据在编辑框第一行显示
		m_PixelData += (sDecData+"\r\n");
		SetDlgItemText(IDC_EDIT_RecData,m_PixelData);		//每行数据将加回车
		//最新数据在编辑框最后一行显示

		sPCRdata = (sDecData+"\r\n");

		//编辑框垂直滚动到底端
//		POINT pt;
		GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}

	if (m_PixOut && m_GainMode <= 1 && !file_open)		//ADC数据显示
	{
		m_ADCRecdata += (sADCData+"\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//每行数据将加回车
		//最新数据在编辑框最后一行显示

		sADCRecdata = (sADCData+"\r\n");

		//编辑框垂直滚动到底端
//		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}


	//-----------------------------------------------
	//编辑框文件保存
	// receive data save file
//	m_recdataFile.Write(sPCRdata,sPCRdata.GetLength());

	// ADC data save file

	if(file_open) {
		m_ADCRecdata += (".");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//每行数据将加回车

		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);

		sADCData += "\r\n";
		m_adcdataFile.Write(sADCData,sADCData.GetLength());

		xls_row++;
	}

	// 保存上一次按键的文件
//	if ((PCRNum == 4) & (Gra_pageFlag == false))
//	{
//		m_recdataFile.Close();
//		m_adcdataFile.Close();
//	}

	// Drawing
//	GraDlgDrawPattern();
/*
	int end_cycle = 0;

	Gra_videoFlag = true;			// video模式时，开始发送下一次

	if (Gra_pageFlag == false)
	{
		PCRCycCnt ++;			// PCR序号加1
		switch(PCRTypeFilterClass)
		{
		case dppage12:
			end_cycle = cyclePage12();		// 发送下一个PCR画页指令 read the next page
			break;
		case dppage24:
			end_cycle = cyclePage24();
			break;
		default:
			break;
		}
	}
	else {
		GraFlag = readgramsg;
		GraCalMainMsg();	// read the next row
	}

	if(end_cycle) RenderPattern();
*/
/*	switch(PCRTypeFilterClass)
	{
	case dppage12:
		{
			if(RxData[5] == 11 && PCRNum == LAST_PCR) {
				RenderPattern();
			}
		}
		break;
	case dppage24:
		{
			if(RxData[5] == 23 && PCRNum == LAST_PCR) {
				RenderPattern();
			}
		}
		break;
	default:
		break;
	}
*/

	//数据传输buffer清零
	memset(RxData,0,sizeof(RxData));

	return 0;
}


int CGraDlg::ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte)
{
	int hb,lb, lbc;
	int hbln,lbp,hbhn;
	bool oflow = false, uflow = false;

	int TemInt;
	char fstrbuf[9];
	CString strbuf;
	CString TemHex;

	int ioffset = 0; 
	int result;


		hb = (int)HighByte;

		int nd = 0;
	
		if(pixelNum == 12) nd = NumData;
		else nd = NumData >> 1;

#ifdef ADC_CORRECT

//		if(PCRNum == 1) ioffset = (int)(kb1[2*nd]*hb + kb1[2*nd+1]);
//		if(PCRNum == 2) ioffset = (int)(kb2[2*nd]*hb + kb2[2*nd+1]);
//		if(PCRNum == 3) ioffset = (int)(kb3[2*nd]*hb + kb3[2*nd+1]);
//		if(PCRNum == 4) ioffset = (int)(kb4[2*nd]*hb + kb4[2*nd+1]);

		if(g_pTrimReader) ioffset = (int)(g_pTrimReader->Node[PCRNum-1].kb[nd][0]*hb + g_pTrimReader->Node[PCRNum-1].kb[nd][1]);
		else ioffset = 0;
#endif

		hbln = hb % 16;		//得到用于溢出判断的hbln  step 2

		hbhn = hb / 16;		//得到用于求result的hbhn  stpe3

		lb = (int)LowByte;

		lbc = lb + ioffset;

		lbp = hbln * 16 + 7;

		if ( (lbp-ioffset) > 255 + 32)
			oflow = true;
		else if( (lbp-ioffset) > 255 && (lbp - lbc) > 48)
			oflow = true;
		else if( (lbp-ioffset) > 191 && (lbp - lbc) > 127)
			oflow = true;
		else
		{
			if ( (lbp-ioffset) < 0 - 32)
				uflow = true;
			else if((lbp-ioffset) < 0 && (lbp - lbc) < -48)
				uflow = true;
			else if((lbp-ioffset) < 64 && (lbp - lbc) < -127)
				uflow = true;
			else
			{
				oflow = false;
				uflow = false;
			}
		}

		if (oflow)
			result = hb * 16 + 7;
		else
		{
			if (uflow)
				result = hb * 16 + 7;
			else
				result = hbhn * 256 + lbc;
		}

#ifdef DARK_MANAGE

/*		if(!gain_mode) {
			if(PCRNum == 1)		 result += -(int) (FPN_hg1[nd]) + 100;
			else if(PCRNum == 2) result += -(int) (FPN_hg2[nd]) + 100;
			else if(PCRNum == 3) result += -(int) (FPN_hg3[nd]) + 100;
			else if(PCRNum == 4) result += -(int) (FPN_hg4[nd]) + 100;
			else result = 100;
		}
		else {
			if(PCRNum == 1)			result += -(int) (FPN1[nd]) + 100;
			else if(PCRNum == 2)	result += -(int) (FPN2[nd]) + 100;
			else if(PCRNum == 3)	result += -(int) (FPN3[nd]) + 100;
			else if(PCRNum == 4)	result += -(int) (FPN4[nd]) + 100;
			else result = 100;
		}
*/
		if(g_pTrimReader) {
			if(!gain_mode)
				result += -(int) (g_pTrimReader->Node[PCRNum-1].fpn[1][nd]) + DARK_LEVEL;		// high gain
			else 
				result += -(int) (g_pTrimReader->Node[PCRNum-1].fpn[0][nd]) + DARK_LEVEL;					// low gain
		}

#endif

	if(result < 0) result = 0;

	return result;
}

void CGraDlg::GraDlgDrawPattern()
{
	CBrush brush[RowNum24][ColNum24];
	int i=0;
	int l=0;
	int gray_level=0;

	// zd comment RxData[5] is the row number.

    	rn = RxData[5];

	for (i=0; i<ColNum24; i++)
	{
		gray_level = adc_result[i]/16;
		if(gray_level > 255) gray_level = 255;
		else if(gray_level < 0) gray_level = 0;

		brush[RxData[5]][i].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));				 // zd mod use corrected adc data with green tint
//		brush[RxData[5]][i].CreateSolidBrush(RGB(255,0,0));		// 取低byte为有效位
//		brush[RxData[5]][i].CreateSolidBrush(RGB(RxData[i*2+6],RxData[i*2+6],RxData[i*2+6]));		// 取高byte为有效位
	}

	switch (PCRNum)
	{
	case 1:
		if (typePixelSize == 12)		// 12行情况
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24行情况
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (typePixelSize == 12)
		{
			xOffset = pixelsize12*12;		// offset = pixelsize * rowNum
			yOffset = 0;
		}
		else
		{
			xOffset = pixelsize24*24;
			yOffset = 0;
		}
		break;
	case 3:
		if (typePixelSize == 12)
		{
			xOffset = 0;
			yOffset = pixelsize12*12;
		}
		else
		{
			xOffset = 0;
			yOffset = pixelsize24*24;
		}
		break;
	case 4:
		if (typePixelSize == 12)
		{
			xOffset = pixelsize12*12;
			yOffset = pixelsize12*12;
		}
		else
		{
			xOffset = pixelsize24*24;
			yOffset = pixelsize24*24;
		}
		break;
	default:
		break;
	}

	switch(PCRTypeFilterClass)
	{
	case dprow12:
		{
			for(int l=0; l<ColNum12; l++)		// l代表列号；rxdata[5]中数据是行号
			{
				displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
				bgDC_Row.Rectangle(displayRect);
				bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
			}
			if(PCRNum == LAST_PCR) {
				PCRNum = 0;
				OnPaint_Flag = DRAWROW;
				Invalidate();
			}
			else {
				PCRCycCnt ++;		// PCR序号加1
				cycleRow12();		// 发送下一个PCR画页指令
				//数据传输buffer清零
				memset(RxData,0,sizeof(RxData));
			}
		break;
		}

	case dppage12:
		{
			for(int l=0; l<ColNum12; l++)		// l代表列号
			{
				if(m_GainMode <= 1) 
				{
#ifdef RUSU
					int row_num;

					if(PCRNum == 1)	row_num= 11-RxData[5];
					else row_num = RxData[5];

					int col_num = 11-l;

					displayRect.SetRect((pixelsize12*row_num + xOffset),(pixelsize12*col_num + yOffset),(pixelsize12*(row_num+1) + xOffset),(pixelsize12*(col_num+1) + yOffset));

#else
					displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
#endif
					bgDC.Rectangle(displayRect);
					bgDC.FillRect(&displayRect,&brush[RxData[5]][l]);
				}
				else
				{
					//zd add
					if(!hdr_phase) {
						pdata_0[rn][l] = adc_result[l];
					}
					else {
						pdata_1[rn][l] = adc_result[l];
					}
				}
			}

				Gra_videoFlag = true;		// video模式时，开始发送下一次

				if (Gra_pageFlag == false)
				{
					PCRCycCnt ++;		// PCR序号加1
					cyclePage12();		// 发送下一个PCR画页指令 read the next page
				}
				else {
					GraFlag = readgramsg;
					GraCalMainMsg();	// read the next row
				}
//			}
			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x0c:		// 积分时间超过10ms
		{
			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dprow24:
		{
			for(int l=0; l<ColNum24; l++)		// l代表列号；rxdata[5]中数据是行号
			{
				displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
				bgDC_Row.Rectangle(displayRect);
				bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
			}

			if(PCRNum == LAST_PCR) {
				PCRNum = 0;
				OnPaint_Flag = DRAWROW;
				Invalidate();
			}
			else {
				PCRCycCnt ++;		// PCR序号加1
				cycleRow24();		// 发送下一个PCR画页指令
				//数据传输buffer清零
				memset(RxData,0,sizeof(RxData));
			}

			break;
		}
	case dppage24:
		{
			for(int l=0; l<ColNum24; l++)		// l代表列号
			{
				if(m_GainMode <= 1) 
				{
#ifdef RUSU
					int row_num;

					if(PCRNum == 1)	row_num= 23-RxData[5];
					else row_num = RxData[5];

					int col_num = 23-l;

					displayRect.SetRect((pixelsize24 * row_num + xOffset),(pixelsize24 * col_num + yOffset),(pixelsize24*(row_num+1) + xOffset),(pixelsize24*(col_num + 1) + yOffset));
#else
					displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
#endif
					bgDC.Rectangle(displayRect);
					bgDC.FillRect(&displayRect,&brush[RxData[5]][l]);
				}
				else
				{
					//zd add
					if(!hdr_phase) {
						pdata_0[rn][l] = adc_result[l];
					}
					else {
						pdata_1[rn][l] = adc_result[l];
					}
				}
			}

				Gra_videoFlag = true;		// video模式时，开始发送下一次

				if (Gra_pageFlag == false)
				{
					PCRCycCnt ++;		// PCR序号加1
					cyclePage24();		// 发送下一个PCR画页指令	read next page
				}
				else {
					GraFlag = readgramsg;
					GraCalMainMsg();	// read the next row
				}
//			}
			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));
			break;
		}
	case dpvideo12:	//video12
		{
			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo24:	//video24
		{
			//数据传输buffer清零
			memset(RxData,0,sizeof(RxData));

			break;
		}

	default:
		break;
	}
}


// CGraDlg message handlers


void CGraDlg::OnClickedBtnReadrow()
{
	// TODO: Add your control notification handler code here

	// 判断是不是第一次按画行按钮
	if (DrawRowStart)
	{
		bBmp_Row.DeleteObject();
		bgDC_Row.SelectObject(oldBMP_Row);
		bgDC_Row.DeleteDC();
	}
	DrawRowStart = TRUE;

	// initial cycle number
	PCRCycCnt =0;

	CString sRownum;
	GetDlgItemText(IDC_EDIT_READROW,sRownum);
	GraBuf[0] = atoi(sRownum);

	// initial background DC
	RowDC = GetDlgItem(IDC_PCR2BMP) -> GetDC();
	bgDC_Row.CreateCompatibleDC(RowDC);

	bBmp_Row.CreateCompatibleBitmap(RowDC,240,240);
	oldBMP_Row = bgDC_Row.SelectObject(&bBmp_Row);
	bgDC_Row.SetBkColor(RGB(0,0,0));

	// start draw row
	PCRType = (PCRCycCnt<<4)|(0x01);	// 分别设置4个PCR的type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}

void CGraDlg::cycleRow12(void)
{
	PCRType = (PCRCycCnt<<4)|(0x01);	// 分别设置4个PCR的type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::OnBnClickedBtnDprow24()
{
	// TODO: Add your control notification handler code here

	// 判断是不是第一次按画行按钮
	if (DrawRowStart)
	{
		bBmp_Row.DeleteObject();
		bgDC_Row.SelectObject(oldBMP_Row);
		bgDC_Row.DeleteDC();
	}
	DrawRowStart = TRUE;

	// initial cycle number
	PCRCycCnt =0;

	CString sRownum;
	GetDlgItemText(IDC_EDIT_READROW,sRownum);
	GraBuf[0] = atoi(sRownum);

	// initial background DC
	RowDC = GetDlgItem(IDC_PCR2BMP) -> GetDC();
	bgDC_Row.CreateCompatibleDC(RowDC);

	bBmp_Row.CreateCompatibleBitmap(RowDC,240,240);
	oldBMP_Row = bgDC_Row.SelectObject(&bBmp_Row);
	bgDC_Row.SetBkColor(RGB(0,0,0));

	// start draw row
	PCRType = (PCRCycCnt<<4)|(0x07);	// 分别设置4个PCR的type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::cycleRow24(void)
{
	PCRType = (PCRCycCnt<<4)|(0x07);	// 分别设置4个PCR的type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}

//Start Video: Repurposed to Auto repeat capture

void CGraDlg::OnBnClickedBtnDpvedio()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	CString sADCDataFileName;
//	sRecDataFileName.Empty();
	sADCDataFileName.Empty();

	sXlsDataFileName.Empty();

	// 按当时时间创建文件名
	CString sTime_Gra;					// system string format
	sTime_Gra.Empty();
	sysTime_Gra = CTime::GetCurrentTime();
	sTime_Gra.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
	sysTime_Gra.GetYear(),sysTime_Gra.GetMonth(),sysTime_Gra.GetDay(),
	sysTime_Gra.GetHour(),sysTime_Gra.GetMinute(),sysTime_Gra.GetSecond());
//	sRecDataFileName = "DisplayData_" + sTime_Gra + ".txt";
	sADCDataFileName = "KineticFrameData_" + sTime_Gra + ".txt";
	sXlsDataFileName = "KineticFrameData_" + sTime_Gra + ".xls";

	m_adcdataFile.Open(sADCDataFileName,CFile::modeCreate|CFile::modeWrite);
	file_open = true;
	
	autoRepeatCounter++;  // Count starts at 1

	xls.New(1);
	xls_sheet = xls.GetWorksheet(0);
	xls_row = 0;	
	
	SetTimer(1, videoElapseTime * 1000, NULL);		// 12行video
	OnBnClickedButtonCapture();
}

// repurposed for preread/pipelined


void CGraDlg::OnBnClickedBtnDpvideo24()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

//	#ifdef RUSU
//		if(g_pTrimDlg) g_pTrimDlg->SetIntTime(1, 1);	
//	#endif

	if(!m_FrameSize) {
		PreRead12();
	}
	else {
		PreRead24();
	}

	OnBnClickedButtonCapture();
}

void CGraDlg::PreRead12()
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0xf2;		//data type, date edit first byte - PRE-READ
	TxData[4] = 0x3;	    	//real data, date edit second byte - The first and second sensor will perform integration
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	GraFlag = sendgramsg;

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


//Stop Video
void CGraDlg::OnBnClickedBtnStopvideo()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	KillTimer(1);		// stop video
//	DisplayAVG(autoRepeatCounter);

	CalculateAVG(0, autoRepeatCounter);
	CalculateAVG(1, autoRepeatCounter);
	RenderPattern();

	autoRepeatCounter = 0;
}


void CGraDlg::OnBnClickedBtnClear()
{
	// TODO: Add your control notification handler code here

	m_PixelData.Empty();
	SetDlgItemText(IDC_EDIT_RecData,m_PixelData);
}


void CGraDlg::OnBnClickedBtnAdcconvert()
{
	// TODO: Add your control notification handler code here
}


void CGraDlg::OnBnClickedBtnClearadc()
{
	// TODO: Add your control notification handler code here
	if(MessageBox("Clear data buffer?", "ULVision", MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL) 
		return;
			
	m_ADCRecdata.Empty();
	SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);
}


void CGraDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CString str;
	// 创建另存对话框
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"文本文件(*.txt)|*.txt|""文档文件(*.doc)|*.doc|""表格文件(*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// 另存对话框默认路径为c盘

	// 进行保存动作
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_RecData);	// 获取要保存编辑框控件内的数据
		// IDC_EDIT_FILE是编辑框控件句柄
		pst->GetWindowTextA(str);
		ofs << str;
	}
}

// Save file button

void CGraDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	CString str;
	// 创建另存对话框
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// 另存对话框默认路径为c盘

	// 进行保存动作
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_ADCDATA);	// 获取要保存编辑框控件内的数据
		// IDC_EDIT_FILE是编辑框控件句柄
		pst->GetWindowTextA(str);
		ofs << str;
	}
}


void CGraDlg::OnBnClickedRadioAdcdata()
{
	// TODO: Add your control notification handler code here
}


void CGraDlg::SetGainMode(int gain)		// 1: low gain; 0: high gain
{
	if(gain == gain_mode) 
		return;

	if(g_pTrimDlg) {
		g_pTrimDlg->SetGainMode(gain, 1);
		g_pTrimDlg->SetGainMode(gain, 2);
	}

#ifdef DARK_MANAGE

	if(g_pTrimDlg && g_pTrimReader) {
		if(!gain) 
			g_pTrimDlg->SetV20(g_pTrimReader->Node[0].auto_v20[1], 1); 
		else 
			g_pTrimDlg->SetV20(g_pTrimReader->Node[0].auto_v20[0], 1); 

		if(!gain) 
			g_pTrimDlg->SetV20(g_pTrimReader->Node[1].auto_v20[1], 2); 
		else 
			g_pTrimDlg->SetV20(g_pTrimReader->Node[1].auto_v20[0], 2); 

/*		if(!gain) g_pTrimDlg->SetV20(g_pTrimReader->Node[2].auto_v20[1], 3); 		// To do: use LAST_PCR to manage
		else g_pTrimDlg->SetV20(g_pTrimReader->Node[2].auto_v20[0], 3); 

		if(!gain) g_pTrimDlg->SetV20(g_pTrimReader->Node[3].auto_v20[1], 4); 
		else g_pTrimDlg->SetV20(g_pTrimReader->Node[3].auto_v20[0], 4); 
*/	}

#endif
}


void CGraDlg::DisplayAVG(unsigned int cnt)
{
	CBrush brush1[24][24];
	CBrush brush2[24][24];
	CBrush brush3[24][24];
	CBrush brush4[24][24];

	int i, j;
	int gray_level = 0;
	int size, pixelsize;

	if(!m_FrameSize) {
		size = 12;
		pixelsize = pixelsize12;
	}
	else {
		size = 24;
		pixelsize = pixelsize24;
	}

	xOffset = 0;
	yOffset = 0;

	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			gray_level = pdata_avg1[i][j]/(16*cnt);
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush1[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));
			frame_average[1] += pdata_avg1[i][j]/cnt;
		}
	}

	for(i=0; i<size; i++) {
			for(j=0; j<size; j++)		// l代表列号
			{
				displayRect.SetRect(pixelsize*j + xOffset, pixelsize*i + yOffset, pixelsize*(j+1) + xOffset, pixelsize*(i+1) + yOffset);
				bgDC.Rectangle(displayRect);
				bgDC.FillRect(&displayRect,&brush1[i][j]);
			}
	}

	xOffset = pixelsize * size;
	yOffset = 0;

	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			gray_level = pdata_avg2[i][j]/(16*cnt);
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush2[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));
			frame_average[2] += pdata_avg2[i][j]/cnt;
			}
	}

	for(i=0; i<size; i++) {
			for(j=0; j<size; j++)		// l代表列号
			{
				displayRect.SetRect(pixelsize*j + xOffset, pixelsize*i + yOffset, pixelsize*(j+1) + xOffset, pixelsize*(i+1) + yOffset);
				bgDC.Rectangle(displayRect);
				bgDC.FillRect(&displayRect,&brush2[i][j]);
			}
	}

	xOffset = 0;
	yOffset = pixelsize * size;

	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			gray_level = pdata_avg3[i][j]/(16*cnt);
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush3[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));
			frame_average[3] += pdata_avg3[i][j]/cnt;
			}
	}

	for(i=0; i<size; i++) {
			for(j=0; j<size; j++)		// l代表列号
			{
				displayRect.SetRect(pixelsize*j + xOffset, pixelsize*i + yOffset, pixelsize*(j+1) + xOffset, pixelsize*(i+1) + yOffset);
				bgDC.Rectangle(displayRect);
				bgDC.FillRect(&displayRect,&brush3[i][j]);
			}
	}

	xOffset = pixelsize * size;
	yOffset = pixelsize * size;

	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			gray_level = pdata_avg4[i][j]/(16*cnt);
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush4[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));
			frame_average[4] += pdata_avg4[i][j]/cnt;
			}
	}

	for(i=0; i<size; i++) {
			for(j=0; j<size; j++)		// l代表列号
			{
				displayRect.SetRect(pixelsize*j + xOffset, pixelsize*i + yOffset, pixelsize*(j+1) + xOffset, pixelsize*(i+1) + yOffset);
				bgDC.Rectangle(displayRect);
				bgDC.FillRect(&displayRect,&brush4[i][j]);
			}
	}

//==========================================================

	CString sRownum;
	char fstrbuf[100];
	CString sADCData;	// 最后显示的ADC数据字符?

	sADCData = "\r\nChip #1 Kinetic Average:\r\n";
	m_ADCRecdata.Empty();

	xls_sheet->Cell(xls_row, 0)->Set("Chip #1 Kinetic Average:");
	xls_row++;

	for (i=0; i<size; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<size; j++)
		{
			itoa (pdata_avg1[i][j]/cnt,fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;
			sADCData += " ";

			xls_sheet->Cell(xls_row + i, j)->SetDouble(pdata_avg1[i][j]/cnt);
		}
		sADCData += sRownum;
		if(i == size - 1) {
			itoa (frame_average[1] / (size*size), fstrbuf, 10);
			sADCData += " Frame average";
			sADCData += fstrbuf;
			sADCData += "\r\n";

			xls_sheet->Cell(xls_row + i, size)->Set("Average:");
			xls_sheet->Cell(xls_row + i, size + 1)->SetDouble(frame_average[1] / (size*size));

			frame_average[1] = 0;
		}
		sADCData += "\r\n";
	}
	xls_row += size;

	sADCData += "Chip #2 Kinetic Average:\r\n";
	xls_sheet->Cell(xls_row, 0)->Set("Chip #2 Kinetic Average:");
	xls_row++;
	for (i=0; i<size; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<size; j++)
		{
			itoa (pdata_avg2[i][j]/cnt,fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;
			sADCData += " ";
			xls_sheet->Cell(xls_row + i, j)->SetDouble(pdata_avg2[i][j]/cnt);
		}
		sADCData += sRownum;
		if(i == size - 1) {
			itoa (frame_average[2] / (size*size), fstrbuf, 10);
			sADCData += " Frame average";
			sADCData += fstrbuf;
			sADCData += "\r\n";
			xls_sheet->Cell(xls_row + i, size)->Set("Average:");
			xls_sheet->Cell(xls_row + i, size + 1)->SetDouble(frame_average[2] / (size*size));
			frame_average[2] = 0;
		}
		sADCData += "\r\n";
	}
	xls_row += size;

	sADCData += "Chip #3 Kinetic Average:\r\n";
	xls_sheet->Cell(xls_row, 0)->Set("Chip #3 Kinetic Average:");
	xls_row++;
	for (i=0; i<size; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<size; j++)
		{
			itoa (pdata_avg3[i][j]/cnt,fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;
			sADCData += " ";
			xls_sheet->Cell(xls_row + i, j)->SetDouble(pdata_avg3[i][j]/cnt);
		}
		sADCData += sRownum;
		if(i == size - 1) {
			itoa (frame_average[3] / (size*size), fstrbuf, 10);
			sADCData += " Frame average";
			sADCData += fstrbuf;
			sADCData += "\r\n";
			xls_sheet->Cell(xls_row + i, size)->Set("Average:");
			xls_sheet->Cell(xls_row + i, size + 1)->SetDouble(frame_average[3] / (size*size));
			frame_average[3] = 0;
		}
		sADCData += "\r\n";
	}
	xls_row += size;

	sADCData += "Chip #4 Kinetic Average:\r\n";
	xls_sheet->Cell(xls_row, 0)->Set("Chip #4 Kinetic Average:");
	xls_row++;
	for (i=0; i<size; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<size; j++)
		{
			itoa (pdata_avg4[i][j]/cnt,fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;
			sADCData += " ";
			xls_sheet->Cell(xls_row + i, j)->SetDouble(pdata_avg4[i][j]/cnt);
		}
		sADCData += sRownum;
		if(i == size - 1) {
			itoa (frame_average[4] / (size*size), fstrbuf, 10);
			sADCData += " Frame average";
			sADCData += fstrbuf;
			sADCData += "\r\n";
			xls_sheet->Cell(xls_row + i, size)->Set("Average:");
			xls_sheet->Cell(xls_row + i, size + 1)->SetDouble(frame_average[4] / (size*size));
			frame_average[4] = 0;
		}
		sADCData += "\r\n";
	}
	xls_row += size;

	if(m_PixOut) {
		m_ADCRecdata += (sADCData+"\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//每行数据将加回车
		//最新数据在编辑框最后一行显示
		//编辑框垂直滚动到底端
		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}
//		sADCData = "";
//	}

	m_adcdataFile.Write(sADCData,sADCData.GetLength());
//	m_adcdataFile.Write(sADCRecdata,sADCRecdata.GetLength());

	m_adcdataFile.Close();
	file_open = false;

	xls.SaveAs(sXlsDataFileName);
	xls.Close();

	for(i=0; i<24; i++) {
		for(j=0; j<24; j++) {
			pdata_avg1[i][j]=0;
			pdata_avg2[i][j]=0;
			pdata_avg3[i][j]=0;
			pdata_avg4[i][j]=0;
		}
	}
}

void CGraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

//	if(m_GainMode <= 1) return; // only works for HDR mode.

	int pos;

/*	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);  

    // Check which slider sent the notification  
    if (pSlider == &c_Slider1)  
    {  
    }
    else if (pSlider == &c_Slider2)  
    {  
    }  
*/
    // Check what happened  

    switch(nSBCode)
    {
    case TB_LINEUP:  
    case TB_LINEDOWN:  
    case TB_PAGEUP:  
    case TB_PAGEDOWN:  
    case TB_THUMBPOSITION: 
		pos = nPos;
		contrast = 16 - pos / 7;
//		if(!m_FrameSize)
//			DisplayHDR();
//		else
//			DisplayHDR24();
//		break;

//		DisplayAVG2(0, 5);
//		DisplayAVG2(1, 5);
		RenderPattern();
//		Invalidate();
		break;
    case TB_TOP:  
    case TB_BOTTOM:  
    case TB_THUMBTRACK:  
    case TB_ENDTRACK:  
    default:  
        break;  
    }

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CGraDlg::DisplayHDR24(void)
{
	CDC *pDC;		//创建目标DC指针
	pDC=GetDlgItem(IDC_Bmp)->GetDC();

	CRect   rect;
	CBrush brush[24][24];
	int i, j;
	int gray_level;
	int pdata[24][24];

	for (i=0; i<24; i++)
	{
		for(j=0; j<24; j++)
		{
			if(pdata_1[i][j] < 400) pdata[i][j] = pdata_0[i][j];
			else pdata[i][j] = (pdata_1[i][j]-100)*8 + 100;
		}
	}

	for (i=0; i<24; i++)
	{
		for(j=0; j<24; j++)
		{
			gray_level = pdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));
		}
	}

	for(i=0; i<24; i++) {
			for(j=0; j<24; j++)		// l代表列号
			{
				rect.SetRect(pixelsize24*j,pixelsize24*i,pixelsize24*(j+1),pixelsize24*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// 最后显示的ADC数据字符?

	for (i=0; i<24; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<24; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//每行数据将加回车
			//最新数据在编辑框最后一行显示
			//编辑框垂直滚动到底端
			POINT pt;
			GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
		}
		sADCData = "";
	}
}

void CGraDlg::OnClickedRadiolowgain()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnRadiohighgain()
{
	// TODO: Add your command handler code here
		UpdateData(true);
}


void CGraDlg::OnRadiohdr()
{
	// TODO: Add your command handler code here
		UpdateData(true);
}


void CGraDlg::OnClickedRadio12()
{
	// TODO: Add your control notification handler code here
			UpdateData(true);
}


void CGraDlg::OnRadio24()
{
	// TODO: Add your command handler code here
			UpdateData(true);
}


void CGraDlg::OnBnClickedButtonCapture()
{
	// TODO: Add your control notification handler code here

		// 发送显示指令
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!g_ChannelMask[0] && !g_ChannelMask[1] ) {
		MessageBox("All channels not selected");
		return;
	}

//	OnBnClickedBtnClearadc(); // Zhimin added. Clear ADC window when capture button is clicked.

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_KEEPHISTORY);

	if(!pCheckbox->GetCheck()) {
	m_ADCRecdata.Empty();
	SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);
	}

	// initial cycle flag
	PCRCycCnt = 0;

	// 判断是不是按第一次绘图按钮（capture）
	if (DrawPageStart)
	{
		bBmp.DeleteObject();
		bgDC.SelectObject(oldBMP);
		bgDC.DeleteDC();
	}
	DrawPageStart = TRUE;

	// initial background DC 
	dispalyDC = GetDlgItem(IDC_Bmp) -> GetDC();
	bgDC.CreateCompatibleDC(dispalyDC);

	if(LAST_PCR == 2) bBmp.CreateCompatibleBitmap(dispalyDC,480,240);
	else bBmp.CreateCompatibleBitmap(dispalyDC,480,240);
	oldBMP = bgDC.SelectObject(&bBmp);
	bgDC.SetBkColor(RGB(0,0,0));

	//-----------------------------------------------------
	// 建立编辑框保存的文件
	//-----------------------------------------------------
//	CString sRecDataFileName,sADCDataFileName;
//	sRecDataFileName.Empty();
//	sADCDataFileName.Empty();

	// 按当时时间创建文件名
//	CString sTime_Gra;					// system string format
//	sTime_Gra.Empty();
//	sysTime_Gra = CTime::GetCurrentTime();
//	sTime_Gra.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
//	sysTime_Gra.GetYear(),sysTime_Gra.GetMonth(),sysTime_Gra.GetDay(),
//	sysTime_Gra.GetHour(),sysTime_Gra.GetMinute(),sysTime_Gra.GetSecond());
//	sRecDataFileName = "DisplayData_" + sTime_Gra + ".txt";
//	sADCDataFileName = "ADCData_" + sTime_Gra + ".txt";

//	m_recdataFile.Open(sRecDataFileName,CFile::modeCreate|CFile::modeWrite);
//	m_adcdataFile.Open(sADCDataFileName,CFile::modeCreate|CFile::modeWrite);

	if(!m_FrameSize) {
		typePixelSize = 12;
		if(m_GainMode <= 1) {
			CaptureFrame12();
		}
		else {
			OnBnClickedBtnDppage12();
		}
	}
	else {
		typePixelSize = 24;
		if(m_GainMode <= 1) {
			CaptureFrame24();
		}
		else {
			OnBnClickedBtnDppage24();
		}
		if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();
	}
}


void CGraDlg::CaptureFrame12(void)
{
	if(!m_GainMode) {
		SetGainMode(1);
	}
	else {
		SetGainMode(0);
	}

//	PCRType = (PCRCycCnt<<4)|(0x02);	// 分别设置4个PCR的type

//	GraFlag = sendpagemsg;
//	Gra_pageFlag = TRUE;	// 开始循环

//	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
//	GraCalMainMsg();		//调用主对话框串口发送消息程序

	cyclePage12();

	int end_cycle = 0;

	Gra_videoFlag = true;			// video模式时，开始发送下一次

	while (!end_cycle && Gra_pageFlag == false)
	{
		PCRCycCnt ++;			// PCR序号加1
		end_cycle = cyclePage12();		// 发送下一个PCR画页指令 read the next page
	}
//	else {
//		GraFlag = readgramsg;
//		GraCalMainMsg();	// read the next row
//	}

	if(end_cycle) 
		RenderPattern();

}

int CGraDlg::cyclePage12(void)
{
	while(!g_ChannelMask[PCRCycCnt]) {
		PCRCycCnt++;

		if(PCRCycCnt == MAX_CHAN) {
			OnPaint_Flag = DRAWPAGE;
			Invalidate();
			Gra_videoFlag = true;		// video模式时，开始发送下一次
			return 1;
		}
	}

// #ifdef RUSU
// 	if(g_pTrimDlg) g_pTrimDlg->SetIntTime(PCRCycCnt+1, 1);	
// #endif
	
	PCRType = (PCRCycCnt<<4) | (0x02);	// 分别设置4个PCR的type

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// 开始循环

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序

	return 0;
}


void CGraDlg::CaptureFrame24(void)
{
	if(!m_GainMode) {
		SetGainMode(1);
	}
	else {
		SetGainMode(0);
	}

	if(g_pTrimDlg) g_pTrimDlg->ResetTxBin();

//	PCRType = (PCRCycCnt<<4)|(0x08);	// 分别设置4个PCR的type

//	GraFlag = sendpagemsg;
//	Gra_pageFlag = TRUE;	// 开始循环

//	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
//	GraCalMainMsg();		//调用主对话框串口发送消息程序

	cyclePage24();

	int end_cycle = 0;

	Gra_videoFlag = true;			// video模式时，开始发送下一次

	while (!end_cycle && Gra_pageFlag == false)
	{
		PCRCycCnt ++;			// PCR序号加1
		end_cycle = cyclePage24();
	}
//	else {
//		GraFlag = readgramsg;
//		GraCalMainMsg();	// read the next row
//	}

	if(end_cycle) 
		RenderPattern();
}


int CGraDlg::cyclePage24(void)
{
	while(!g_ChannelMask[PCRCycCnt]) {
		PCRCycCnt++;

		if(PCRCycCnt == MAX_CHAN) {
			OnPaint_Flag = DRAWPAGE;
			Invalidate();
			Gra_videoFlag = true;		// video模式时，开始发送下一次
			return 1;
		}
	}

// #ifdef RUSU
//	if(g_pTrimDlg) g_pTrimDlg->SetIntTime(PCRCycCnt+1, 1);	
// #endif

	PCRType = (PCRCycCnt<<4)|(0x08);	// 分别设置4个PCR的type

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// 开始循环

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序

	return 0;
}

// Repurposed for HDR read

void CGraDlg::OnBnClickedBtnDppage12()
{
	// TODO: Add your control notification handler code here

	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4个PCR依次发送
	{
		PCRType = (PCRCycCnt<<4)|(0x02);	// 分别设置4个PCR的type

		SetGainMode(0);
		hdr_phase = 0;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// 开始循环

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//调用主对话框串口发送消息程序

		//===========
		SetGainMode(1);
		hdr_phase = 1;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// 开始循环

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//调用主对话框串口发送消息程序

//		DisplayHDR();
	}
}

// Repurposed for HDR read

void CGraDlg::OnBnClickedBtnDppage24()
{
	// TODO: Add your control notification handler code here

	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4个PCR依次发送
	{
		PCRType = (PCRCycCnt<<4)|(0x08);	// 分别设置4个PCR的type

		SetGainMode(0);
		hdr_phase = 0;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// 开始循环

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//调用主对话框串口发送消息程序

		//===========

		SetGainMode(1);
		hdr_phase = 1;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// 开始循环

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//调用主对话框串口发送消息程序

		//		DisplayHDR24();
	}
}


void CGraDlg::OnClickedCheckPixout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


void CGraDlg::OnClickedCheckOrigout()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
}


BOOL CGraDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	OnPaint_Flag = 0;

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CH1);

	pCheckbox->SetCheck(1);

	CButton *pCheckbox2 = (CButton*)GetDlgItem(IDC_CHECK_CH2);

	pCheckbox2->SetCheck(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CGraDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here

	switch (OnPaint_Flag)
	{
	case DRAWPAGE:
		{
			dispalyDC -> BitBlt(0,0,480,240,&bgDC,0,0,SRCCOPY);
			break;
		}
	case DRAWROW:
		{
			RowDC -> BitBlt(0,0,240,240,&bgDC_Row,0,0,SRCCOPY);
			break;
		}
	default:
		break;
	}

	// Do not call CDialogEx::OnPaint() for painting messages
}


void CGraDlg::OnEnChangeEditAdcdata()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void CGraDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch(nIDEvent)
	{
	case 1:
		{
			if (Gra_videoFlag) {
				autoRepeatCounter++;
				OnBnClickedButtonCapture();
				if(autoRepeatCounter >= autoRepeatMaxCount) {
					KillTimer(1);
//					DisplayAVG(autoRepeatCounter);
				CalculateAVG(0, autoRepeatCounter);
				CalculateAVG(1, autoRepeatCounter);
				RenderPattern();
				autoRepeatCounter = 0;
				}
			}
			break;
		}
	case 2:			// graphic HID polling
		{
			if (PollingGraTimerFlag)
			{
				GraFlag = sendgramsg;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x15;		//command  TXC
				TxData[2] = 0x05;		//data length
				TxData[3] = 0x01;		//data type
				TxData[4] = 0x00;		
				TxData[5] = 0x00;	
				TxData[6] = 0x00;       
				TxData[7] = 0x00;	
				for (int i=1; i<8; i++)
					TxData[8] += TxData[i];
				if (TxData[8]==0x17)
					TxData[8]=0x18;
				else
					TxData[8]=TxData[8];
				TxData[9]=0x17;
				TxData[10]=0x17;

				//Send message to main dialog
				GraCalMainMsg();	//调用主对话框处理消息程序
			}
			break;
		}
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CGraDlg::OnBnClickedBtnCopy()
{
	// TODO: Add your control notification handler code here

	CEdit *myEdit;

	myEdit = (CEdit*)GetDlgItem(IDC_EDIT_ADCDATA);

	myEdit->SetSel(0, -1, FALSE);
	myEdit->Copy();
}

void CGraDlg::PreRead24()
{
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0xf2;		//data type, date edit first byte - PRE-READ
	TxData[4] = 0x13;	    //real data, date edit second byte - The first and second sensor will perform integration
	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	GraFlag = sendgramsg;

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::OnBnClickedCheckCh1()
{
	// TODO: Add your control notification handler code here

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CH1);
	CButton *pCheckbox_2 = (CButton*)GetDlgItem(IDC_CHECK_CH2);

	g_ChannelMask[0] = (pCheckbox->GetCheck())? 1:0;	


	// TODO: Add your control notification handler code here


	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x24;		//data type, channel mask
	TxData[4]=0;
	TxData[4] = (g_ChannelMask[0])? 1:0;
	if((pCheckbox_2->GetCheck())) TxData[4] |= 0x2;


	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	GraFlag = sendgramsg;

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序



}


void CGraDlg::OnBnClickedCheckCh2()
{
	// TODO: Add your control notification handler code here

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_CH2);
	CButton *pCheckbox_1 = (CButton*)GetDlgItem(IDC_CHECK_CH1);

	g_ChannelMask[1] = (pCheckbox->GetCheck())? 1:0;	



	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x24;		//data type, channel mask
	TxData[4]=0;
	TxData[4] = (g_ChannelMask[1])? 2:0;
	if((pCheckbox_1->GetCheck())) TxData[4] |= 0x1;


	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	GraFlag = sendgramsg;

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}


void CGraDlg::OnBnClickedButtonSaveimage()
{
	// TODO: Add your control notification handler code here
	CFileDialog FileDialog(FALSE,".bmp",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Bmp (*.bmp)|*.bmp|""Jpeg (*.jpeg)|*.jpeg|""Gif (*.gif)|*.gif|""Png (*.png)|*.png|",
		NULL,0,TRUE);

	FileDialog.DoModal();

	if(FileDialog.GetPathName().GetLength() <= 0)
	{
		return;
	}
	else
	{
		CDC MemDC;
//		CRect ClientRect;
		MemDC.CreateCompatibleDC(&bgDC);
		CBitmap Bmp;
		Bmp.CreateCompatibleBitmap(&bgDC,480, 240); // ClientRect.Width(),ClientRect.Height());
		MemDC.SelectObject(&Bmp);
		MemDC.BitBlt(0,0,/*ClientRect.Width(),ClientRect.Height()*/ 480, 240,&bgDC,0,0,SRCCOPY);
		CImage TempImageObj;
		TempImageObj.Attach((HBITMAP)Bmp.Detach());
		TempImageObj.Save(FileDialog.GetPathName());
	}
}

void CGraDlg::CalculateAVG(unsigned int chip, unsigned int cnt)
{
	int i, j;
	int size;

	if(!m_FrameSize) {
		size = 12;
//		pixelsize = pixelsize12;
	}
	else {
		size = 24;
//		pixelsize = pixelsize24;
	}

	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			frame[chip].fdata[i][j] = frame[chip].avgdata[i][j]/cnt;
			frame_average[chip] += frame[chip].avgdata[i][j]/cnt;
		}
	}

/*	for(i=0; i<size; i++) {
		for(j=0; j<size; j++)		// l代表列号
		{
			displayRect.SetRect(pixelsize*j + xOffset, pixelsize*i + yOffset, pixelsize*(j+1) + xOffset, pixelsize*(i+1) + yOffset);
			bgDC.Rectangle(displayRect);
			bgDC.FillRect(&displayRect,&brush[i][j]);
		}
	}
*/
//	return;
//==========================================================

	CString sRownum;
	char fstrbuf[100];
	CString sADCData;	// 最后显示的ADC数据字符?

	sRownum.Empty();
	sRownum.Format(" %d",chip+1);


	sADCData = "\r\nChip" + sRownum + " Kinetic Average:\r\n";
	if(chip == 0) m_ADCRecdata.Empty();

	xls_sheet->Cell(xls_row, 0)->Set("Chip"); xls_sheet->Cell(xls_row, 1)->Set((int)(chip + 1)); xls_sheet->Cell(xls_row, 2)->Set("Kinetic Average:");
	xls_row++;

	for (i=0; i<size; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<size; j++)
		{
			itoa (frame[chip].avgdata[i][j]/cnt,fstrbuf,10);		//将结果转成字符串显示
			sADCData += fstrbuf;
			sADCData += " ";

			xls_sheet->Cell(xls_row + i, j)->SetDouble(frame[chip].avgdata[i][j]/cnt);
		}
		sADCData += sRownum;
		if(i == size - 1) {
			itoa (frame_average[chip] / (size*size), fstrbuf, 10);
			sADCData += " Frame average";
			sADCData += fstrbuf;
			sADCData += "\r\n";

			xls_sheet->Cell(xls_row + i, size)->Set("Average:");
			xls_sheet->Cell(xls_row + i, size + 1)->SetDouble(frame_average[chip] / (size*size));

			frame_average[chip] = 0;
		}
		sADCData += "\r\n";
	}
	xls_row += size;

	if(m_PixOut) {
		m_ADCRecdata += (sADCData+"\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//每行数据将加回车
		//最新数据在编辑框最后一行显示
		//编辑框垂直滚动到底端
		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}
//		sADCData = "";
//	}

	m_adcdataFile.Write(sADCData,sADCData.GetLength());
//	m_adcdataFile.Write(sADCRecdata,sADCRecdata.GetLength());

	if(chip == LAST_PCR - 1) {
	m_adcdataFile.Close();
	file_open = false;

	xls.SaveAs(sXlsDataFileName);
	xls.Close();
	}

	for(i=0; i<24; i++) {
		for(j=0; j<24; j++) {
			frame[chip].avgdata[i][j]=0;
		}
	}
}


void CGraDlg::DisplayPattern(int chip)
{
	CBrush brush[24][24];

	int i, j;
	int gray_level = 0;
	int size, pixelsize;

	if(!m_FrameSize) {
		size = 12;
		pixelsize = pixelsize12;
	}
	else {
		size = 24;
		pixelsize = pixelsize24;
	}

	switch(chip) {
	case 0:
		xOffset = 0;
		yOffset = 0;	
		break;
	case 1:
		xOffset = pixelsize * size;
		yOffset = 0;
		break;
	case 2:
		xOffset = 0;
		yOffset = pixelsize * size;
		break;
	case 3:
		xOffset = pixelsize * size;
		yOffset = pixelsize * size;
		break;
	default:
		break;
	}

	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			gray_level = frame[chip].fdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));
			frame_average[chip] += frame[chip].fdata[i][j];
		}
	}

	for(i=0; i<size; i++) {
		for(j=0; j<size; j++)	
		{
/*#ifdef RUSU
					int row_num;

					if(chip == 0)	{
						if(size == 12) row_num= 11-i;
						else row_num = 23-i;
					}
					else {
						row_num = i;
					}

					int col_num;
					if(size == 12) col_num = 11-j;
					else col_num = 23-j;
*/

#ifdef Lucentix
					int row_num = j;

					if(chip == 1)	{
						if(size == 12) row_num= 11-j;
						else row_num = 23-j;
					}
					else {
						row_num = j;
					}

					int col_num;
					if(chip == 11)	{
						if(size == 12) col_num = 11-i;
						else col_num = 23-i;
					}
					else {
						col_num = i;
					}

					displayRect.SetRect((pixelsize*row_num + xOffset),(pixelsize*col_num + yOffset),(pixelsize*(row_num+1) + xOffset),(pixelsize*(col_num+1) + yOffset));
#else
//					displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));

			displayRect.SetRect(pixelsize*j + xOffset, pixelsize*i + yOffset, pixelsize*(j+1) + xOffset, pixelsize*(i+1) + yOffset);
#endif			bgDC.Rectangle(displayRect);
			bgDC.FillRect(&displayRect,&brush[i][j]);
		}
	}

	//release brush here
	for (i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			brush[i][j].DeleteObject();
		}
	}
}

void CGraDlg::RenderPattern() 
{
	int i;

	for(i=0; i<LAST_PCR; i++) {
		if(g_ChannelMask[i]) {
			DisplayPattern(i);
		}
	}

	Invalidate();
}

//////////////////////////////////////////////////////////
// Polling feature related
//////////////////////////////////////////////////////////


//BOOL	PollingGraMode = FALSE;
//BOOL	PollingGraTimerFlag = FALSE;

void CGraDlg::OnBnClickedCheckImgen()
{
	// TODO: Add your control notification handler code here

	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_IMGEN);

	if(pCheckbox->GetCheck()) {
		PollingGraMode = TRUE;
		PollingGraTimerFlag = TRUE;
		SetTimer(2,300,NULL);		// start polling
	}
	else {
		PollingGraMode = FALSE;
		PollingGraTimerFlag = FALSE;
		KillTimer(2);		// stop polling
	}


	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x28;		//data type, date edit first byte
	TxData[4] = (pCheckbox->GetCheck())? 1:0;		//real data, date edit second byte

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	GraFlag = sendgramsg;

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}

/*

void CGraDlg::OnBnClickedCheckImgen()
{
	// TODO: Add your control notification handler code here

	// TODO: 在此添加控件通知处理程序代码
	CButton *pCheckbox = (CButton*)GetDlgItem(IDC_CHECK_IMGEN);

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command
	TxData[2] = 0x02;		//data length
	TxData[3] = 0x28;		//data type, date edit first byte
	TxData[4] = (pCheckbox->GetCheck())? 1:0;		//real data, date edit second byte

	TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
	if (TxData[5]==0x17)
		TxData[5]=0x18;
	else
		TxData[5]=TxData[5];
	TxData[6] = 0x17;		//back code
	TxData[7] = 0x17;		//back code

	GraFlag = sendgramsg;

	//Send message to main dialog
	GraCalMainMsg();		//调用主对话框串口发送消息程序
}

*/

	void CGraDlg::OnBnClickedLedIndvEn()
{
	// TODO: 在此添加控件通知处理程序代码
	if(((CButton*)GetDlgItem(IDC_LED_INDV_EN))->GetCheck()==BST_CHECKED)
	{
	   ((CButton*)GetDlgItem(IDC_LED2_EN))->SetCheck(false);
	   ((CButton*)GetDlgItem(IDC_LED3_EN))->SetCheck(false);
	   ((CButton*)GetDlgItem(IDC_LED4_EN))->SetCheck(false);

	   ((CButton*)GetDlgItem(IDC_LED2_EN))->EnableWindow(true);
	   ((CButton*)GetDlgItem(IDC_LED3_EN))->EnableWindow(true);
	   ((CButton*)GetDlgItem(IDC_LED4_EN))->EnableWindow(true);
	}
	else
	{
	   
	   ((CButton*)GetDlgItem(IDC_LED2_EN))->EnableWindow(false);
	   ((CButton*)GetDlgItem(IDC_LED3_EN))->EnableWindow(false);
	   ((CButton*)GetDlgItem(IDC_LED4_EN))->EnableWindow(false);
	}
}


	void CGraDlg::OnBnClickedLed1En()
	{
		// TODO: 在此添加控件通知处理程序代码
	}


	void CGraDlg::OnBnClickedLedSwitch()
	{
	// TODO: 在此添加控件通知处理程序代码
		// TODO: 在此添加控件通知处理程序代码
			TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x01;		//command
		TxData[2] = 0x02;		//data length
		TxData[3] = 0x23;		//data type, date edit first byte
		if(((CButton*)GetDlgItem(IDC_LED_INDV_EN))->GetCheck()==BST_UNCHECKED)
		{
			
			TxData[4] = (((CButton*)GetDlgItem(IDC_LED1_EN))->GetCheck()==BST_CHECKED)? 1:0;		//real data, date edit second byte
		}
		else
		{
			TxData[4] = 0x80;
			if((((CButton*)GetDlgItem(IDC_LED1_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 1;
			if((((CButton*)GetDlgItem(IDC_LED2_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 2;
			if((((CButton*)GetDlgItem(IDC_LED3_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 4;
			if((((CButton*)GetDlgItem(IDC_LED4_EN))->GetCheck()==BST_CHECKED))
				TxData[4] |= 8;

		}
		TxData[5] = TxData[1]+TxData[2]+TxData[3]+TxData[4];		//check sum
		if (TxData[5]==0x17)
			TxData[5]=0x18;
		else
			TxData[5]=TxData[5];
		TxData[6] = 0x17;		//back code
		TxData[7] = 0x17;		//back code

		GraFlag = sendgramsg;    
		//Send message to main dialog
		GraCalMainMsg();		//调用主对话框串口发送消息程序
	}

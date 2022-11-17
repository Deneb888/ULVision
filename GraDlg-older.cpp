// GraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "GraDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <afx.h>
using namespace std;

//***************************************************************
//Global variable definition
//***************************************************************

int GraFlag = 0;				// graphic dialog flag
bool Gra_pageFlag = false;		// graphic dialog ��ҳѭ����־
bool Gra_videoFlag = false;		// graphic dialog videoѭ����־
BYTE GraBuf[gradatanum];		// graphic dialog transmitted data buffer
int RecdataFlag = 0;			// receive data recode flag

byte PCRType = 0;				// PCR select type
int PCRCycCnt = 0;				// PCR ���η��ͼ���
int PCRNum = 0;					// ��PCR���

BYTE PCRTypeFilterClass = 0;	// Graphic command����type ������֣��续�С�ҳ��vedio������type byte�ĵ�4λ��
BYTE PCRTypeFilterNum = 0;		// Graphic command����type ��PCR���֣�����type byte�ĸ�4λ��

int Page12_24Flag = 0;			// 12��/24�л�ҳ��־(0:12�У�1:24��)
int Vedio12_24Flag = 0;			// 12��/24��Vedio��־(0:12�У�1:24��)

int xOffset = 0;				// ��PCR��ͼƫ����
int yOffset = 0;				// ��PCR��ͼƫ����

CBitmap bBmp;						// ��̨��ͼBMP
CBitmap *oldBMP;					// ԭBMPָ��
CDC bgDC;							// ��̨DC
CDC * dispalyDC;					// ǰ̨��ʾDC
CRect displayRect;					// ��ʾ��rect

CBitmap bBmp_Row;					// ��̨����BMP
CBitmap *oldBMP_Row;				// ԭBMPָ��
CDC bgDC_Row;						// ��̨DC
CDC *RowDC;							// ǰ̨��ʾDC

int OnPaint_Flag = 0;				// OnPaint���Ʊ�־
BOOL DrawPageStart = FALSE;			// �ж��ǲ��ǵ�һ�ΰ���ͼ��ť(Capture)
BOOL DrawRowStart = FALSE;			// �ж��ǲ��ǵ�һ�ΰ����а�ť

CString sPCRdata;
CString sADCRecdata;
//CFile m_recdataFile(_T("PRCdata.txt"),CFile::modeCreate|CFile::modeWrite);			// receive data file
//CFile m_adcdataFile(_T("ADCdata.txt"),CFile::modeCreate|CFile::modeWrite);			// ADC data file
//CFile m_recdataFile;			// receive data file
//CFile m_adcdataFile;			// ADC data file

CTime sysTime_Gra;					// system time

POINT pt;			// �༭����ʾλ��ָ��
int typePixelSize = 0;		// 12�к�24�����ֱ�־

//*****************************************************************
//Own function
//*****************************************************************


//*****************************************************************
//External function
//*****************************************************************
extern unsigned char AsicConvert (unsigned char i, unsigned char j);		//ASIC convert to HEX
extern int ChangeNum (CString str, int length);								//�ַ���ת10����

extern float int_time; // zd add, in ms
extern int gain_mode;  // zd add, 0 high gain, 1 low gain
int adc_result[24];    // zd add
int rn;					// zd add

extern BOOL g_DeviceDetected;

//=====HDR Support==========

int pdata_0[24][24];	// first pass
int pdata_1[24][24];	// second pass
int hdr_phase = 0;

int contrast = 100;

int frame_average[4];

//==========================

extern CTrimDlg *g_pTrimDlg;

unsigned int videoElapseTime = 5000;

unsigned int autoRepeatCounter = 0;

unsigned int autoRepeatMaxCount = 5;

//===========

const double kb1[] = {
-0.487179487,	107.4358974,
-0.195918367,	3.979591837,
-0.194871795,	0.974358974,
-0.246153846,	23.23076923,
-0.21025641,	-7.948717949,
-0.138461538,	12.69230769,
-0.215384615,	19.07692308,
-0.128205128,	25.64102564,
 0.051282051,	44.74358974,
-0.179487179,	105.8974359,
-0.461538462,	2.307692308,
 0.01025641,	17.94871795
};

const double kb2[] = {
-0.066666667,	38.33333333,
0.102040816,	-20.51020408,
0.169230769,	-38.84615385,
-0.061538462,	42.30769231,
0.112820513,	9.435897436,
0.046153846,	7.769230769,
0.241025641,	11.79487179,
0.271794872,	-14.35897436,
0.076923077,	-40.38461538,
-0.020512821,	-17.8974359,
0.487179487,	-64.43589744,
0.292307692,	-46.46153846
};

const double kb3[] = {
0.18974359,	-5.948717949,
0.020408163,	-5.102040816,
0,				20,
0.143589744,	-28.71794872,
0.138461538,	22.30769231,
-0.215384615,	76.07692308,
0.071794872,	-10.35897436,
-0.066666667,	-11.66666667,
0.2,			5,
0.102564103,	31.48717949,
0.015384615,	-8.076923077,
0.282051282,	13.58974359
};

const double kb4[] = {
-0.435897436,	47.17948718,
-0.06122449,	55.30612245,
0.056410256,	-9.282051282,
-0.148717949,	57.74358974,
0.256410256,	18.71794872,
0.251282051,	13.74358974,
0.051282051,	-50.25641026,
-0.066666667,	-4.666666667,
0.261538462,	-28.30769231,
0.082051282,	59.58974359,
-0.107692308,	28.53846154,
-0.138461538,	19.69230769
};

	const double FPN1[] = {272,	107.2777778,	153.9305556,	94.44444444,	74.51388889,	111.7083333,	161.4722222,	101.7222222,	203.5833333,	171.0138889,	126.0833333,	170.7777778
};

	const double FPN_hg1[] = {310.7777778,	138.1388889,	183.6388889,	126.9583333,	104.3055556,	144.9444444,	192.25,	132.4444444,	273.2222222,	203.4722222,	155.3611111,	201.4861111
};

	const char auto_v20_lg1 = 0x09;
	const char auto_v20_hg1 = 0x0b;

	const double FPN2[] = {151.8888889,	75.19444444,	123.9444444,	286.5555556,	140.7916667,	97.27777778,	72.48611111,	96.66666667,	48.56944444,	80.04166667,	88.47222222,	103.0277778
};

	const double FPN_hg2[] = {176.9166667,	98.20833333,	149.9166667,	315.7916667,	166.2916667,	122.1111111,	98.48611111,	121.8194444,	74.5,	103.5138889,	112.8888889,	127.2361111
};

	const char auto_v20_lg2 = 0x09;
	const char auto_v20_hg2 = 0x0b;

	const double FPN3[] = {164.8888889,	127.1527778,	328.4861111,	131.4583333,	236.125,	143.7361111,	200.3194444,	166.7638889,	210.5,	203.5555556,	102.75,	148.3888889
};

	const double FPN_hg3[] = {200.9305556,	161.3055556,	364.4444444,	167.2777778,	297.1527778,	179.25,	257.4444444,	204.1527778,	277.3194444,	262.1944444,	138.7916667,	184.6527778
};

	const char auto_v20_lg3 = 0x09;
	const char auto_v20_hg3 = 0x0b;


	const double FPN4[] = {105.125,	130.8333333,	190.8472222,	130.8055556,	160.6666667,	123.375,	64.5,	88.56944444,	135.5416667,	154.4027778,	144.4861111,	148.4722222
};

	const double FPN_hg4[] = {129.3611111,	154.5833333,	263.6805556,	155.4305556,	187.1805556,	147.3472222,	88.80555556,	112.9583333,	158.7222222,	176.7777778,	168.6527778,	171.8333333
};

	const char auto_v20_lg4 = 0x09;
	const char auto_v20_hg4 = 0x0b;


#define ADC_CORRECT
#define DARK_MANAGE

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

	int i;

	for(i=0; i<24; i++) {
		adc_result[i] = 0;
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
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//�������Ի����Ӧ��Ϣ������
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
	TxData[5] = 0x00;		//Ԥ��λ
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

void CGraDlg::DrawRaw(CDC *pBufDC, CRect pRect,CBrush pBrush[RowNum24][ColNum24], int pRowNum, int pColNum, int pPixelSize, int pPCRNum)
{
	int xOffset = 0;
	int yOffset = 0;
	switch (pPCRNum)
	{
	case 1:
		if (pPixelSize == pixelsize12)		// 12�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = 72;
			yOffset = 0;
		}
		break;
	case 3:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 0;
			yOffset = 60;
		}
		else
		{
			xOffset = 0;
			yOffset = 72;
		}
		break;
	case 4:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;
			yOffset = 60;
		}
		else
		{
			xOffset = 72;
			yOffset = 72;
		}
		break;
	default:
		break;
	}

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pBufDC,pRowNum*pPixelSize,pColNum*pPixelSize);		// ���õ�Ļ��С

	CDC bDC; 
	bDC.CreateCompatibleDC(pBufDC);	

	bDC.SelectObject(&bmp);		

	for(int l=0; l<pColNum; l++)		// l�����кţ�rxdata[5]���������к�
	{
		pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));
		bDC.Rectangle(pRect);
		bDC.FillRect(&pRect,&pBrush[RxData[5]][l]);
	}
	pBufDC->BitBlt(0,0,900, 380, &bDC, 0, 0, SRCCOPY);
}


void CGraDlg::DrawPage(CDC *pBufDC, CRect pRect, CBrush pBrush[RowNum24][ColNum24], int pPixelSize, int pPCRNum)
{
	switch (pPCRNum)
	{
	case 1:
		if (pPixelSize == pixelsize12)		// 12�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (pPixelSize == pixelsize12)
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
		if (pPixelSize == pixelsize12)
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
		if (pPixelSize == pixelsize12)
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

	if (pPixelSize == pixelsize12)
	{
		for(int l=0; l<ColNum12; l++)		// l�����к�
		{
			if(m_GainMode <= 1) {
				pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));

				pBufDC->Rectangle(pRect);
				pBufDC->FillRect(&pRect,&pBrush[RxData[5]][l]);
			}
			else {

				//zd add

				if(!hdr_phase) {
					pdata_0[rn][l] = adc_result[l];
				}
				else {
					pdata_1[rn][l] = adc_result[l];
				}
			}
		}
	}
	else
	{
		for(int l=0; l<ColNum24; l++)		// l�����к�
		{
			if(m_GainMode <= 1) {
				pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));
				pBufDC->Rectangle(pRect);
				pBufDC->FillRect(&pRect,&pBrush[RxData[5]][l]);
			}
			else {

				//zd add

				if(!hdr_phase) {
					pdata_0[rn][l] = adc_result[l];
				}
				else {
					pdata_1[rn][l] = adc_result[l];
				}
			}
		}
	}
}


void DrawOverTime(CDC *pBufDC, CRect pRect, CBrush pBrush[RowNum24][ColNum24], int pPixelSize, int pPCRNum)
{
	int xOffset = 0;
	int yOffset = 0;
	switch (pPCRNum)
	{
	case 1:
		if (pPixelSize == pixelsize12)		// 12�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		break;
	case 2:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;		// offset = pixelsize * rowNum		
			yOffset = 0;
		}
		else
		{
			xOffset = 72;
			yOffset = 0;
		}
		break;
	case 3:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 0;
			yOffset = 60;
		}
		else
		{
			xOffset = 0;
			yOffset = 72;
		}
		break;
	case 4:
		if (pPixelSize == pixelsize12)
		{
			xOffset = 60;
			yOffset = 60;
		}
		else
		{
			xOffset = 72;
			yOffset = 72;
		}
		break;
	default:
		break;
	}

	for(int l=0; l<ColNum12; l++)		// l�����к�
	{
		pRect.SetRect((pPixelSize*l + xOffset),(pPixelSize*RxData[5] + yOffset),(pPixelSize*(l+1) + xOffset),(pPixelSize*(RxData[5]+1) + yOffset));
		pBufDC->Rectangle(pRect);
		pBufDC->FillRect(&pRect,&pBrush[RxData[5]][l]);
	}
}


LRESULT CGraDlg::OnGraProcess(WPARAM wParam, LPARAM lParam)
{
	//���ݴ�����ʾ

	int pixelNum=0;

	// ���ݷ��ص�type��������ʾ12�����ݻ���24������
	switch(PCRTypeFilterClass)
	{
	case dprow12:		// 12�л���
	case dppage12:		// 12�л�ҳ
	case dpvideo12:		// 12�л�vedio
	case 0x0c:			// ����ʱ�䳬��10ms
		pixelNum = 12;
		break;

	case dprow24:		// 24�л���
	case dppage24:		// 24�л�ҳ
	case dpvideo24:		// 24�л�vedio
		pixelNum = 24;
		break;

	default: break;
	}

	WORD * DecData = new WORD [pixelNum];
	int NumData = 0;
	int iDecVal = 0;
	
	CString TmHex;
	TmHex.Empty();
	
	CString sDecData;	// �����ʾ��ʮ�����ַ���
	sDecData.Empty();
	
	char sDecVal[10];
	BYTE lByte = 0x00;
	
	CString sRownum;
	sRownum.Empty();
	sRownum.Format(" %d",RxData[5]);

	for (NumData=0; NumData<pixelNum; NumData++)			//��ÿ����byte���ϳ�һ��word
	{
		lByte=RxData[NumData*2+6];				//ȡ����4λbyte
		lByte = lByte<<4;						//����4λbyte����4λ
		DecData[NumData] = RxData[NumData*2+7];		//����8λbyte��ֵ��word����
		DecData[NumData] <<= 8;						//word buffer����8λ������8λbyte���ݷ��õ���8λ
		DecData[NumData] |= lByte;				//����4λbyte�ŵ�word buffer��8λ
		DecData[NumData] >>= 4;						//��word buffer��������4λ�������Ч12λ����
	}

	for (NumData=0; NumData<pixelNum; NumData++)			//��һ���е�ÿ��Pixelʮ��������Ч����ת��ʮ����
	{
		TmHex.Format("%2X",DecData[NumData]);		//��ÿ��word bufferת��ʮ�������ַ���
		iDecVal = ChangeNum(TmHex, TmHex.GetLength());		//��ÿ��ʮ�������ַ���ת����Чʮ������
		gcvt(iDecVal,4,sDecVal);		//��ʮ���Ƹ�����ת�����ַ���
		//�ڶ�����������ʮ�������ж���λ
		//����������������Ϊchar *, �������sDecVal�Ķ���

		sDecData += sDecVal;				//��char* ��ֵ��CString��������ʾ
		sDecData += "  ";					//ÿ�����ݼ�ӿո�
	}

	sDecData += sRownum;
	delete[] DecData;

	// ����������Ϊ0x14ʱ��ֻ��ʾ�����ַ���
	if (RxData[2] == 0x14)
	{
		sDecData = "";
		sDecData.Format("the cycle number is %d",RxData[3]);
	}

	int ioffset = 0; // ilbc, iresult;
	int lbc = 0;
	int result = 0;
	int TemInt = 0;
	
	char fstrbuf[9];

	CString TemHex;
	TemHex.Empty();

	CString sADCData;	// �����ʾ��ADC�����ַ���
	sADCData.Empty();

	int hb = 0;
	int lb = 0;
	int hbln = 0;
	int lbp = 0;
	int hbhn = 0;
	bool oflow,uflow;
	BYTE LowByte = 0x00;
	BYTE HighByte = 0x00;
//	BYTE bTem;

	CString soffset,slbc,shbln,slbp,shbhn,sresult;

	soffset.Empty();
	slbc.Empty();
	shbln.Empty();
	slbp.Empty();
	shbhn.Empty();
	sresult.Empty();

	for (NumData=0; NumData<pixelNum; NumData++)
	{
		//��8λ��1byte�����ݴ�����offset(step1)���ж����(step2)����result(step3)
		HighByte = RxData[NumData*2+7];
		TemHex.Format("%2X",HighByte);
		TemInt = ChangeNum(TemHex, TemHex.GetLength());
		hb = TemInt;

		int nd = 0;
	
		if(pixelNum == 12) nd= NumData;
		else nd = NumData>>1;

#ifdef ADC_CORRECT
		if(PCRNum == 1) ioffset = (int)(kb1[2*nd]*hb + kb1[2*nd+1]);
		if(PCRNum == 2) ioffset = (int)(kb2[2*nd]*hb + kb2[2*nd+1]);
		if(PCRNum == 3) ioffset = (int)(kb3[2*nd]*hb + kb3[2*nd+1]);
		if(PCRNum == 4) ioffset = (int)(kb4[2*nd]*hb + kb4[2*nd+1]);
#endif

		hbln = hb % 16;		//�õ���������жϵ�hbln  step 2

		hbhn = hb / 16;		//�õ�������result��hbhn  stpe3

		//��8λ��1byte�����ݴ�����lbc
		LowByte = RxData[NumData*2+6];
		TemHex.Format("%2X",LowByte);
		TemInt = ChangeNum(TemHex,TemHex.GetLength());
		lb = TemInt;
		lbc = lb + ioffset;

		//�ж����
		TemHex.Format("%2X",hbln);
		TemInt = ChangeNum(TemHex,TemHex.GetLength());
		lbp = TemInt*16+7;

		if ( (lbp-ioffset) > 255)
			oflow = true;
		else
		{
			if ( (lbp-ioffset) < 0)
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

		if(!gain_mode) {
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
#endif

		if(result < 0) result = 0;

		adc_result[NumData] = result; // zd add

		itoa (result,fstrbuf,10);		//�����ת���ַ�����ʾ
		sADCData += fstrbuf;
		sADCData += " ";

		frame_average[PCRNum] += result; 
	}

	// �༭����������
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

	if(RxData[5] == 11 & pixelNum == 12) {
		itoa (frame_average[PCRNum] / 144, fstrbuf, 10);
		sADCData += " Frame average";
		sADCData += fstrbuf;

		frame_average[PCRNum] = 0;
	}
	else if(RxData[5] == 23 & pixelNum == 24) {
		itoa (frame_average[PCRNum] / 576, fstrbuf, 10);
		sADCData += " Frame average";
		sADCData += fstrbuf;

		frame_average[PCRNum] = 0;
	}

	// ����������Ϊ0x14ʱ��ֻ��ʾ�����ַ���
	if (RxData[2] == 0x14)
	{
		sADCData = "";
		sADCData.Format("the cycle number is %d",RxData[3]);
	}

	if (m_OrigOut && m_ReadHex.GetCheck())			//��ʮ������
	{
		if (m_ShowAllData.GetCheck())	//��ʾ��������
		{
			//			m_PixelData = RegRecStr + m_PixelData;		//���������ڱ༭���һ����ʾ
			m_PixelData += (RegRecStr+"\r\n");					//���������ڱ༭�����һ����ʾ
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			sPCRdata = (RegRecStr+"\r\n");

			//�༭��ֱ�������׶�			
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			RegRecStr = "";
		}

		if (m_ShowValidData.GetCheck())	//��ʾ��Ч����
		{
			//			m_PixelData = Valid_RegRecStr + m_PixelData;	//���������ڱ༭���һ����ʾ
			m_PixelData += (Valid_RegRecStr+"\r\n");					//���������ڱ༭�����һ����ʾ
			SetDlgItemText(IDC_EDIT_RecData,m_PixelData);

			sPCRdata = (Valid_RegRecStr+"\r\n");

			//�༭��ֱ�������׶�
//			POINT pt;
			GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
			Valid_RegRecStr = "";
		}		
	}

	if (m_OrigOut && m_ReadDec.GetCheck())			//��ʮ������ʾ
	{
		//		m_PixelData = sTest + m_PixelData +"\r\n";			//ÿ�����ݽ��ӻس�
		//���������ڱ༭���һ����ʾ
		m_PixelData += (sDecData+"\r\n");						
		SetDlgItemText(IDC_EDIT_RecData,m_PixelData);		//ÿ�����ݽ��ӻس�
		//���������ڱ༭�����һ����ʾ	

		sPCRdata = (sDecData+"\r\n");

		//�༭��ֱ�������׶�
//		POINT pt;
		GetDlgItem(IDC_EDIT_RecData)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_RecData)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
	}

	if (m_PixOut && m_GainMode <= 1)		//ADC������ʾ
	{
		m_ADCRecdata += (sADCData+"\r\n");
		SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//ÿ�����ݽ��ӻس�
		//���������ڱ༭�����һ����ʾ

		sADCRecdata = (sADCData+"\r\n");

		//�༭��ֱ�������׶�
//		POINT pt;
		GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
		pt.x=0;
		GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);

	}

	//-----------------------------------------------
	//�༭���ļ�����	
	// receive data save file
//	m_recdataFile.Write(sPCRdata,sPCRdata.GetLength());

	// ADC data save file
//	m_adcdataFile.Write(sADCRecdata,sADCRecdata.GetLength());

	// ������һ�ΰ������ļ�
//	if ((PCRNum == 4) & (Gra_pageFlag == false))
//	{
//		m_recdataFile.Close();
//		m_adcdataFile.Close();
//	}

	// Drawing
	GraDlgDrawPattern();

	if (Gra_pageFlag)
	{
		GraFlag = readgramsg;
		GraCalMainMsg();
	}

	return 0;
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
//		brush[RxData[5]][i].CreateSolidBrush(RGB(255,0,0));		// ȡ��byteΪ��Чλ
//		brush[RxData[5]][i].CreateSolidBrush(RGB(RxData[i*2+6],RxData[i*2+6],RxData[i*2+6]));		// ȡ��byteΪ��Чλ
	}

	switch (PCRNum)
	{
	case 1:
		if (typePixelSize == 12)		// 12�����
		{
			xOffset = 0;
			yOffset = 0;
		}
		else		// 24�����
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
			switch(PCRNum)
			{
			case 1:
				{
					for(int l=0; l<ColNum12; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					PCRCycCnt ++;		// PCR��ż�1
					cycleRow12();		// ������һ��PCR��ҳָ��
					break;
				}
			case 2:
				{
					for(int l=0; l<ColNum12; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					PCRCycCnt ++;		// PCR��ż�1
					cycleRow12();		// ������һ��PCR��ҳָ��
					break;
				}
			case 3:
				{
					for(int l=0; l<ColNum12; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					PCRCycCnt ++;		// PCR��ż�1
					cycleRow12();		// ������һ��PCR��ҳָ��
					break;
				}
			case 4:
				{
					for(int l=0; l<ColNum12; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}

					PCRNum = 0;
					OnPaint_Flag = DRAWROW;
					Invalidate();
					break;
				}
			}
			//���ݴ���buffer����
			memset(RxData,0,sizeof(RxData));	

			break;
		}
	case dppage12:	
		{
			switch(PCRNum)
			{
			case 1:
				{
					for(int l=0; l<ColNum12; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
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
					break;
				}
			case 2:
				{
					for(int l=0; l<ColNum12; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
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
					break;
				}
			case 3:
				{
					for(int l=0; l<ColNum12; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
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
					break;
				}
			case 4:
				{
					for(int l=0; l<ColNum12; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize12*l + xOffset),(pixelsize12*RxData[5] + yOffset),(pixelsize12*(l+1) + xOffset),(pixelsize12*(RxData[5]+1) + yOffset));
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
					break;
				}
			default:
				break;
			}

			if ((PCRNum == 4) & (Gra_pageFlag == false))
			{
				PCRNum = 0;
				OnPaint_Flag = DRAWPAGE;
				Invalidate();
				Gra_videoFlag = true;		// videoģʽʱ����ʼ������һ��
			}
			else
			{
				Gra_videoFlag = true;		// videoģʽʱ����ʼ������һ��

				if (Gra_pageFlag == false)
				{
					PCRCycCnt ++;		// PCR��ż�1
					cyclePage12();		// ������һ��PCR��ҳָ��
				}
			}
			//���ݴ���buffer����
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x0c:		// ����ʱ�䳬��10ms
		{
			//���ݴ���buffer����
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dprow24:	
		{
			switch(PCRNum)
			{
			case 1:
				{
					for(int l=0; l<ColNum24; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					PCRCycCnt ++;		// PCR��ż�1
					cycleRow24();		// ������һ��PCR��ҳָ��
					break;
				}
			case 2:
				{
					for(int l=0; l<ColNum24; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					PCRCycCnt ++;		// PCR��ż�1
					cycleRow24();		// ������һ��PCR��ҳָ��
					break;
				}
			case 3:
				{
					for(int l=0; l<ColNum24; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					PCRCycCnt ++;		// PCR��ż�1
					cycleRow24();		// ������һ��PCR��ҳָ��
					break;
				}
			case 4:
				{
					for(int l=0; l<ColNum24; l++)		// l�����кţ�rxdata[5]���������к�
					{
						displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
						bgDC_Row.Rectangle(displayRect);
						bgDC_Row.FillRect(&displayRect,&brush[RxData[5]][l]);
					}
					
					PCRNum = 0;
					OnPaint_Flag = DRAWROW;
					Invalidate();
					break;
				}
			}
			//���ݴ���buffer����
			memset(RxData,0,sizeof(RxData));	

			break;
		}
	case dppage24:	
		{
			switch(PCRNum)
			{
			case 1:
				{
					for(int l=0; l<ColNum24; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
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
					break;
				}
			case 2:
				{
					for(int l=0; l<ColNum24; l++)		// l�����к�
					{if(m_GainMode <= 1) 
					{
						displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
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
					break;
				}
			case 3:
				{
					for(int l=0; l<ColNum24; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
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
					break;
				}
			case 4:
				{
					for(int l=0; l<ColNum24; l++)		// l�����к�
					{
						if(m_GainMode <= 1) 
						{
							displayRect.SetRect((pixelsize24*l + xOffset),(pixelsize24*RxData[5] + yOffset),(pixelsize24*(l+1) + xOffset),(pixelsize24*(RxData[5]+1) + yOffset));
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
					break;
				}	
			default:
				break;
			}

			if ((PCRNum == 4) & (Gra_pageFlag == false))
			{
				PCRNum = 0;
				OnPaint_Flag = DRAWPAGE;
				Invalidate();			
				Gra_videoFlag = true;		// videoģʽʱ����ʼ������һ��
			}
			else
			{
				Gra_videoFlag = true;		// videoģʽʱ����ʼ������һ��

				if (Gra_pageFlag == false)
				{
					PCRCycCnt ++;		// PCR��ż�1
					cyclePage24();		// ������һ��PCR��ҳָ��
				}
			}
			//���ݴ���buffer����
			memset(RxData,0,sizeof(RxData));
			break;
		}
	case dpvideo12:	//video12
		{
			//���ݴ���buffer����
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case dpvideo24:	//video24
		{
			//���ݴ���buffer����
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

	// �ж��ǲ��ǵ�һ�ΰ����а�ť
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
	PCRType = (PCRCycCnt<<4)|(0x01);	// �ֱ�����4��PCR��type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
/*
	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4��PCR���η���
	{
		PCRType = (PCRCycCnt<<4)|(0x01);	// �ֱ�����4��PCR��type

		GraFlag = sendgramsg;
		MakeGraPacket(0x02,PCRType,GraBuf[0]);
		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
	}
*/	
}

void CGraDlg::cycleRow12(void)
{
	PCRType = (PCRCycCnt<<4)|(0x01);	// �ֱ�����4��PCR��type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
}


void CGraDlg::OnBnClickedBtnDprow24()
{
	// TODO: Add your control notification handler code here

	// �ж��ǲ��ǵ�һ�ΰ����а�ť
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
	PCRType = (PCRCycCnt<<4)|(0x07);	// �ֱ�����4��PCR��type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
/*
	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4��PCR���η���
	{
		PCRType = (PCRCycCnt<<4)|(0x07);	// �ֱ�����4��PCR��type

		GraFlag = sendgramsg;
		MakeGraPacket(0x02,PCRType,GraBuf[0]);
		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
	}
*/
}


void CGraDlg::cycleRow24(void)
{
	PCRType = (PCRCycCnt<<4)|(0x07);	// �ֱ�����4��PCR��type

	GraFlag = sendgramsg;
	MakeGraPacket(0x02,PCRType,GraBuf[0]);
	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
}


//Start Video
void CGraDlg::OnBnClickedBtnDpvedio()
{
	// TODO: Add your control notification handler code here
/*
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else if(m_GainMode == 1) {
		SetGainMode(0);
	}
	else {
		MessageBox("Video in HDR mode not allowed");
		return;
	}

	GraFlag = sendvideomsg;
	Vedio12_24Flag = 0;		// 12��vedio
	Gra_videoFlag = true;	// video��ʼѭ��
	Gra_pageFlag = TRUE;	// ��ҳ��ʼѭ��

	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
*/

	autoRepeatCounter++;
	SetTimer(1,videoElapseTime,NULL);		// 12��video	
	OnBnClickedButtonCapture();

}


void CGraDlg::OnBnClickedBtnDpvideo24()
{
	// TODO: Add your control notification handler code here
/*
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	if(!m_GainMode) {	
		SetGainMode(1);
	}
	else if(m_GainMode == 1) {
		SetGainMode(0);
	}
	else {
		MessageBox("Video in HDR mode not allowed");
		return;
	}

	GraFlag = sendvideomsg;
	Vedio12_24Flag = 1;		// 24��vedio
	Gra_videoFlag = true;	// video��ʼѭ��
	Gra_pageFlag = TRUE;	// ��ҳ��ʼѭ��

	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
*/

	autoRepeatCounter++;
	SetTimer(1,videoElapseTime,NULL);		// 12��video	
	OnBnClickedButtonCapture();
}


//Stop Video
void CGraDlg::OnBnClickedBtnStopvideo()
{
	// TODO: Add your control notification handler code here

	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}
	
//	GraFlag = sendvideomsg;
//	Gra_videoFlag = false;
//	memset(TxData,0,sizeof(TxData));
//	memset(GraBuf,0,sizeof(GraBuf));
//	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
	
/*
//---------------------------------------------------------------
// ԭvideo���2014-10-10��Ϊ�û�ҳ�ķ�����ʾvideo���������
//---------------------------------------------------------------
	GraFlag = sendgramsg;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x02;		//command
	TxData[2] = 0x0C;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte
	TxData[4] = 0x01;		//real data
	TxData[5] = 0x00;		//Ԥ��λ
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


	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
*/
	KillTimer(1);		// stop video
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
	
	m_ADCRecdata.Empty();
	SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);
}


void CGraDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CString str;
	// �������Ի���
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"�ı��ļ�(*.txt)|*.txt|""�ĵ��ļ�(*.doc)|*.doc|""����ļ�(*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// ���Ի���Ĭ��·��Ϊc��

	// ���б��涯��
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_RecData);	// ��ȡҪ����༭��ؼ��ڵ�����
		// IDC_EDIT_FILE�Ǳ༭��ؼ����
		pst->GetWindowTextA(str);
		ofs << str;
	}
}


void CGraDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	CString str;
	// �������Ի���
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Text(*.txt)|*.txt|""Word doc(*.doc)|*.doc|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// ���Ի���Ĭ��·��Ϊc��

	// ���б��涯��
	if (saveDlg.DoModal() == IDOK)
	{
		ofstream ofs(saveDlg.GetPathName());
		CStatic * pst = (CStatic*)GetDlgItem(IDC_EDIT_ADCDATA);	// ��ȡҪ����༭��ؼ��ڵ�����
		// IDC_EDIT_FILE�Ǳ༭��ؼ����
		pst->GetWindowTextA(str);
		ofs << str;
	}
}



void CGraDlg::OnBnClickedRadioAdcdata()
{
	// TODO: Add your control notification handler code here
}



void CGraDlg::SetGainMode(int gain)
{
	if(gain == gain_mode) return;

	GraFlag = sendgramsg;

	if (!gain)
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
//		GraCalMainMsg();
		gain_mode = 0;
	}
	else	
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
//			GraCalMainMsg();
			gain_mode = 1;
	}

	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����

#ifdef DARK_MANAGE
	
	if(g_pTrimDlg) {
		if(!gain) {
			g_pTrimDlg->SetV20(auto_v20_hg1, 1);
			g_pTrimDlg->SetV20(auto_v20_hg2, 2);
			g_pTrimDlg->SetV20(auto_v20_hg3, 3);
			g_pTrimDlg->SetV20(auto_v20_hg4, 4);
		}
		else {
			g_pTrimDlg->SetV20(auto_v20_lg1, 1);
			g_pTrimDlg->SetV20(auto_v20_lg2, 2);
			g_pTrimDlg->SetV20(auto_v20_lg3, 3);
			g_pTrimDlg->SetV20(auto_v20_lg4, 4);
		}
	}

#endif
}


void CGraDlg::DisplayHDR(void)
{	
	CDC *pDC;		//����Ŀ��DCָ��
	pDC=GetDlgItem(IDC_Bmp)->GetDC();	

	CRect   rect;
	CBrush brush[12][12];	
	int i, j;
	int gray_level;
	int pdata[12][12];

	for (i=0; i<12; i++)
	{
		for(j=0; j<12; j++)
		{
			if(pdata_1[i][j] < 400) pdata[i][j] = pdata_0[i][j];
			else pdata[i][j] = (pdata_1[i][j]-100)*8 + 100;
		}
	}

	for (i=0; i<12; i++)
	{
		for(j=0; j<12; j++)
		{
			gray_level = pdata[i][j]/contrast;
			if(gray_level > 255) gray_level = 255;
			else if(gray_level < 0) gray_level = 0;
			brush[i][j].CreateSolidBrush(RGB(gray_level, gray_level, gray_level));		
		}
	}

	for(i=0; i<12; i++) {
			for(j=0; j<12; j++)		// l�����к�
			{
				rect.SetRect(pixelsize12*j,pixelsize12*i,pixelsize12*(j+1),pixelsize12*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// �����ʾ��ADC�����ַ�?	

	for (i=0; i<12; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<12; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//�����ת���ַ�����ʾ
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//ÿ�����ݽ��ӻس�
			//���������ڱ༭�����һ����ʾ
			//�༭��ֱ�������׶�
			POINT pt;
			GetDlgItem(IDC_EDIT_ADCDATA)->GetScrollRange(SB_VERT,(LPINT)&pt.x,(LPINT)&pt.y);
			pt.x=0;
			GetDlgItem(IDC_EDIT_ADCDATA)->SendMessage(EM_LINESCROLL,pt.x,pt.y);
		}
		sADCData = "";
	}
}


void CGraDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	if(m_GainMode <= 1) return; // only works for HDR mode.

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
		contrast = 116 - pos;
		if(!m_FrameSize)
			DisplayHDR();
		else
			DisplayHDR24();
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
	CDC *pDC;		//����Ŀ��DCָ��
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
			for(j=0; j<24; j++)		// l�����к�
			{
				rect.SetRect(pixelsize24*j,pixelsize24*i,pixelsize24*(j+1),pixelsize24*(i+1));
				pDC->Rectangle(rect);
				pDC->FillRect(&rect,&brush[i][j]);
			}
	}

	CString sRownum;
	char fstrbuf[10];
	CString sADCData;	// �����ʾ��ADC�����ַ�?	

	for (i=0; i<24; i++)
	{
		sRownum.Format(" %d",i);
		for(j=0; j<24; j++)
		{
			itoa (pdata[i][j],fstrbuf,10);		//�����ת���ַ�����ʾ
			sADCData += fstrbuf;
			sADCData += " ";
		}
		sADCData += sRownum;

		if(m_PixOut) {
			m_ADCRecdata += (sADCData+"\r\n");
			SetDlgItemText(IDC_EDIT_ADCDATA,m_ADCRecdata);		//ÿ�����ݽ��ӻس�
			//���������ڱ༭�����һ����ʾ
			//�༭��ֱ�������׶�
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

		// ������ʾָ��
	if(!g_DeviceDetected) {
		MessageBox("ULS24 Device Not Attached");
		return;
	}

	OnBnClickedBtnClearadc(); // Zhimin added. Clear ADC window when capture button is clicked.

	// initial cycle flag
	PCRCycCnt = 0;

	// �ж��ǲ��ǰ���һ�λ�ͼ��ť��capture��
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

	bBmp.CreateCompatibleBitmap(dispalyDC,240,240);
	oldBMP = bgDC.SelectObject(&bBmp);
	bgDC.SetBkColor(RGB(0,0,0));

	//-----------------------------------------------------
	// �����༭�򱣴���ļ�
	//-----------------------------------------------------
//	CString sRecDataFileName,sADCDataFileName;
//	sRecDataFileName.Empty();
//	sADCDataFileName.Empty();

	// ����ʱʱ�䴴���ļ���
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

/*
	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4��PCR���η���
	{

		PCRType = (PCRCycCnt<<4)|(0x02);	// �ֱ�����4��PCR��type

		//	hdr_phase = 0;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// ��ʼѭ��

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
	}
*/

	PCRType = (PCRCycCnt<<4)|(0x02);	// �ֱ�����4��PCR��type

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ��ʼѭ��

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
}


void CGraDlg::cyclePage12(void)
{
	PCRType = (PCRCycCnt<<4)|(0x02);	// �ֱ�����4��PCR��type

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ��ʼѭ��

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
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
/*
	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4��PCR���η���
	{
		PCRType = (PCRCycCnt<<4)|(0x08);	// �ֱ�����4��PCR��type

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// ��ʼѭ��

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
	}
*/
	PCRType = (PCRCycCnt<<4)|(0x08);	// �ֱ�����4��PCR��type

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ��ʼѭ��

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
}


void CGraDlg::cyclePage24(void)
{
	PCRType = (PCRCycCnt<<4)|(0x08);	// �ֱ�����4��PCR��type

	GraFlag = sendpagemsg;
	Gra_pageFlag = TRUE;	// ��ʼѭ��

	MakeGraPacket(0x02,PCRType,0xff);

	//Send message to main dialog
	GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����
}


void CGraDlg::OnBnClickedBtnDppage12()
{
	// TODO: Add your control notification handler code here

	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4��PCR���η���
	{
		PCRType = (PCRCycCnt<<4)|(0x02);	// �ֱ�����4��PCR��type

		SetGainMode(0);
		hdr_phase = 0;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// ��ʼѭ��

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����

		//===========
		SetGainMode(1);
		hdr_phase = 1;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// ��ʼѭ��

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����

		DisplayHDR();
	}

}


void CGraDlg::OnBnClickedBtnDppage24()
{
	// TODO: Add your control notification handler code here

	for(PCRCycCnt=0; PCRCycCnt<4; PCRCycCnt++)	// 4��PCR���η���
	{
		PCRType = (PCRCycCnt<<4)|(0x08);	// �ֱ�����4��PCR��type

		SetGainMode(0);
		hdr_phase = 0;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// ��ʼѭ��

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����

		//===========
		SetGainMode(1);
		hdr_phase = 1;

		GraFlag = sendpagemsg;
		Gra_pageFlag = TRUE;	// ��ʼѭ��

		MakeGraPacket(0x02,PCRType,0xff);

		//Send message to main dialog
		GraCalMainMsg();		//�������Ի��򴮿ڷ�����Ϣ����

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
			dispalyDC -> BitBlt(0,0,240,240,&bgDC,0,0,SRCCOPY);
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
	case 1:		// ����video����͡����ݶ�ȡ
		{
			if (Gra_videoFlag) {
				autoRepeatCounter++;
				OnBnClickedButtonCapture();
				if(autoRepeatCounter >= autoRepeatMaxCount) {
					KillTimer(1);
				}
			}
			break;
		}
	default:
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

// OperationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "OperationDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <afx.h>
using namespace std;


//***************************************************************
//Global variable definition
//***************************************************************
// Parameter variable in edit
CString sPITem_write,sCycle;
CString sInidenTem_write,sInidenTim_write;
CString sDenTem_write,sDenTim_write;
CString sAnnTem_write,sAnnTim_write;
CString sExtTem_write,sExtTim_write;
CString sHoldTem_write,sHoldTim_write;
CString sSave;		// save parameter buffer

int OperDlgFlag = 0;			// operation dialog message flag
int OperReadstautsFlag = 0;		// read status flag
int cntReadstatuspel = 0;		// read status edit clear counter for peliter
int cntReadstatuspi = 0;		// read status edit clear counter for pi heat

int opeDlgTimerFlag = 0;		// timer flag

CBrush m_editBrush;				// edit back-color brush

BYTE OperBuf [200];				// operation dialog buffer

//CFile m_readPIFile(_T("Read_PEL_Data.txt"),CFile::modeCreate|CFile::modeWrite);			// read peliter file
//CFile m_readPELFile(_T("Read_PI_Data.txt"),CFile::modeCreate|CFile::modeWrite);			// read pi file

CFile m_readPIFile;			// read peliter file
CFile m_readPELFile;			// read pi file

CTime sysTime;					// system time
int iYear,iMonth,iDay;			// year / month / day
int iHour,iMinute,iSecond;		// hour / minute / second
CString sTime;					// system string format


//***************************************************************
// Own function
//***************************************************************



// COperationDlg dialog

IMPLEMENT_DYNAMIC(COperationDlg, CDialog)

COperationDlg::COperationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COperationDlg::IDD, pParent)
{

	m_operdlg_sEditpel = _T("");
	m_operdlg_sEditpiread = _T("");
	m_operdlg_sEditcycleread = _T("");
	//  m_operdlg_sEditpwm = _T("");
	m_operdlg_sEditvanstatus = _T("");
	m_operdlg_sEditcurstatus = _T("");
	//  m_operdlg_sEditpwm2 = _T("");
	//  m_operdlg_sEditpwm1 = _T("");
}

COperationDlg::~COperationDlg()
{
}

void COperationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_OPE_PEL, m_operdlg_sEditpel);
	DDX_Text(pDX, IDC_EDIT_OPE_PIREAD, m_operdlg_sEditpiread);
	DDX_Text(pDX, IDC_EDIT_OPE_CYCLEREAD, m_operdlg_sEditcycleread);
	//  DDX_Control(pDX, IDC_EDIT_OPE_PWM, m_operdlg_sEditpwm);
	//  DDX_Text(pDX, IDC_EDIT_OPE_PWM, m_operdlg_sEditpwm);
	DDX_Text(pDX, IDC_EDIT_OPE_VANSTATUS, m_operdlg_sEditvanstatus);
	DDX_Text(pDX, IDC_EDIT_OPE_CURRENTSTATUS, m_operdlg_sEditcurstatus);
	//  DDX_Text(pDX, IDC_EDIT_OPE_PWM2, m_operdlg_sEditpwm2);
	//  DDX_Text(pDX, IDC_EDIT_OPE_PWM, m_operdlg_sEditpwm1);
}


BEGIN_MESSAGE_MAP(COperationDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_OPE_LOAD, &COperationDlg::OnBnClickedBtnOpeLoad)
	ON_BN_CLICKED(IDC_BTN_OPE_SAVESTATUS, &COperationDlg::OnBnClickedBtnOpeSavestatus)
	ON_BN_CLICKED(IDC_BTN_OPE_CLEAR, &COperationDlg::OnBnClickedBtnOpeClear)
//	ON_EN_CHANGE(IDC_EDIT_OPE_DENTEM, &COperationDlg::OnEnChangeEditOpeDentem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_PITEM, &COperationDlg::OnEnKillfocusEditOpePitem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENTEM, &COperationDlg::OnEnKillfocusEditOpeDentem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENTIME, &COperationDlg::OnEnKillfocusEditOpeDentime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENINGTEM, &COperationDlg::OnEnKillfocusEditOpeDeningtem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_DENINGTIME, &COperationDlg::OnEnKillfocusEditOpeDeningtime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNTEM, &COperationDlg::OnEnKillfocusEditOpeAnntem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNTIME, &COperationDlg::OnEnKillfocusEditOpeAnntime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_ANNCYCLE, &COperationDlg::OnEnKillfocusEditOpeAnncycle)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_EXTTEM, &COperationDlg::OnEnKillfocusEditOpeExttem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_EXTTIME, &COperationDlg::OnEnKillfocusEditOpeExttime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_HOLDTEM, &COperationDlg::OnEnKillfocusEditOpeHoldtem)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_HOLDTIME, &COperationDlg::OnEnKillfocusEditOpeHoldtime)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KP, &COperationDlg::OnEnKillfocusEditOpeKp)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KI, &COperationDlg::OnEnKillfocusEditOpeKi)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KD, &COperationDlg::OnEnKillfocusEditOpeKd)
ON_EN_KILLFOCUS(IDC_EDIT_OPE_KL, &COperationDlg::OnEnKillfocusEditOpeKl)
ON_BN_CLICKED(IDC_BTN_OPE_SAVEPARA, &COperationDlg::OnBnClickedBtnOpeSavepara)
ON_BN_CLICKED(IDC_BTN_OPE_MORE, &COperationDlg::OnBnClickedBtnOpeMore)
ON_MESSAGE(UM_OPERPROCESS,OnOpearProcess)
ON_MESSAGE(UM_OPERPIDPROCESS,OnOpearPIDProcess)
ON_MESSAGE(UM_OPERLEDPROCESS,OnOpearLEDProcess)
ON_MESSAGE(UM_OPEROVERSHOOTPROCESS,OnOpearOvershootProcess)
ON_BN_CLICKED(IDC_BTN_OPE_READSTART, &COperationDlg::OnBnClickedBtnOpeReadstart)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BTN_OPE_START, &COperationDlg::OnBnClickedBtnOpeStart)
ON_WM_CTLCOLOR()
ON_BN_CLICKED(IDC_BTN_OPE_LEDSET, &COperationDlg::OnBnClickedBtnOpeLedset)
ON_BN_CLICKED(IDC_BTN_OPE_LEDREAD, &COperationDlg::OnBnClickedBtnOpeLedread)
ON_BN_CLICKED(IDC_BTN_OPE_FANON, &COperationDlg::OnBnClickedBtnOpeFanon)
ON_BN_CLICKED(IDC_BTN_OPE_FANOFF, &COperationDlg::OnBnClickedBtnOpeFanoff)
ON_BN_CLICKED(IDC_BTN_OPE_LEDON, &COperationDlg::OnBnClickedBtnOpeLedon)
ON_BN_CLICKED(IDC_BTN_OPE_LENOFF, &COperationDlg::OnBnClickedBtnOpeLenoff)
ON_BN_CLICKED(IDC_BTN_OPE_KPSET, &COperationDlg::OnBnClickedBtnOpeKpset)
ON_BN_CLICKED(IDC_BTN_OPE_KDSET, &COperationDlg::OnBnClickedBtnOpeKdset)
ON_BN_CLICKED(IDC_BTN_OPE_KISET, &COperationDlg::OnBnClickedBtnOpeKiset)
ON_BN_CLICKED(IDC_BTN_OPE_KLSET, &COperationDlg::OnBnClickedBtnOpeKlset)
ON_BN_CLICKED(IDC_BTN_OPE_PIDREAD, &COperationDlg::OnBnClickedBtnOpePidread)
ON_BN_CLICKED(IDC_BTN_OPE_PIDZONESET, &COperationDlg::OnBnClickedBtnOpePidzoneset)
ON_BN_CLICKED(IDC_BTN_OPE_STOP, &COperationDlg::OnBnClickedBtnOpeStop)
ON_BN_CLICKED(IDC_BTN_OPE_FANCLOSETEMP, &COperationDlg::OnBnClickedBtnOpeFanclosetemp)
ON_BN_CLICKED(IDC_BTN_OPE_FANREAD, &COperationDlg::OnBnClickedBtnOpeFanread)
ON_BN_CLICKED(IDC_BTN_OPE_OVERSEND, &COperationDlg::OnBnClickedBtnOpeOversend)
ON_BN_CLICKED(IDC_BTN_OPE_OVERREAD, &COperationDlg::OnBnClickedBtnOpeOverread)
END_MESSAGE_MAP()


//*********************************************************************
//Own function
//*********************************************************************

//调用主对话框对应消息处理函数
void COperationDlg::OperCalMainMsg()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_OperDlg_event,a,b);
}


void COperationDlg::OperReadStatus()
{
	if (OperReadstautsFlag == 6)	// 所有状态读取完毕
	{
		OperReadstautsFlag = 0;		// 读取序号清零
	}
	else
	{
		switch(OperReadstautsFlag)
		{
		case 2:		// 读取PI heat温度
			{
				OperDlgFlag = READSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x10;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x02;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		case 3:		// 读取风扇状态
			{
				OperDlgFlag = READSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x10;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x0A;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		case 4:		// 读取当前循环数和阶段数
			{
				OperDlgFlag = READSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x14;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x01;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		case 5:		// 读取当前通道工作状态
			{
				OperDlgFlag = READSTATUS;

				TxData[0] = 0xaa;		//preamble code
				TxData[1] = 0x14;		//command
				TxData[2] = 0x0C;		//data length
				TxData[3] = 0x02;		//data type, date edit first byte
				TxData[4] = 0x01;		//real data
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

				//Send message to main dialog
				OperCalMainMsg();		//调用主对话框串口发送消息程序
				break;
			}
		default:
			break;
		}
	}
}


LRESULT COperationDlg::OnOpearProcess(WPARAM wParam, LPARAM lParam)
{
	if (OperReadstautsFlag !=0 )
	{
		// add received data decoder code here
		switch(OperReadstautsFlag)
		{
		case 1:		// decoder peltier temperature
			{
				if ( cntReadstatuspel == 1000)		// 显示满1000行后编辑框清零
				{
					m_operdlg_sEditpel.Empty();
					SetDlgItemTextA(IDC_EDIT_OPE_PEL,m_operdlg_sEditpel);	// edit display
					cntReadstatuspel = 0;
				}
				else
				{
					// float处理
					CString sTem;
					unsigned char cTem[] = {RxData[5],RxData[6],RxData[7],RxData[8]};
					float * fTem = (float *)cTem;
					sTem.Format("%g",*fTem);

					m_operdlg_sEditpel += (sTem + "\r\n");		// 取出接收的float数据
					SetDlgItemTextA(IDC_EDIT_OPE_PEL,m_operdlg_sEditpel);	// edit display
					
					//编辑框垂直滚动到底端
					POINT ptPel;
					GetDlgItem(IDC_EDIT_OPE_PEL)->GetScrollRange(SB_VERT,(LPINT)&ptPel.x,(LPINT)&ptPel.y);
					ptPel.x=0;
					GetDlgItem(IDC_EDIT_OPE_PEL)->SendMessage(EM_LINESCROLL,ptPel.x,ptPel.y);

					CString sPeldata;
					sPeldata.Empty();
					sPeldata += (sTem + "\r\n");
					// receive data save file
					m_readPELFile.Write(sPeldata,sPeldata.GetLength());

					//清除数据传输buffer
					memset(RxData,0,sizeof(RxData));

					cntReadstatuspel++;
					OperReadstautsFlag += 1;
					OperReadStatus();	// start next read
				}
				break;
			}
		case 2:		// decoder PI heater
			{
				if ( cntReadstatuspi == 1000)		// 显示满1000行后编辑框清零
				{
					m_operdlg_sEditpiread.Empty();
					SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,m_operdlg_sEditpiread);		// edit display
					cntReadstatuspi = 0;
				}
				else
				{
					// float处理
					CString sTem;
					unsigned char cTem[] = {RxData[5],RxData[6],RxData[7],RxData[8]};
					float * fTem = (float *)cTem;
					sTem.Format("%g",*fTem);

					m_operdlg_sEditpiread += (sTem + "\r\n");		// 取出接收的float数据

					SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,m_operdlg_sEditpiread);		// edit display
					
					//编辑框垂直滚动到底端
					POINT ptPi;
					GetDlgItem(IDC_EDIT_OPE_PIREAD)->GetScrollRange(SB_VERT,(LPINT)&ptPi.x,(LPINT)&ptPi.y);
					ptPi.x=0;
					GetDlgItem(IDC_EDIT_OPE_PIREAD)->SendMessage(EM_LINESCROLL,ptPi.x,ptPi.y);

					CString sPidata;
					sPidata.Empty();
					sPidata += (sTem + "\r\n");
					// receive data save file
					m_readPIFile.Write(sPidata,sPidata.GetLength());

					//清除数据传输buffer
					memset(RxData,0,sizeof(RxData));

					cntReadstatuspi++;
					OperReadstautsFlag += 1;
					OperReadStatus();	// start next read
				}
				break;
			}
		case 3:		// 读取风扇状态
			{
				CString sTem;
				if (RxData[5] == 0x00)
				//	m_operdlg_sEditvanstatus = "OFF";
				sTem = "OFF";
				else
				{
					if (RxData [5] == 0x01)
					//	m_operdlg_sEditvanstatus = "ON";
						sTem = "ON";
					else
					//	m_operdlg_sEditvanstatus = "ERROR";
						sTem = "ERROR";
				}

				SetDlgItemTextA(IDC_EDIT_OPE_VANSTATUS,sTem);		// edit display

				OperReadstautsFlag += 1;
				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));
				
//				OperReadstautsFlag += 1;
				OperReadStatus();	// start next read
				break;
			}
		case 4:		// 读取cycle状态
			{
				CString stem;
				stem.Format("%d",RxData[6]);			// 当前第几个循环
				m_operdlg_sEditcycleread = stem + " / ";
				stem.Empty();
				stem.Format("%d",RxData[5]);			// 当前第几个阶段
				m_operdlg_sEditcycleread += stem;

				SetDlgItemTextA(IDC_EDIT_OPE_CYCLEREAD,m_operdlg_sEditcycleread);		// edit display

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

				OperReadstautsFlag += 1;
				OperReadStatus();	// start next read
				break;
			}
		case 5:		// 读取cycle、pwm、current状态
			{
				BYTE bpwmPel,bpwmPi,bCur;
				bCur = RxData[9] & 0x80;
				bpwmPel = RxData[9] & 0x70;
				bpwmPi = RxData[9] & 0x07;
				
				// display current status
				if (bCur == 0)
					SetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS, "positive");
				else
					SetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS,"negative");

				// display peliter pwm status
				switch(bpwmPel)
				{
				case 0x00:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Idle");
						break;
					}
				case 0x10:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Start");
						break;
					}
				case 0x20:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Adjust");
						break;
					}
				case 0x30:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Count Start");
						break;
					}
				case 0x40:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,"Count End");
						break;
					}
				default:
					break;
				}

				// display PI pwm status
				switch(bpwmPi)
				{
				case 0x00:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Idle");
						break;
					}
				case 0x01:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Start");
						break;
					}
				case 0x02:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Adjust");
						break;
					}
				case 0x03:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Count Start");
						break;
					}
				case 0x04:
					{
						SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,"Count End");
						break;
					}
				default:
					break;
				}

				//清除数据传输buffer
				memset(RxData,0,sizeof(RxData));

				OperReadstautsFlag += 1;
				OperReadStatus();	// start next read
				break;
			}
		default:
			break;
		}
	}

	return 0;
}

LRESULT COperationDlg::OnOpearPIDProcess(WPARAM wParam, LPARAM lParam)
{
	CString sKP0,sKP1,sKI0,sKI1,sKD0,sKD1,sKL0,sKL1,sTemp;
	sKP0.Empty();
	sKP1.Empty();
	sKI0.Empty();
	sKI1.Empty();
	sKD0.Empty();
	sKD1.Empty();
	sKL0.Empty();
	sKL1.Empty();
	sTemp.Empty();
	
	//将接收的4byte格式浮点数据转为浮点字符串
	CString sTem;
	sTem.Empty();
//	unsigned char cTem[] = {RxData[6],RxData[7],RxData[8],RxData[9]};
//	float * fTem = (float *)cTem;
//	sTem.Format("%g",*fTem);
	unsigned char cTem[4];
	float * fTem;
	
	// get tempture
	cTem[0] = RxData[6];
	cTem[1] = RxData[7];
	cTem[2] = RxData[8];
	cTem[3] = RxData[9];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sTemp = sTem;

	// get kp0
	sTem.Empty();
	cTem[0] = RxData[10];
	cTem[1] = RxData[11];
	cTem[2] = RxData[12];
	cTem[3] = RxData[13];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKP0 = sTem;

	// get ki0
	sTem.Empty();
	cTem[0] = RxData[14];
	cTem[1] = RxData[15];
	cTem[2] = RxData[16];
	cTem[3] = RxData[17];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKI0 = sTem;

	// get kd0
	sTem.Empty();
	cTem[0] = RxData[18];
	cTem[1] = RxData[19];
	cTem[2] = RxData[20];
	cTem[3] = RxData[21];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKD0 = sTem;

	// get kl0
	sTem.Empty();
	cTem[0] = RxData[22];
	cTem[1] = RxData[23];
	cTem[2] = RxData[24];
	cTem[3] = RxData[25];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKL0 = sTem;

	// get kp1
	sTem.Empty();
	cTem[0] = RxData[26];
	cTem[1] = RxData[27];
	cTem[2] = RxData[28];
	cTem[3] = RxData[29];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKP1 = sTem;

	// get ki1
	sTem.Empty();
	cTem[0] = RxData[30];
	cTem[1] = RxData[31];
	cTem[2] = RxData[32];
	cTem[3] = RxData[33];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKI1 = sTem;

	// get kd1
	sTem.Empty();
	cTem[0] = RxData[34];
	cTem[1] = RxData[35];
	cTem[2] = RxData[36];
	cTem[3] = RxData[37];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKD1 = sTem;

	// get kl1
	sTem.Empty();
	cTem[0] = RxData[38];
	cTem[1] = RxData[39];
	cTem[2] = RxData[40];
	cTem[3] = RxData[41];
	fTem = (float *)cTem;
	sTem.Format("%g",*fTem);
	sKL1 = sTem;

	SetDlgItemTextA(IDC_EDIT_OPE_KP0,sKP0);
	SetDlgItemTextA(IDC_EDIT_OPE_KI0,sKI0);
	SetDlgItemTextA(IDC_EDIT_OPE_KD0,sKD0);
	SetDlgItemTextA(IDC_EDIT_OPE_KL0,sKL0);
	SetDlgItemTextA(IDC_EDIT_OPE_KP1,sKP1);
	SetDlgItemTextA(IDC_EDIT_OPE_KI1,sKI1);
	SetDlgItemTextA(IDC_EDIT_OPE_KD1,sKD1);
	SetDlgItemTextA(IDC_EDIT_OPE_KL1,sKL1);
	SetDlgItemTextA(IDC_EDIT_OPE_PIDZONE,sTemp);

	//清除数据传输buffer
	memset(RxData,0,sizeof(RxData));

	return 0;
}


LRESULT COperationDlg::OnOpearLEDProcess(WPARAM wParam, LPARAM lParam)
{
	CString sSetup,sHold;
	sSetup.Empty();
	sHold.Empty();

	//将接收的4byte格式浮点数据转为浮点字符串
	CString sTem;
	sTem.Empty();
	//	unsigned char cTem[] = {RxData[6],RxData[7],RxData[8],RxData[9]};
	//	float * fTem = (float *)cTem;
	//	sTem.Format("%g",*fTem);
	unsigned char cTem[4];
	float * fTem;

	switch(RxData[4])
	{
	case 0x21:		// read led setup time
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sSetup = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_LEDSETTIMEREAD,sSetup);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x22:		// read led hold time
		{
			// get hold time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sHold = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_LEDHOLDTIMEREAD,sHold);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x05:		// read fan pre-close temperature
		{
			// get the temperature
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sHold = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_FANREAD,sHold);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));
		}
	default:	break;
	}
	return 0;
}


LRESULT COperationDlg::OnOpearOvershootProcess(WPARAM wParam, LPARAM lParam)
{
	CString sTime,sTemp;
	sTime.Empty();
	sTemp.Empty();

	//将接收的4byte格式浮点数据转为浮点字符串
	CString sTem;
	sTem.Empty();
	//	unsigned char cTem[] = {RxData[6],RxData[7],RxData[8],RxData[9]};
	//	float * fTem = (float *)cTem;
	//	sTem.Format("%g",*fTem);
	unsigned char cTem[4];
	float * fTem;

	switch(RxData[4])
	{
	case 0x05:		// read overshoot time
		{
			// get setup time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTime = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_OVERREADTIME,sTime);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	case 0x06:		// read overshoot temp
		{
			// get hold time
			cTem[0] = RxData[5];
			cTem[1] = RxData[6];
			cTem[2] = RxData[7];
			cTem[3] = RxData[8];
			fTem = (float *)cTem;
			sTem.Format("%g",*fTem);

			sTemp = sTem;
			SetDlgItemTextA(IDC_EDIT_OPE_OVERREADTEMP,sTemp);
			//清除数据传输buffer
			memset(RxData,0,sizeof(RxData));

			break;
		}
	default:	break;
	}
	return 0;
}


BOOL COperationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	// initial edit back-color brush
	m_editBrush.CreateSolidBrush(RGB(255,250,0));

	// Set parameter default
	sPITem_write = PITEMP;
	sCycle = ANNCYCLE;
	sInidenTem_write = INIDENTEMP;
	sInidenTim_write = INIDENTIME;
	sDenTem_write = DENTEMP;
	sDenTim_write = DENTIME;
	sAnnTem_write = ANNTEMP;
	sAnnTim_write = ANNTIME;
	sExtTem_write = EXTTEMP;
	sExtTim_write = EXTTIME;
	sHoldTem_write = HOLDTEMP;
	sHoldTim_write = HOLDTIME;

	SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
	SetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sInidenTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
	SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);
//	SetDlgItemTextA(IDC_EDIT_OPE_KP,sKP);
//	SetDlgItemTextA(IDC_EDIT_OPE_KI,sKI);
//	SetDlgItemTextA(IDC_EDIT_OPE_KD,sKD);
//	SetDlgItemTextA(IDC_EDIT_OPE_KL,sKL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


// COperationDlg message handlers


BOOL COperationDlg::PreTranslateMessage(MSG* pMsg)
{

if ((WM_KEYFIRST <= pMsg->message) && (pMsg->message <= WM_KEYLAST))
{
	if (pMsg->wParam == VK_RETURN)
	{
		::SetFocus(m_hWnd);
		return TRUE;
	}
}
return FALSE;
}


void COperationDlg::OnEnKillfocusEditOpePitem()
{
	// TODO: Add your control notification handler code here
	
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_PITEM,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sPITem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
}


void COperationDlg::OnEnKillfocusEditOpeDentem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENTEM,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sInidenTem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sInidenTem_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sInidenTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeDentime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENTIME,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sInidenTim_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeDeningtem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENINGTEM,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sDenTem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeDeningtime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_DENINGTIME,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sDenTim_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnntem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNTEM,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sAnnTem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnntime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sAnnTim_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeAnncycle()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sCycle = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
}


void COperationDlg::OnEnKillfocusEditOpeExttem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_EXTTEM,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sExtTem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeExttime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_EXTTIME,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sExtTim_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeHoldtem()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_HOLDTEM,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sHoldTem_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
}


void COperationDlg::OnEnKillfocusEditOpeHoldtime()
{
	// TODO: Add your control notification handler code here

	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_HOLDTIME,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sHoldTim_write = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);
}


void COperationDlg::OnEnKillfocusEditOpeKp()
{
	// TODO: Add your control notification handler code here
/*	
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KP,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKP = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KP,sKP);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KP,sKP);
*/
}


void COperationDlg::OnEnKillfocusEditOpeKi()
{
	// TODO: Add your control notification handler code here
/*
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KI,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKI = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KI,sKI);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KI,sKI);
*/
}


void COperationDlg::OnEnKillfocusEditOpeKd()
{
	// TODO: Add your control notification handler code here
/*
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KD,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKD = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KD,sKD);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KD,sKD);
*/
}


void COperationDlg::OnEnKillfocusEditOpeKl()
{
	// TODO: Add your control notification handler code here
/*
	CString sTemp;
	sTemp.Empty();
	GetDlgItemText(IDC_EDIT_OPE_KL,sTemp);
	if (MessageBox("参数将被更改为"+sTemp,"参数更改",MB_YESNO) == IDYES)
	{
		sKL = sTemp;
		SetDlgItemTextA(IDC_EDIT_OPE_KL,sKL);
	}
	else
		SetDlgItemTextA(IDC_EDIT_OPE_KL,sKL);
*/
}


void COperationDlg::OnBnClickedBtnOpeSavestatus()
{
	// TODO: Add your control notification handler code here

	CString sTemp,sStatus;
	sTemp.Empty();
	sStatus.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PEL,sTemp);
	sStatus += (sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PIREAD,sTemp);
	sStatus += (sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_CYCLEREAD,sTemp);
	sStatus += (sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_PWM,sTemp);
	sStatus += (sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_VANSTATUS,sTemp);
	sStatus += (sTemp + "\r\n");
	sTemp.Empty();
	GetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS,sTemp);
	sStatus += (sTemp + "\r\n");

	// 创建另存对话框
	CFileDialog saveDlg(FALSE,".txt",
		NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"文本文件(*.txt)|*.txt|""文档文件(*.doc)|*.doc|""表格文件(*.xls)|*.xls|""All Files(*.*)|*.*||",
		NULL,0,TRUE);

	//	saveDlg.m_ofn.lpstrInitialDir = "c:\\";		// 另存对话框默认路径为c盘

	// 进行保存动作
	if (saveDlg.DoModal() == IDOK)
	{
		CFile nFile(saveDlg.GetPathName(),CFile::modeCreate | CFile::modeReadWrite);   // 创建文件，用选取的文件名
		nFile.Write(sStatus,sStatus.GetLength());		// 将要保存的数据写入文件 （CString sStatus为要写入的内容）
		nFile.Close();		// 完成文件写入
	}
}


void COperationDlg::OnBnClickedBtnOpeClear()
{
	// TODO: Add your control notification handler code here
	CString sEmpty;
	sEmpty.Empty();
	SetDlgItemTextA(IDC_EDIT_OPE_PEL,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_PIREAD,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_CYCLEREAD,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_PWMPEL,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_PWMPI,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_VANSTATUS,sEmpty);
	SetDlgItemTextA(IDC_EDIT_OPE_CURRENTSTATUS,sEmpty);
	m_operdlg_sEditpel.Empty();
	m_operdlg_sEditpiread.Empty();
}


void COperationDlg::OnBnClickedBtnOpeSavepara()
{
	// TODO: Add your control notification handler code here

	// Get current parameter setting

	// 显示保存窗口
	m_ParsaveDlg.Create(IDD_PARSAVE_DIALOG,this);
	m_ParsaveDlg.ShowWindow(1);

	// 获取参数值
	CString sTemp;
	GetDlgItemTextA(IDC_EDIT_OPE_PITEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");
	GetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sTemp);
//	sSave += (sTemp + "\r\n");
	sSave += (sTemp + " ");

}


//int StartPos = 0;	
//int EndPos = 0;		
//int cNum = 0;		
//int enterkeyNum = 0;	
void COperationDlg::OnBnClickedBtnOpeLoad()
{
	// TODO: Add your control notification handler code here
	
	CString sFilePath;
	CFileDialog FileOpenDialog(TRUE);
	FileOpenDialog.m_ofn.Flags = OFN_ENABLEHOOK|OFN_EXPLORER;
	CString strFilter;
	strFilter = _T("TXT");
	strFilter += (TCHAR)'\0'; 
	strFilter += _T("*.txt");
	strFilter += (TCHAR)'\0';  
	CString allFilter;
	VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
	strFilter += allFilter;
	strFilter += (TCHAR)'\0';  
	strFilter += _T("*.*");
	strFilter += (TCHAR)'\0';   
	FileOpenDialog.m_ofn.lpstrFilter = strFilter;
	FileOpenDialog.m_ofn.lpstrTitle = "Download Filename...";
	CString sFileName;
	sFileName.Empty();
	if ((((CMDIFrameWnd *) AfxGetMainWnd())->MDIGetActive()) != NULL)
		sFileName = (((CMDIFrameWnd *) AfxGetMainWnd())->MDIGetActive())->GetActiveDocument()->GetTitle();
	if (!sFileName.IsEmpty())
	{
		if (sFileName.Find('.') != -1)
			sFileName = sFileName.Left(sFileName.Find('.'));
		sFileName += _T(".hex");
		strcpy(FileOpenDialog.m_ofn.lpstrFile,LPCTSTR(sFileName));
	}
	BOOL bResult = FileOpenDialog.DoModal() == IDOK ? TRUE : FALSE;
	if (bResult)
	{
		sFilePath = FileOpenDialog.GetPathName();	
		HANDLE fileHand;
		DWORD fileSize;
		fileHand = CreateFile(sFilePath,0,0,0,OPEN_EXISTING,0,0);
		fileSize = GetFileSize(fileHand,0);
		char * bufFile = new char [fileSize];
		memset(bufFile,0,fileSize);
		CFile file;
		file.Open(sFilePath,CFile::modeRead);	
		file.Read(bufFile,fileSize);
		CString sFile;
		sFile = bufFile;
		delete [] bufFile;
//		CString sFind = "\r\n";
		CString sFind = " ";
		int StartPos = 0;	
		int EndPos = 0;		
		int cNum = 0;		
		int enterkeyNum = 0;	
		while((EndPos=sFile.Find(sFind,StartPos)) != -1)
		{
			StartPos = EndPos+1;
			enterkeyNum ++;
		}
		CString* sEdit = new CString [enterkeyNum];	
		for(int i=0; i<enterkeyNum; i++)
			sEdit[i] = "";
		StartPos = 0;
		EndPos = 0;
		if (enterkeyNum != 13)		
		{
			AfxMessageBox("The number of parameter is wrong");
		}
		else
		{
			for (int j=0; j<enterkeyNum; j++)
			{
				EndPos = sFile.Find(sFind,StartPos);
				cNum = EndPos - StartPos;
				unsigned char * cEdit = new unsigned char[cNum];
				memset(cEdit,0,cNum);
				for (int i=0; i<cNum; i++)
				{
//					cEdit[i] = sFile[(StartPos+i)];
					cEdit[i] = sFile.GetAt(StartPos+i);
				}
				sEdit[j] = cEdit;
				StartPos = EndPos+1;
				delete[] cEdit;
			}			
		}
		sPITem_write = sEdit[1];
		sInidenTem_write = sEdit[2];
		sInidenTim_write = sEdit[3];
		sDenTem_write = sEdit[4];
		sDenTim_write = sEdit[5];
		sAnnTem_write = sEdit[6];
		sAnnTim_write = sEdit[7];
		sCycle = sEdit[8];
		sExtTem_write = sEdit[9];
		sExtTim_write = sEdit[10];
		sHoldTem_write = sEdit[11];
		sHoldTim_write = sEdit[12];

		delete []sEdit;

		SetDlgItemTextA(IDC_EDIT_OPE_PITEM,sPITem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENTEM,sInidenTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENTIME,sInidenTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTEM,sDenTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_DENINGTIME,sDenTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTEM,sAnnTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_ANNTIME,sAnnTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_ANNCYCLE,sCycle);
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTEM,sExtTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_EXTTIME,sExtTim_write);
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTEM,sHoldTem_write);
		SetDlgItemTextA(IDC_EDIT_OPE_HOLDTIME,sHoldTim_write);
	}
	else
		AfxMessageBox("Can't find the file");
	
	
}


void COperationDlg::OnBnClickedBtnOpeMore()
{
	// TODO: Add your control notification handler code here
	m_ManuDlg.Create(IDD_MANUALCTR_DIALOG,this);
	m_ManuDlg.ShowWindow(1);
}


void COperationDlg::OnBnClickedBtnOpeReadstart()
{
	// TODO: Add your control notification handler code here

	CString sPELfilename,sPIfilename;
	sPELfilename.Empty();
	sPIfilename.Empty();

	if (opeDlgTimerFlag == 0 )	// timer 未打开
	{
		opeDlgTimerFlag = 1;
		SetTimer(1,1000,NULL);	//设置连续发送的timer1
		sysTime = CTime::GetCurrentTime();
		sTime.Format("%4d-%.2d-%.2d_%.2d%.2d%.2d",
					sysTime.GetYear(),sysTime.GetMonth(),sysTime.GetDay(),
					sysTime.GetHour(),sysTime.GetMinute(),sysTime.GetSecond());
		sPELfilename = "PELTemp_" + sTime + ".txt";
		sPIfilename = "PITemp_" + sTime + ".txt";
		m_readPIFile.Open(sPIfilename,CFile::modeCreate|CFile::modeWrite);
		m_readPELFile.Open(sPELfilename,CFile::modeCreate|CFile::modeWrite);
	}		
	else
	{
		opeDlgTimerFlag = 0;
		KillTimer(1);

		m_readPELFile.Close();
		m_readPIFile.Close();
	}				
	
}


void COperationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	switch (nIDEvent)
	{
	case 1:
		{
			// 读取peltier温度
			OperReadstautsFlag = 1;
			OperDlgFlag = READSTATUS;

			TxData[0] = 0xaa;		//preamble code
			TxData[1] = 0x10;		//command
			TxData[2] = 0x0C;		//data length
			TxData[3] = 0x02;		//data type, date edit first byte
			TxData[4] = 0x02;		//real data
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

			//Send message to main dialog
			OperCalMainMsg();		//调用主对话框串口发送消息程序

			break;
		}
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


void COperationDlg::OnBnClickedBtnOpeStart()
{
	// TODO: Add your control notification handler code here

	CString stime, stempture;
	float ftempture;
	int itime;
	unsigned char * hData;

	// first send
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_PITEM,stempture);		//取PI heater编辑框中的数据

	ftempture = (float)atof(stempture);	//将tempture编辑框的字符串转成浮点型变量
	if ((ftempture > 120) | (ftempture < 10))
		MessageBox("please fill the number between 40 and 100");
	else
	{
		unsigned char * hData = (unsigned char *)&ftempture;	//将浮点数据转化为十六进制数据

		OperBuf[0] = hData[0];
		OperBuf[1] = hData[1];
		OperBuf[2] = hData[2];
		OperBuf[3] = hData[3];

		TxData[0] = 0xaa;		//preamble code
		TxData[1] = 0x10;		//command
		TxData[2] = 0x0C;		//data length
		TxData[3] = 0x01;		//data type, date edit first byte
		TxData[4] = 0x01;		//real data
		TxData[5] = OperBuf[0];	//tp第一字节				
		TxData[6] = OperBuf[1];	
		TxData[7] = OperBuf[2];
		TxData[8] = OperBuf[3];	//tp最后一字节
		TxData[9] = 0x00;		//time低字节
		TxData[10] = 0x00;		//time高字节
		TxData[11] = 0x00;		//预留位
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

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// second send
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_DENINGTEM,stempture);		//取Dennature编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_DENINGTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[4] = itime>>8;
	OperBuf[5] = itime;

	GetDlgItemText(IDC_EDIT_OPE_ANNTEM,stempture);		//取Anneal编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_ANNTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[10] = itime>>8;
	OperBuf[11] = itime;

	GetDlgItemText(IDC_EDIT_OPE_EXTTEM,stempture);		//取Inter Extension编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_EXTTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[12] = hData[0];
	OperBuf[13] = hData[1];
	OperBuf[14] = hData[2];
	OperBuf[15] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[16] = itime>>8;
	OperBuf[17] = itime;

	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,stime);
	itime = atoi(stime);
	OperBuf[18] = itime;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x16;		//data length
	TxData[3] = 0x03;		//data type, date edit first byte TXC
	//TxData[4] = 0x02;		//real data
	TxData[4] = 0x03;		//RegBuf[18];						
	TxData[5] = 0x01;        //RegBuf[20];	
	TxData[6] = 0x03;       // RegBuf[21];
	TxData[7] = OperBuf[0];		//dennature数据
	TxData[8] = OperBuf[1];	
	TxData[9] = OperBuf[2];	
	TxData[10] = OperBuf[3];		
	TxData[11] = OperBuf[4];
	TxData[12] = OperBuf[5];
	TxData[13] = OperBuf[6];		//Anneal数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	TxData[19] = OperBuf[12];	//Inter extension数据
	TxData[20] = OperBuf[13];	
	TxData[21] = OperBuf[14];	
	TxData[22] = OperBuf[15];	
	TxData[23] = OperBuf[16];	
	TxData[24] = OperBuf[17];
	for (int i=1; i<25; i++)
		TxData[25] += TxData[i];
	if (TxData[25]==0x17)
		TxData[25]=0x18;
	else
		TxData[25]=TxData[25];
	TxData[26]=0x17;
	TxData[27]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// third send
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_DENTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_DENTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[4] = itime>>8;
	OperBuf[5] = itime;

	GetDlgItemText(IDC_EDIT_OPE_HOLDTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_HOLDTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[10] = itime>>8;
	OperBuf[11] = itime;

	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,stime);
	itime = atoi(stime);
	OperBuf[12] = itime;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x10;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte TXC
	TxData[4] = 0x01;		//real data, close
	TxData[5] = OperBuf[12];	//cycle setting
	TxData[6] = 0x00;       //
	TxData[7] = OperBuf[0];	//inital dennature数据	
	TxData[8] = OperBuf[1];  //
	TxData[9] =	OperBuf[2];		
	TxData[10] = OperBuf[3];	
	TxData[11] = OperBuf[4];	
	TxData[12] = OperBuf[5];		
	TxData[13] = OperBuf[6];	//extern extension数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	//TxData[18] = RegBuf[1];
	//TxData[18] = 0x01;
	for (int i=1; i<19; i++)
		TxData[19] += TxData[i];
	if (TxData[19]==0x17)
		TxData[19]=0x18;
	else
		TxData[19]=TxData[19];
	TxData[20]=0x17;
	TxData[21]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

}


void COperationDlg::OnBnClickedBtnOpeStop()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = READSTATUS;
	
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x00;		//data type
	TxData[4] = 0x02;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// second send
	OperDlgFlag = READSTATUS;

	CString stime, stempture;
	float ftempture;
	int itime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_DENTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_DENTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[4] = itime>>8;
	OperBuf[5] = itime;

	GetDlgItemText(IDC_EDIT_OPE_HOLDTEM,stempture);		//取Initail denaturation编辑框中的数据
	GetDlgItemText(IDC_EDIT_OPE_HOLDTIME,stime);

	ftempture = (float)atof(stempture);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&ftempture;
	OperBuf[6] = hData[0];
	OperBuf[7] = hData[1];
	OperBuf[8] = hData[2];
	OperBuf[9] = hData[3];

	itime = atoi(stime);	//将编辑框中整型字符串转成byte
	OperBuf[10] = itime>>8;
	OperBuf[11] = itime;

	GetDlgItemText(IDC_EDIT_OPE_ANNCYCLE,stime);
	itime = atoi(stime);
	OperBuf[12] = itime;		//cycle编辑框值

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x10;		//data length
	TxData[3] = 0x04;		//data type, date edit first byte TXC
	TxData[4] = 0x00;		//real data, start
	TxData[5] = OperBuf[12];	//cycle setting
	TxData[6] = 0x00;       //
	TxData[7] = OperBuf[0];	//inital dennature数据	
	TxData[8] = OperBuf[1];  //
	TxData[9] =	OperBuf[2];		
	TxData[10] = OperBuf[3];	
	TxData[11] = OperBuf[4];	
	TxData[12] = OperBuf[5];		
	TxData[13] = OperBuf[6];	//extern extension数据
	TxData[14] = OperBuf[7];
	TxData[15] = OperBuf[8];
	TxData[16] = OperBuf[9];
	TxData[17] = OperBuf[10];
	TxData[18] = OperBuf[11];
	//TxData[18] = RegBuf[1];
	//TxData[18] = 0x01;
	for (int i=1; i<19; i++)
		TxData[19] += TxData[i];
	if (TxData[19]==0x17)
		TxData[19]=0x18;
	else
		TxData[19]=TxData[19];
	TxData[20]=0x17;
	TxData[21]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


HBRUSH COperationDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
/*
	// set peliter & pi read edit back_color
	if (nCtlColor==CTLCOLOR_EDIT && (pWnd->GetDlgCtrlID()== IDC_EDIT_OPE_PEL | pWnd->GetDlgCtrlID()== IDC_EDIT_OPE_PIREAD) )
	{
		pDC -> SetTextColor(RGB(255,0,0));
		pDC -> SetBkMode(TRANSPARENT);
		hbr = m_editBrush;
	}
*/
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void COperationDlg::OnBnClickedBtnOpeLedset()
{
	// TODO: Add your control notification handler code here

	// send led setup time command
	OperDlgFlag = READSTATUS;

	CString stime;
	float fstime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_LEDSETTIME,stime);		//取LED setup time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x21;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;
	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send led hold time command
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_LEDHOLDTIME,stime);		//取LED hold time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x22;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序


	
}


void COperationDlg::OnBnClickedBtnOpeLedread()
{
	// TODO: Add your control notification handler code here
	
	// read led setup time
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x21;		//data type, read setup time
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// read led hold time
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x22;		//data type, read hold time
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeFanon()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x03;		//data type, fan control
	TxData[4] = 0x03;		//real data, start to auto fan	
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeFanoff()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command 
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x03;		//data type, fan control
	TxData[4] = 0x02;		//real data, close to auto fan		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLedon()
{
	// TODO: Add your control notification handler code here
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
	TxData[4] = 0x01;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeLenoff()
{
	// TODO: Add your control notification handler code here

	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x01;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x23;		//data type, led control
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKpset()
{
	// TODO: Add your control notification handler code here
	
	CString sData;
	float fData;
	unsigned char * hData;
	
	// send zone 0 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KP0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x01;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KP1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x11;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKdset()
{
	// TODO: Add your control notification handler code here

	CString sData;
	float fData;
	unsigned char * hData;

	// send zone 0 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KD0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x04;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KD1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x14;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKiset()
{
	// TODO: Add your control notification handler code here

	CString sData;
	float fData;
	unsigned char * hData;

	// send zone 0 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KI0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x02;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KI1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x12;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeKlset()
{
	// TODO: Add your control notification handler code here
	CString sData;
	float fData;
	unsigned char * hData;

	// send zone 0 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KL0,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x08;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send zone 1 data
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_KL1,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x18;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpePidread()
{
	// TODO: Add your control notification handler code here

	// send zone 1 data
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x12;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x01;		//data type
	TxData[4] = 0x01;		
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpePidzoneset()
{
	// TODO: Add your control notification handler code here

	CString sData;
	float fData;
	unsigned char * hData;

	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_PIDZONE,sData);		//取KP zone0 编辑框中的数据

	fData = (float)atof(sData);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fData;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x11;		//command  
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x09;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}





void COperationDlg::OnBnClickedBtnOpeFanclosetemp()
{
	// TODO: Add your control notification handler code here
	// send led setup time command
	OperDlgFlag = READSTATUS;

	CString stime;
	float fstime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_FANCLOSETEMP,stime);		//取LED setup time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x04;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;
	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeFanread()
{
	// TODO: Add your control notification handler code here

	// send led setup time command
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x05;		//data type
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeOversend()
{
	// TODO: Add your control notification handler code here

	// send overshoot setup time command
	OperDlgFlag = READSTATUS;

	CString stime;
	float fstime;
	unsigned char * hData;

	GetDlgItemText(IDC_EDIT_OPE_OVERTIME,stime);		//取overshoot set time编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x13;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x05;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;
	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// send overshoot temp command
	OperDlgFlag = READSTATUS;

	GetDlgItemText(IDC_EDIT_OPE_OVERTEMP,stime);		//取overshoot temp编辑框中的数据

	fstime = (float)atof(stime);		//将编辑框中的浮点字符串转成十六进制
	hData = (unsigned char *)&fstime;
	OperBuf[0] = hData[0];
	OperBuf[1] = hData[1];
	OperBuf[2] = hData[2];
	OperBuf[3] = hData[3];

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x10;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x06;		//data type
	TxData[4] = OperBuf[0];		
	TxData[5] = OperBuf[1];	
	TxData[6] = OperBuf[2];       
	TxData[7] = OperBuf[3];	
	for (int i=1; i<8; i++)
		TxData[8] += TxData[i];
	if (TxData[8]==0x17)
		TxData[8]=0x18;
	else
		TxData[8]=TxData[8];
	TxData[9]=0x17;
	TxData[10]=0x17;

	//Send message to main dialog
	OperCalMainMsg();	//调用主对话框处理消息程序
}


void COperationDlg::OnBnClickedBtnOpeOverread()
{
	// TODO: Add your control notification handler code here

	// read overshoot time
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x14;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x05;		//data type, read setup time
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
	OperCalMainMsg();	//调用主对话框处理消息程序

	Sleep(100);

	// read overshoot temp
	OperDlgFlag = READSTATUS;

	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x12;		//command  TXC
	TxData[2] = 0x01;		//data length
	TxData[3] = 0x06;		//data type, read hold time
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
	OperCalMainMsg();	//调用主对话框处理消息程序
}

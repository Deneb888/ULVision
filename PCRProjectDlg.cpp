// PCRProjectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "PCRProjectDlg.h"
#include "StartDlg.h"
#include "afxdialogex.h"

using namespace std;

//....................................................................
#include <wtypes.h>
#include <initguid.h>

#define MAX_LOADSTRING 256

extern "C" {

	// This file is in the Windows DDK available from Microsoft.
#include "hidsdi.h"

#include <setupapi.h>
#include <dbt.h>
}
//....................................................................

#ifdef _DEBUG
#define new DEBUG_NEW
//....................................................................
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//....................................................................
#endif
//....................................................................

//....................................................................

//....................................................................
//function prototypes

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

UINT ReadReportThread(LPVOID pParam);	// �������߳�

//....................................................................

//....................................................................
//Application global variables 
DWORD								ActualBytesRead;
DWORD								BytesRead;
HIDP_CAPS							Capabilities;
DWORD								cbBytesRead;
PSP_DEVICE_INTERFACE_DETAIL_DATA	detailData;
HANDLE								DeviceHandle;
DWORD								dwError;
char								FeatureReport[256];
HANDLE								hEventObject;
HANDLE								hDevInfo;
GUID								HidGuid;
OVERLAPPED							HIDOverlapped;
char								InputReport[HIDREPORTNUM];
ULONG								Length;
LPOVERLAPPED						lpOverLap;
bool								MyDeviceDetected = FALSE; 
CString								MyDevicePathName;
DWORD								NumberOfBytesRead;
char								OutputReport[HIDREPORTNUM];
HANDLE								ReadHandle;
DWORD								ReportType;
ULONG								Required;
CString								ValueToDisplay;
HANDLE								WriteHandle;


//These are the vendor and product IDs to look for.
//Uses Lakeview Research's Vendor ID.
int VendorID = 0x0483;
int ProductID = 0x5750;
//....................................................................

//*****************************************************
//Global variable definition
//*****************************************************
BYTE TxData[TxNum];		// the buffer of sent data to COMX
BYTE RxData[RxNum];		// the buffer of received data from COMX
CByteArray array;		// the buffer used to send data to COMX

CString RegRecStr;				//���������ַ���buffer ʮ������
CString Dec_RegRecStr;			//���������ַ���buffer ʮ����
CString Valid_RegRecStr;		//��Ч���������ַ���buffer ʮ������
CString Valid_Dec_RegRecStr;	//��Ч���������ַ���buffer ʮ����

BYTE rCmd;		//��λ����������Ĵ�buffer
BYTE rType;		//��λ������type�Ĵ�buffer

int pnum;		//CommPort number

int mRegFlag;	//���Ի�������Ϣ��־

//���ձ����õ�OVERLAPPED��
OVERLAPPED ReadOverlapped;

//ָ��������̵߳�ָ��
CWinThread * pReadReportThread;

DWORD InputLength = 0;

//*****************************************************
//External variable definition
//*****************************************************
extern int RegFlag;		// register dialog message flag
extern int GraFlag;		// graphic dialog message flag
extern int TrimFlag;	// trim dialog message flag

extern bool Gra_pageFlag;		// graphic dialog ��ҳѭ����־
extern bool Gra_videoFlag;		// graphic dialog videoѭ����־

extern BYTE RegBuf [regdatanum];	// register dialog transmitted data buffer
extern BYTE GraBuf[gradatanum];	// graphic dialog transmitted data buffer

//****************************************************
//Own function prototype 
//****************************************************

unsigned char AsicConvert (unsigned char i, unsigned char j);				//ASIC convert to HEX
int ChangeNum (CString str, int length);									//ʮ�������ַ���תʮ��������
char* EditDataCvtChar (CString strCnv,  char * charRec);	//�༭��ȡֵת�ַ�����

BOOL			g_DeviceDetected = false;

CGraDlg			*g_pGraDlg = 0;
CTrimDlg		*g_pTrimDlg = 0;
CTrimReader		*g_pTrimReader = 0;

CString sGraFirmwareVer;			// graphic HID version string

extern BOOL PollingGraTimerFlag;

extern CString g_ChipID;

BOOL g_EEPROM_Trim_Loaded = false;
BOOL g_Trim_File_Loaded = false;


extern int LAST_PCR;

//*****************************************************
//Own function
//*****************************************************

//ASIC�ַ�תʮ�����ƺ���
unsigned char AsicConvert (unsigned char i, unsigned char j)
{
	switch (i) 
	{	               
	case 0x30:return (j=0x00);break; 
	case 0x31:return (j=0x01);break; 
	case 0x32:return (j=0x02);break; 
	case 0x33:return (j=0x03);break; 
	case 0x34:return (j=0x04);break; 
	case 0x35:return (j=0x05);break; 
	case 0x36:return (j=0x06);break; 
	case 0x37:return (j=0x07);break; 
	case 0x38:return (j=0x08);break; 
	case 0x39:return (j=0x09);break; 
	case 0x41:
	case 0x61:return (j=0x0A);break;
	case 0x42:	
	case 0x62:return (j=0x0B);break; 
	case 0x43:
	case 0x63:return (j=0x0C);break; 
	case 0x44:
	case 0x64:return (j=0x0d);break;
	case 0x45:	
	case 0x65:return (j=0x0e);break; 
	case 0x46:
	case 0x66:return (j=0x0f);break;
	case 0x20:return (' ');break; 
	default: return(j=0x10);break;
	}
}

//�ַ���תʮ���ƺ���
int ChangeNum (CString str, int length)
{
	char  revstr[16]={0};  
	int   num[16]={0};  
	int   count=1;  
	int   result=0;  
	strcpy_s(revstr,str);  
	for   (int i=length-1;i>=0;i--)  
	{  
		if ((revstr[i]>='0') && (revstr[i]<='9'))  
			num[i]=revstr[i]-48;//�ַ�0��ASCIIֵΪ48
		else if ((revstr[i]>='a') && (revstr[i]<='f'))  
			num[i]=revstr[i]-'a'+10;  
		else if ((revstr[i]>='A') && (revstr[i]<='F'))  
			num[i]=revstr[i]-'A'+10;  
		else  
			num[i]=0;
		result=result+num[i]*count;  
		count=count*16;  
	}  
	return result;
}

//�ӱ༭��ȡֵתchar��
 char* EditDataCvtChar (CString strCnv,  char * charRec)
{
	//��RowNum�༭��ȡֵ
	//	CString RNum;
	char * BSNum;
	//	unsigned char * BANum;
	int CNumByte;
	unsigned char k=0;
	int i,j=0;

	CNumByte=strCnv.GetLength();
	BSNum = new char [CNumByte];
	charRec = new char [CNumByte];

	BSNum = (char*)(LPCSTR)strCnv;

	for(i=0;i<(CNumByte/2);i++)
	{
		charRec[i] = (AsicConvert(BSNum[j],k)<<4) | AsicConvert(BSNum[j+1],k);
		j += 2;
	}

	return charRec;
}

//�򴮿ڷ������ݣ����в���Ϊ�������ݵ�byte����
void CPCRProjectDlg:: CommSend(int num)
{
/*	int a;

	array.RemoveAll();
	array.SetSize(num);
	for (a=0;a<num;a++)
	{
		array.SetAt(a,TxData[a]);
	}
	m_mscomm.put_Output(COleVariant(array));	//send data
*/
}

//
void CPCRProjectDlg::SendHIDRead()
{
	WPARAM a = 8;
	LPARAM b = 9;
	HWND hwnd = AfxGetApp()->GetMainWnd()->GetSafeHwnd();
	::SendMessage(hwnd,WM_ReadHID_event,a,b);
}

//�������߳�
UINT ReadReportThread(LPVOID pParam)
{

	return 0;
}

//....................................................................
LRESULT CPCRProjectDlg::Main_OnDeviceChange(WPARAM wParam, LPARAM lParam)  
{

	//DisplayData("Device change detected.");

	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

	switch(wParam) 
	{
		// Find out if a device has been attached or removed.
		// If yes, see if the name matches the device path name of the device we want to access.

	case DBT_DEVICEARRIVAL:
//		DisplayData("A device has been attached.");

//		if (DeviceNameMatch(lParam))
//		{
//			DisplayData("My device has been attached.");
			if (FindTheHID()) {
				if (!g_Trim_File_Loaded) {
					CString mess = "Device trim loaded: ";
					m_TrimDlg.EEPROMRead(0);						// got to find HID first
					m_TrimReader.CopyEepromBuff(0);					// right now only do channel 0. How to do multi-channels?
					m_TrimReader.RestoreTrimBuff(0);

					mess += g_ChipID;

					if (LAST_PCR > 1) {

						m_TrimDlg.EEPROMRead(1);						// got to find HID first
						m_TrimReader.CopyEepromBuff(1);
						m_TrimReader.RestoreTrimBuff(1);

						mess += ' ';
						mess += g_ChipID;
					}

					if (LAST_PCR > 2) {

						m_TrimDlg.EEPROMRead(2);						// got to find HID first
						m_TrimReader.CopyEepromBuff(2);
						m_TrimReader.RestoreTrimBuff(2);

						mess += ' ';
						mess += g_ChipID;
					}

					if (LAST_PCR > 3) {

						m_TrimDlg.EEPROMRead(3);						// got to find HID first
						m_TrimReader.CopyEepromBuff(3);
						m_TrimReader.RestoreTrimBuff(3);

						mess += ' ';
						mess += g_ChipID;
					}

					SetDlgItemText(IDC_STATIC_TRIM, mess);
				}
				m_TrimDlg.ResetTrim();
				GetFWVersion();
			}
//		}

		return TRUE; 

	case DBT_DEVICEREMOVECOMPLETE:
//		DisplayData("A device has been removed.");

//		if (DeviceNameMatch(lParam))
//		{
//			DisplayData("My device has been removed.");

			// Look for the device on the next transfer attempt.

			if (!FindTheHID()) {
				MyDeviceDetected = false;
			}
//		}
		return TRUE; 

	default:
		return TRUE; 
	} 
}

BOOL CPCRProjectDlg::DeviceNameMatch(LPARAM lParam)
{

	// Compare the device path name of a device recently attached or removed 
	// with the device path name of the device we want to communicate with.

	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;

//	DisplayData("MyDevicePathName = " + MyDevicePathName);

	if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) 
	{

		PDEV_BROADCAST_DEVICEINTERFACE lpdbi = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;


		CString DeviceNameString;

		//The dbch_devicetype parameter indicates that the event applies to a device interface.
		//So the structure in LParam is actually a DEV_BROADCAST_INTERFACE structure, 
		//which begins with a DEV_BROADCAST_HDR.

		//The dbcc_name parameter of DevBroadcastDeviceInterface contains the device name. 

		//Compare the name of the newly attached device with the name of the device 
		//the application is accessing (myDevicePathName).

		DeviceNameString = lpdbi->dbcc_name;

//		DisplayData("DeviceNameString = " + DeviceNameString);


		if ((DeviceNameString.CompareNoCase(MyDevicePathName)) == 0)

		{
			//The name matches.

			return true;
		}
		else
		{
			//It's a different device.

			return false;
		}

	}
	else
	{
		return false;
	}	
}

bool CPCRProjectDlg::FindTheHID()
{
	//Use a series of API calls to find a HID with a specified Vendor IF and Product ID.

	HIDD_ATTRIBUTES						Attributes;
	DWORD								DeviceUsage;
	SP_DEVICE_INTERFACE_DATA			devInfoData;
	bool								LastDevice = FALSE;
	int									MemberIndex = 0;
	LONG								Result;	
	CString								UsageDescription;

	Length = 0;
	detailData = NULL;
	DeviceHandle=NULL;

	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/

	HidD_GetHidGuid(&HidGuid);	
	
	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	
	hDevInfo=SetupDiGetClassDevs 
		(&HidGuid, 
		NULL, 
		NULL, 
		DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
		
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want. 
	//Quit on detecting the desired device or checking all available devices without success.

	MemberIndex = 0;
	LastDevice = FALSE;

	do
	{
		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/

		Result=SetupDiEnumDeviceInterfaces 
			(hDevInfo, 
			0, 
			&HidGuid, 
			MemberIndex, 
			&devInfoData);

		if (Result != 0)
		{
			//A device has been detected, so get more information about it.

			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.
			
			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.			
			If retrieving the structure, set 
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/
			
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				NULL, 
				0, 
				&Length, 
				NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			
			//Set cbSize in the detailData structure.

			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.

			Result = SetupDiGetDeviceInterfaceDetail 
				(hDevInfo, 
				&devInfoData, 
				detailData, 
				Length, 
				&Required, 
				NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.

			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/

			DeviceHandle=CreateFile 
				(detailData->DevicePath, 
				0, 
				FILE_SHARE_READ|FILE_SHARE_WRITE, 
				(LPSECURITY_ATTRIBUTES)NULL,
				OPEN_EXISTING, 
				0, 
				NULL);

			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//			DisplayLastError("CreateFile: ");

			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.

			Attributes.Size = sizeof(Attributes);

			Result = HidD_GetAttributes 
				(DeviceHandle, 
				&Attributes);
			
//			DisplayLastError("HidD_GetAttributes: ");
			
			//Is it the desired device?

			MyDeviceDetected = FALSE;
			

			if (Attributes.VendorID == VendorID)
			{
				if (Attributes.ProductID == ProductID)
				{
					//Both the Vendor ID and Product ID match.

					MyDeviceDetected = TRUE;
					MyDevicePathName = detailData->DevicePath;
//					DisplayData("Device detected");

					//Register to receive device notifications.

					RegisterForDeviceNotifications();

					//Get the device's capablities.

					GetDeviceCapabilities();

					// Find out if the device is a system mouse or keyboard.
					
					DeviceUsage = (Capabilities.UsagePage * 256) + Capabilities.Usage;

					if (DeviceUsage == 0x102)
						{
						UsageDescription = "mouse";
						}
				
					if (DeviceUsage == 0x106)
						{
						UsageDescription = "keyboard";
						}

					if ((DeviceUsage == 0x102) | (DeviceUsage == 0x106)) 
						{
//						DisplayData("");
//						DisplayData("*************************");
//						DisplayData("The device is a system " + UsageDescription + ".");
//						DisplayData("Windows 2000 and Windows XP don't allow applications");
//						DisplayData("to directly request Input reports from or "); 
//						DisplayData("write Output reports to these devices.");
//						DisplayData("*************************");
//						DisplayData("");
						}

					// Get a handle for writing Output reports.

					WriteHandle=CreateFile 
						(detailData->DevicePath, 
						GENERIC_WRITE, 
						FILE_SHARE_READ|FILE_SHARE_WRITE, 
						(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING, 
						0, 
						NULL);

					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					HidD_SetNumInputBuffers(WriteHandle,HIDBUFSIZE);
					//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//					DisplayLastError("CreateFile: ");

					// Prepare to read reports using Overlapped I/O.

					PrepareForOverlappedTransfer();

					// Set HID driver input buffer
//					HidD_SetNumInputBuffers(DeviceHandle,64);

				} //if (Attributes.ProductID == ProductID)

				else
					//The Product ID doesn't match.

					CloseHandle(DeviceHandle);

			} //if (Attributes.VendorID == VendorID)

			else
				//The Vendor ID doesn't match.

				CloseHandle(DeviceHandle);

		//Free the memory used by the detailData structure (no longer needed).

		free(detailData);

		}  //if (Result != 0)

		else
			//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.

			LastDevice=TRUE;

		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.

		MemberIndex = MemberIndex + 1;

	} //do

	while ((LastDevice == FALSE) && (MyDeviceDetected == FALSE));

	if (MyDeviceDetected == FALSE) {
//		DisplayData("Device not detected");
		SetDlgItemText(IDC_STATICOpenComm,"Device Not Detected");
		g_DeviceDetected = false;
	}
	else {
//		DisplayData("Device detected");
		SetDlgItemText(IDC_STATICOpenComm,"ULS24 Device Detected");
		g_DeviceDetected = true;
	}

	//Free the memory reserved for hDevInfo by SetupDiClassDevs.

	SetupDiDestroyDeviceInfoList(hDevInfo);
//	DisplayLastError("SetupDiDestroyDeviceInfoList");

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	HidD_SetNumInputBuffers(DeviceHandle,HIDBUFSIZE);
	HidD_SetNumInputBuffers(WriteHandle,HIDBUFSIZE);
	HidD_SetNumInputBuffers(ReadHandle,HIDBUFSIZE);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	return MyDeviceDetected;
}

void CPCRProjectDlg::CloseHandles()
{
	//Close open handles.

	if (DeviceHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(DeviceHandle);
	}

	if (ReadHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(ReadHandle);
	}

	if (WriteHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(WriteHandle);
	}
}

void CPCRProjectDlg::DisplayInputReport()
{
	USHORT	ByteNumber;
	CHAR	ReceivedByte;

	//Display the received data in the log and the Bytes Received List boxes.
	//Start at the top of the List Box.

	m_BytesReceived.ResetContent();

	//Step through the received bytes and display each.

	for (ByteNumber=0; ByteNumber < Capabilities.InputReportByteLength; ByteNumber++)
	{
		//Get a byte.

		ReceivedByte = InputReport[ByteNumber];

		//Display it.

		DisplayReceivedData(ReceivedByte);
	}
}

void CPCRProjectDlg::DisplayReceivedData(char ReceivedByte)
{
	//Display data received from the device.

	CString	strByteRead;

	//Convert the value to a 2-character Cstring.

	strByteRead.Format("%02X", ReceivedByte);
	strByteRead = strByteRead.Right(2); 

	//Display the value in the Bytes Received List Box.

	m_BytesReceived.InsertString(-1, strByteRead);

	//Display the value in the log List Box (optional).
	//MessageToDisplay.Format("%s%s", "Byte 0: ", strByteRead); 
	//DisplayData(MessageToDisplay);	
	//UpdateData(false);
}

void CPCRProjectDlg::GetDeviceCapabilities()
{
	//Get the Capabilities structure for the device.

	PHIDP_PREPARSED_DATA	PreparsedData;

	/*
	API function: HidD_GetPreparsedData
	Returns: a pointer to a buffer containing the information about the device's capabilities.
	Requires: A handle returned by CreateFile.
	There's no need to access the buffer directly,
	but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	HidD_GetPreparsedData 
		(DeviceHandle, 
		&PreparsedData);
//	DisplayLastError("HidD_GetPreparsedData: ");

	/*
	API function: HidP_GetCaps
	Learn the device's capabilities.
	For standard devices such as joysticks, you can find out the specific
	capabilities of the device.
	For a custom device, the software will probably know what the device is capable of,
	and the call only verifies the information.
	Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
	Returns: a Capabilities structure containing the information.
	*/
	
	HidP_GetCaps 
		(PreparsedData, 
		&Capabilities);
//	DisplayLastError("HidP_GetCaps: ");

	//Display the capabilities

	ValueToDisplay.Format("%s%X", "Usage Page: ", Capabilities.UsagePage);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Input Report Byte Length: ", Capabilities.InputReportByteLength);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Output Report Byte Length: ", Capabilities.OutputReportByteLength);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Feature Report Byte Length: ", Capabilities.FeatureReportByteLength);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Link Collection Nodes: ", Capabilities.NumberLinkCollectionNodes);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Input Button Caps: ", Capabilities.NumberInputButtonCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of InputValue Caps: ", Capabilities.NumberInputValueCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of InputData Indices: ", Capabilities.NumberInputDataIndices);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Output Button Caps: ", Capabilities.NumberOutputButtonCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Output Value Caps: ", Capabilities.NumberOutputValueCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Output Data Indices: ", Capabilities.NumberOutputDataIndices);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Feature Button Caps: ", Capabilities.NumberFeatureButtonCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Feature Value Caps: ", Capabilities.NumberFeatureValueCaps);
//	DisplayData(ValueToDisplay);
	ValueToDisplay.Format("%s%d", "Number of Feature Data Indices: ", Capabilities.NumberFeatureDataIndices);
///	DisplayData(ValueToDisplay);

	//No need for PreparsedData any more, so free the memory it's using.

	HidD_FreePreparsedData(PreparsedData);
//	DisplayLastError("HidD_FreePreparsedData: ") ;
}

void CPCRProjectDlg::PrepareForOverlappedTransfer()
{
	//Get a handle to the device for the overlapped ReadFiles.

	ReadHandle=CreateFile 
		(detailData->DevicePath, 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_OVERLAPPED,
		NULL);

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		HidD_SetNumInputBuffers(ReadHandle,HIDBUFSIZE);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//	DisplayLastError("CreateFile (ReadHandle): ");

	//Get an event object for the overlapped structure.

	/*API function: CreateEvent
	Requires:
	  Security attributes or Null
	  Manual reset (true). Use ResetEvent to set the event object's state to non-signaled.
	  Initial state (true = signaled) 
	  Event object name (optional)
	Returns: a handle to the event object
	*/

	if (hEventObject == 0)
	{
		hEventObject = CreateEvent 
			(NULL, 
			TRUE, 
			TRUE, 
			"");
//	DisplayLastError("CreateEvent: ") ;

	//Set the members of the overlapped structure.

	HIDOverlapped.hEvent = hEventObject;
	HIDOverlapped.Offset = 0;
	HIDOverlapped.OffsetHigh = 0;
	}
}

void CPCRProjectDlg::ReadAndWriteToDevice()
{
	//If necessary, find the device and learn its capabilities.
	//Then send a report and request a report.

	//Clear the List Box (optional).
	//m_ResultsList.ResetContent();

//	DisplayData("***HID Test Report***");
//	DisplayCurrentTime();

	//If the device hasn't been detected already, look for it.

	if (MyDeviceDetected==FALSE)
	{
		MyDeviceDetected=FindTheHID();
	}

	// Do nothing if the device isn't detected.

	if (MyDeviceDetected==TRUE)
	{
		//Write a report to the device.

		WriteHIDOutputReport();

		//Active read message

		SendHIDRead();
	} 
}

#define F1_DETECT

void CPCRProjectDlg::ReadHIDInputReport()
{

	// Retrieve an Input report from the device.

	DWORD	Result;
	
	//The first byte is the report number.
	InputReport[0]=0;

 	
	/*API call:ReadFile
	'Returns: the report in InputReport.
	'Requires: a device handle returned by CreateFile
	'(for overlapped I/O, CreateFile must be called with FILE_FLAG_OVERLAPPED),
	'the Input report length in bytes returned by HidP_GetCaps,
	'and an overlapped structure whose hEvent member is set to an event object.
	*/

	if (ReadHandle != INVALID_HANDLE_VALUE)
		{
		Result = ReadFile 
		(ReadHandle, 
		InputReport, 
		Capabilities.InputReportByteLength, 
		&NumberOfBytesRead,
		(LPOVERLAPPED) &HIDOverlapped
		); 
		}

//	DisplayLastError("ReadFile: ") ;

	/*API call:WaitForSingleObject
	'Used with overlapped ReadFile.
	'Returns when ReadFile has received the requested amount of data or on timeout.
	'Requires an event object created with CreateEvent
	'and a timeout value in milliseconds.
	*/

	Result = WaitForSingleObject 
		(hEventObject, 
		VEDIOPITCHTIME);


	CString strtest;		//���յ���λ�����صĴ��������buffer
	strtest.Empty();
	CString strtemp;		//��ʱ�����ַ���buffer,�ݴ�һ��byte���ݣ�ʮ������
	strtemp.Empty();
	CString DStrtemp;		//��ʱ�����ַ���buffer,�ݴ�һ��byte���ݣ�ʮ����
	DStrtemp.Empty();
	CString Valid_strtemp;	//��Ч��ʱ�����ַ���buffer,�ݴ�һ��byte���ݣ�ʮ������
	Valid_strtemp.Empty();
	CString Valid_DStrtemp;	//��Ч��ʱ�����ַ���buffer,�ݴ�һ��byte���ݣ�ʮ����
	Valid_DStrtemp.Empty();
	long k = 0;

	InputLength = Capabilities.InputReportByteLength;
	if (InputLength>65)
		AfxMessageBox("The length of received data is over 65");

	switch (Result)
	{
	case WAIT_OBJECT_0:
		{
				//����ת���ַ���buffer����
				RegRecStr.Empty();	// Clear֮ǰ����ʾ����
				Dec_RegRecStr.Empty();	// Clear֮ǰ����ʾ����
				Valid_RegRecStr.Empty();
				Valid_Dec_RegRecStr.Empty();

				//������յ�����
				for (k=0;k<HIDREPORTNUM-1;k++)
					RxData[k] = InputReport[k+1];	//�����յ����ݰ��ֽڷ��䵽�ֽڴ洢buffer(RxData[200])
				//ȡ�����ص������type
				rCmd = RxData[2];	//ȡ����λ�����ص�����
				rType = RxData[4];	//ȡ����λ�����ص�type
				strtest.Format("%2x",rCmd);

				//�����ص���������ɸ�������ַ���
				//ȡ��������
				for(k=0;k<HIDREPORTNUM;k++)			 //���ַ�����ĸ��ַ�ת���ַ���
				{
					strtemp.Format("%02X ",RxData[k]);			//ÿ���ַ�ת����ʮ��������ʾ,���С�0��Ҳ����ʾ�硰02��
					//����Ҫ��ʾ��0����strtemp.Format("%x",bt)
					DStrtemp.Format("%02d",RxData[k]);			//ʮ������ʾ����������

					RegRecStr+=strtemp;					//��ÿ��ʮ�������ַ�ת�ɵ��ַ�����ϳ�һ���ַ���
					//ʮ������ת�������ڸ�byte���Դ��ո�
					//���յ�ÿ���������ݶ����ڸ��ַ��������ǶԸ��ַ������㣬����֮ǰ�������ݱ���

					Dec_RegRecStr += (DStrtemp+" ");	//��ÿ��ʮ�����ַ�ת�ɵ��ַ�����ϳ�һ���ַ���
					//ʮ����ת�����������ַ����Դ��ո����������
					//���յ�ÿ���������ݶ����ڸ��ַ��������ǶԸ��ַ������㣬����֮ǰ�������ݱ���
				}
				//ÿ�����ݼ����س�
				//		RegRecStr += "\r\n";
				//		Dec_RegRecStr += "\r\n";

				//������λ�����ص����������Ե���Ϣ�������
				switch(rCmd)
				{
				case GraCmd:
					{
						PCRTypeFilterClass = rType & 0x0F;		// �жϷ��ص�����ǻ��С�ҳ����vedio
						PCRTypeFilterNum = rType & 0xF0;		// �ж��ǵڼ���PCR���ص�����

//=================F1 Code detection
#ifdef F1_DETECT
						if(RxData[5]==0xf1)
						{
							MessageBox("Error code 0xF1.  Sensor communication time out.", "ULVision", MB_ICONWARNING);
							return ;
						}
#endif
//==================	

						// Support for Remap

						if (rType == 0x09) {
							PCRNum = RxData[5] + 1;		// PCRNum is the channel number + 1, as it is 1 based.

							if (RxData[6] == 11)		// HID������12�к�ֹͣ��ȡ
								Gra_pageFlag = false;
							else
								Gra_pageFlag = true;

							for (k = 0; k<26; k++)
							{
								Valid_strtemp.Format("%02X", RxData[k + 7]);	//ʮ��������ʾ�ַ���buffer, ÿ��byte��ӿո�
								Valid_DStrtemp.Format("%02d", RxData[k + 7]);	//ʮ������ʾ�ַ���buffer, ÿ��byte��ӿո�

								Valid_RegRecStr += (Valid_strtemp + " ");
								Valid_Dec_RegRecStr += (Valid_DStrtemp + " ");
							}
						}
						else if (rType == 0x0A) {
							PCRNum = RxData[5] + 1;		// PCRNum is the channel number + 1, as it is 1 based.

							if (RxData[6] == 23)		// HID������12�к�ֹͣ��ȡ
								Gra_pageFlag = false;
							else
								Gra_pageFlag = true;

							for (k = 0; k<50; k++)
							{
								Valid_strtemp.Format("%02X", RxData[k + 7]);	//ʮ��������ʾ�ַ���buffer, ÿ��byte��ӿո�
								Valid_DStrtemp.Format("%02d", RxData[k + 7]);	//ʮ������ʾ�ַ���buffer, ÿ��byte��ӿո�

								Valid_RegRecStr += (Valid_strtemp + " ");
								Valid_Dec_RegRecStr += (Valid_DStrtemp + " ");
							}
						}
						// End Remap support code
						//ȡ��Ӧ����Ч����
						else if ((PCRTypeFilterClass==0x01)|(PCRTypeFilterClass==0x02)|(PCRTypeFilterClass==0x03) )		//����12 pixel ����ʱ
						{	
							// �������
							switch(PCRTypeFilterNum)
							{
							case 0x00:
								PCRNum = 1;
								break;
							case 0x10:
								PCRNum = 2;
								break;
							case 0x20:
								PCRNum = 3;
								break;
							case 0x30:
								PCRNum = 4;
								break;
							default:
								break;
							}
							
							if (RxData[5]==0x0b)		// HID������12�к�ֹͣ��ȡ
								Gra_pageFlag = false;
							else
								Gra_pageFlag = true;

							for(k=0; k<26; k++)
							{
								Valid_strtemp.Format("%02X",RxData[k+6]);	//ʮ��������ʾ�ַ���buffer, ÿ��byte��ӿո�
								Valid_DStrtemp.Format("%02d",RxData[k+6]);	//ʮ������ʾ�ַ���buffer, ÿ��byte��ӿո�

								Valid_RegRecStr += (Valid_strtemp+" ");
								Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
							}
							//		ÿ�����ݼ����س�
							//		Valid_RegRecStr += "\r\n";
							//		Valid_Dec_RegRecStr += "\r\n";
						}
						else
						{
							if ((PCRTypeFilterClass==0x07)|(PCRTypeFilterClass==0x08)|(PCRTypeFilterClass==0x0b))	//����24 pixel ����ʱ
							{
								// �������
								switch(PCRTypeFilterNum)
								{
								case 0x00:
									PCRNum = 1;
									break;
								case 0x10:
									PCRNum = 2;
									break;
								case 0x20:
									PCRNum = 3;
									break;
								case 0x30:
									PCRNum = 4;
									break;
								default:
									break;
								}
								
								if (RxData[5]==0x17)	// HID������24�к�ֹͣ��ȡ
									Gra_pageFlag = false;
								else
									Gra_pageFlag = true;

								for(k=0; k<50; k++)
								{
									Valid_strtemp.Format("%02X",RxData[k+6]);	//ʮ��������ʾ�ַ��� buffer, ÿ�� byte ��ӿո�
									Valid_DStrtemp.Format("%02d",RxData[k+6]);	//ʮ������ʾ�ַ��� buffer, ÿ�� byte ��ӿո�

									Valid_RegRecStr += (Valid_strtemp+" ");
									Valid_Dec_RegRecStr += (Valid_DStrtemp+" ");
								}
								//ÿ�����ݼ����س�
								//						Valid_RegRecStr += "\r\n";
								//						Valid_Dec_RegRecStr += "\r\n";
							}
							else	//����type����
							{
								Valid_RegRecStr.Empty();
								Valid_Dec_RegRecStr.Empty();
							}
						}

						//����Graphic dialog��Ϣ�������
						if(PCRTypeFilterNum != 0xf0) 
							m_GraDlg.SendMessage(UM_GRAPROCESS);		// Pre-read terminates here		
						else
							Gra_pageFlag = false;

						break;
			}
				case ReadCmd:
					{
//						if ((rType == 0x21) | (rType == 0x22))
//							m_OperDlg.SendMessage(UM_OPERLEDPROCESS);
						if ((rType == 0x11) | (rType == 0x2d) | (rType == 0x12) | (rType == 0x0e))
							m_TrimDlg.SendMessage(UM_TRIMPROCESS);
						else if (rType == 0x27)
						{
							sGraFirmwareVer.Empty();

							int iGraFirmwareVer1, iGraFirmwareVer2;
							int iYear,iMonth,iDay;
							int iFuncCode;

							iFuncCode = RxData[5];
							//iGraFirmwareVer = (RxData[6]<<8) | RxData[7];
							iGraFirmwareVer1 = RxData[6];
							iGraFirmwareVer2 = RxData[7];
							iYear = RxData[8];
							iMonth = RxData[9];
							iDay =RxData[10];

							sGraFirmwareVer.Format("FW Version: %2d.%.2d, FuncCode: %2d, ( Release date: %.2d-%.2d-20%.2d )",iGraFirmwareVer1,iGraFirmwareVer2,iFuncCode,iMonth,iDay, iYear);
						}
						break;
					}
				case 0x15: // POLLINGRACMD:
					{
						if (rType == 0x01)
						{
							BYTE rData = 0;
							rData = RxData[5] & 0x03;	// 0x0F; // only consider first 2 channel
							if (rData)		// polling����Ч��������
							{
								PollingGraTimerFlag = FALSE;	// ��ͣgraphic HID polling

								// ��ʾgraphic dialog 
								// m_GraDlg.ShowWindow(SW_SHOW);	
								CRect tabRect;    // ��ǩ�ؼ��ͻ�����Rect     
								// ��ȡ��ǩ�ؼ��ͻ���Rect����������������ʺϷ��ñ�ǩҳ     
								m_tab.GetClientRect(&tabRect);     
								tabRect.left += 1;     
								tabRect.right -= 1;     
								tabRect.top += 25;     
								tabRect.bottom -= 1; 

								m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
								m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								// m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
								
								// ����ҳͼ����ʾ����
								/*m_GraDlg.CaptureFrame();*/	
								
								m_GraDlg.OnBnClickedButtonCapture();	

								PollingGraTimerFlag = TRUE;	// ��ͣgraphic HID polling

							}
						}
						break;
					}
				default:
					{
						//				AfxMessageBox(strtest);
						break;
					}
				}
					
		break;
		}
	case WAIT_TIMEOUT:
		{
		SetDlgItemText(IDC_STATICOpenComm, "ReadFile timeout");

		/*API call: CancelIo
		Cancels the ReadFile
        Requires the device handle.
        Returns non-zero on success.
		*/
		
		Result = CancelIo(ReadHandle);
		
		//A timeout may mean that the device has been removed. 
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.
		CloseHandles();
		MyDeviceDetected = FALSE;
		break;
		}
	default:
		{
		//Close the device handles and set MyDeviceDetected = False 
		//so the next access attempt will search for the device.

		CloseHandles();
		SetDlgItemText(IDC_STATICOpenComm,"Can't read from device");
		MyDeviceDetected = FALSE;
		break;
		}
	}

	/*
	API call: ResetEvent
	Sets the event object to non-signaled.
	Requires a handle to the event object.
	Returns non-zero on success.
	*/

	ResetEvent(hEventObject);

	//Display the report data.

	DisplayInputReport();

}

void CPCRProjectDlg::RegisterForDeviceNotifications()
{

	// Request to receive messages when a device is attached or removed.
	// Also see WM_DEVICECHANGE in BEGIN_MESSAGE_MAP(CPCRProjectDlg, CDialog).

	DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
	HDEVNOTIFY DeviceNotificationHandle;

	DevBroadcastDeviceInterface.dbcc_size = sizeof(DevBroadcastDeviceInterface);
	DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	DevBroadcastDeviceInterface.dbcc_classguid = HidGuid;

	DeviceNotificationHandle =
		RegisterDeviceNotification(m_hWnd, &DevBroadcastDeviceInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

}

void CPCRProjectDlg::WriteHIDOutputReport()
{
	//Send a report to the device.

	DWORD	BytesWritten = 0;
	INT		Index =0;
	ULONG	Result;
	CString	strBytesWritten = "";

	UpdateData(true);

	//The first byte is the report number.

	OutputReport[0]=0;

	for (int i=1; i<TxNum+1; i++)
	 OutputReport[i] = TxData[i-1];

	/*
		API Function: WriteFile
		Sends a report to the device.
		Returns: success or failure.
		Requires:
		A device handle returned by CreateFile.
		A buffer that holds the report.
		The Output Report length returned by HidP_GetCaps,
		A variable to hold the number of bytes written.
	*/

		if (WriteHandle != INVALID_HANDLE_VALUE)
			{
			Result = WriteFile 
			(WriteHandle, 
			OutputReport, 
			Capabilities.OutputReportByteLength, 
			&BytesWritten, 
			NULL);
		}

		//Display the result of the API call and the report bytes.

		if (!Result)
			{
			//The WriteFile failed, so close the handles, display a message,
			//and set MyDeviceDetected to FALSE so the next attempt will look for the device.

			CloseHandles();
			SetDlgItemText(IDC_STATICOpenComm,"Can't write to device");
			MyDeviceDetected = FALSE;
			}

}

//....................................................................


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()


// CPCRProjectDlg dialog




CPCRProjectDlg::CPCRProjectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPCRProjectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_StaticOpenComm = _T("");
	//  m_strBytesReceived = _T("");
	m_strBytesReceived = _T("");
}

void CPCRProjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_MSCOMM1, m_mscomm);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Text(pDX, IDC_STATICOpenComm, m_StaticOpenComm);
	//  DDX_Text(pDX, IDC_EDIT_HIDRECEIVE, m_strBytesReceived);
	//  DDX_Control(pDX, IDC_EDIT_HIDRECEIVE, m_BytesReceived);
	DDX_Control(pDX, IDC_lstBytesReceived, m_BytesReceived);
	DDX_LBString(pDX, IDC_lstBytesReceived, m_strBytesReceived);
}

void CPCRProjectDlg::OnOK()
{

}

void CPCRProjectDlg::OnCancel()
{

}

BEGIN_MESSAGE_MAP(CPCRProjectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CPCRProjectDlg::OnTcnSelchangeTab1)
	ON_MESSAGE(WM_RegDlg_event,OnRegDlg)
	ON_MESSAGE(WM_GraDlg_event,OnGraDlg)
	ON_MESSAGE(WM_TrimDlg_event,OnTrimDlg)
	ON_MESSAGE(WM_ReadHID_event,OnReadHID)
	ON_MESSAGE(WM_OperDlg_event,OnOperDlg)
	ON_MESSAGE(WM_ParsaveDlg_event,OnParsaveDlg)
	ON_BN_CLICKED(IDC_BTN_OPENCOMM, &CPCRProjectDlg::OnBnClickedBtnOpencomm)
	ON_BN_CLICKED(IDC_BTN_OPENHID, &CPCRProjectDlg::OnBnClickedBtnOpenhid)
	//....................................................................
	//ON_WM_DEVICECHANGE()
	ON_MESSAGE(WM_DEVICECHANGE, Main_OnDeviceChange)
	//....................................................................
	ON_BN_CLICKED(IDC_BTN_SENDHID, &CPCRProjectDlg::OnBnClickedBtnSendhid)
	ON_BN_CLICKED(IDC_BTN_READHID, &CPCRProjectDlg::OnBnClickedBtnReadhid)
	ON_WM_SHOWWINDOW()
//	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

TCHAR g_CurrentDirectory[MAX_PATH];

// CPCRProjectDlg message handlers

BOOL CPCRProjectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ModifyStyle(WS_MAXIMIZEBOX, 0);

	// TODO: Add extra initialization here



	//Initial TAB component

	CRect tabRect;

	// ��ʾ����Ի���
#if PAGENUM == 2
	//	m_tab.InsertItem(0, _T("Thermal Cycle"));         // �����һ����ǩ��Register��     
	m_tab.InsertItem(0, _T("Image window"));  // ����ڶ�����ǩ��Graphic��  
	m_tab.InsertItem(1, _T("Parameters"));
//	m_tab.InsertItem(2,_T(("Operation")));
	//	m_RegDlg.Create(IDD_REGISTER_DIALOG, &m_tab);	// ������һ����ǩҳ     
	m_GraDlg.Create(IDD_GRAPHIC_DIALOG, &m_tab);	// �����ڶ�����ǩҳ  
	m_TrimDlg.Create(IDD_TRIM_DIALOG, &m_tab);		//������������ǩҳ
//	m_OperDlg.Create(IDD_OPERATION_DIALOG,&m_tab);

	m_tab.GetClientRect(&tabRect);    // ��ȡ��ǩ�ؼ��ͻ���Rect     
	// ����tabRect��ʹ�串�Ƿ�Χ�ʺϷ��ñ�ǩҳ     
	tabRect.left += 1;                    
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1;     
	// ���ݵ����õ�tabRect����m_RegDlg�ӶԻ��򣬲�����Ϊ��ʾ     
	//	m_RegDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);     
	// ���ݵ����õ�tabRect����m_GraDlg�ӶԻ��򣬲�����Ϊ����     
	m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);  // SWP_HIDEWINDOW);
	// ���ݵ����õ�tabRect����m_TrimDlg�ӶԻ��򣬲�����Ϊ����     
	m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
	// ���ݵ����õ�tabRect����m_OperDlg�ӶԻ��򣬲�����Ϊ����     
//	m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
#endif

	// ֻ��ʾoperation dialog
#if PAGENUM == 1
	m_tab.InsertItem(2,_T(("Operation")));
	m_OperDlg.Create(IDD_OPERATION_DIALOG,&m_tab);
	m_tab.GetClientRect(&tabRect);    // ��ȡ��ǩ�ؼ��ͻ���Rect     
	// ����tabRect��ʹ�串�Ƿ�Χ�ʺϷ��ñ�ǩҳ     
	tabRect.left += 1;                    
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1;
	m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
#endif

	g_pGraDlg = &m_GraDlg;
	g_pTrimDlg = &m_TrimDlg;

	m_TrimDlg.Loadconf();
	// auto load default json file regardless of whether find HW.

	//==================================================================
	// Load Trim data

	GetCurrentDirectory(MAX_PATH, g_CurrentDirectory);

	CString path;
	path = g_CurrentDirectory;
	path += "\\Trim\\trim.dat";

	LPTSTR lpszData = path.GetBuffer(path.GetLength());
	int e = m_TrimReader.Load((TCHAR*)lpszData);

	path.ReleaseBuffer(0);
	CString mess = "Device Trim Loaded: ";

	if (e) {
		m_TrimReader.Parse();

		mess += m_TrimReader.Node[0].name + " ";
		mess += m_TrimReader.Node[1].name + " ";
		mess += m_TrimReader.Node[2].name + " ";
		mess += m_TrimReader.Node[3].name;

		SetDlgItemText(IDC_STATIC_TRIM, mess);
		g_Trim_File_Loaded = true;

		LAST_PCR = m_TrimReader.NumNode;
	}
	else {
		SetDlgItemText(IDC_STATIC_TRIM, "Device Trim Data Not Loaded");
		//		MessageBox("Trim file not loaded. Loading trim data from EEPROM");
		g_Trim_File_Loaded = false;
	}


	//===================================================================

	g_pTrimReader = &m_TrimReader;
	m_GraDlg.GreyChanSel();

	if(FindTheHID()) {
		if (!e) {

			//============Message=====================

/*			CStartDlg startDlg;					

			startDlg.m_NumChipsInstalled = LAST_PCR;

			if (startDlg.DoModal() == IDOK)
			{
				LAST_PCR = startDlg.m_NumChipsInstalled;
				m_GraDlg.GreyChanSel();
			}
*/
			//========================================		// Num of chips installed determined by config.json file

			m_TrimDlg.EEPROMRead(0);						// got to find HID first
			m_TrimReader.CopyEepromBuff(0);
			m_TrimReader.RestoreTrimBuff(0);

			mess += g_ChipID;

			if (LAST_PCR > 1) {

				m_TrimDlg.EEPROMRead(1);						// got to find HID first
				m_TrimReader.CopyEepromBuff(1);
				m_TrimReader.RestoreTrimBuff(1);

				mess += ' ';
				mess += g_ChipID;
			}

			if (LAST_PCR > 2) {

				m_TrimDlg.EEPROMRead(2);						// got to find HID first
				m_TrimReader.CopyEepromBuff(2);
				m_TrimReader.RestoreTrimBuff(2);

				mess += ' ';
				mess += g_ChipID;
			}

			if (LAST_PCR > 3) {

				m_TrimDlg.EEPROMRead(3);						// got to find HID first
				m_TrimReader.CopyEepromBuff(3);
				m_TrimReader.RestoreTrimBuff(3);

				mess += ' ';
				mess += g_ChipID;
			}

			SetDlgItemText(IDC_STATIC_TRIM, mess);

			g_EEPROM_Trim_Loaded = true;
		}

		m_TrimDlg.ResetTrim();
		GetFWVersion();
	}
	else {
		RegisterForDeviceNotifications();					// ...so device arrival can be detected
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPCRProjectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPCRProjectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPCRProjectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BEGIN_EVENTSINK_MAP(CPCRProjectDlg, CDialogEx)
	ON_EVENT(CPCRProjectDlg, IDC_MSCOMM1, 1, CPCRProjectDlg::OnCommMscomm1, VTS_NONE)
END_EVENTSINK_MAP()


void CPCRProjectDlg::OnCommMscomm1()
{
}

LRESULT CPCRProjectDlg::OnReadHID(WPARAM wParam, LPARAM lParam)
{
	ReadHIDInputReport();

	return 0;
}

LRESULT CPCRProjectDlg::OnRegDlg(WPARAM wParam, LPARAM lParam)
{
	switch(RegFlag)
	{

	case sendregmsg:
		{
			// ��HID��������
			WriteHIDOutputReport();		

			//�����־������buffer����
			RegFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(RegBuf,0,sizeof(RegBuf));
			
			// ��ȡHID��������
			SendHIDRead();			

			break;
		}
	default:
		AfxMessageBox(_T("Please Send Register Flag Correctly"));
		break;

	}

	return 0;
}

LRESULT CPCRProjectDlg::OnGraDlg(WPARAM wParam, LPARAM lParam)
{
	switch(GraFlag)
	{
	case sendgramsg:
		{
			// ��HID��������
			WriteHIDOutputReport();		

			//�����־������buffer����
			GraFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(GraBuf,0,sizeof(GraBuf));

			// ��ȡHID��������
			SendHIDRead();		// �����ǳ�ʱ�����ʱ������ԭ��
								// ��HIDд����ٶ��ļ��̫��
								// ���Ժ���λ�����ӷ��������ƣ��жϽ��������λ���ٷ���һ������
			break;
		}
	case sendpagemsg:
		{
			WriteHIDOutputReport();		// ��HID��������

			//�����־������buffer����
			GraFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(GraBuf,0,sizeof(GraBuf));

			while(Gra_pageFlag)
				ReadHIDInputReport();
			break;
		}
	case readgramsg:
		{
			GraFlag = 0;
			ReadHIDInputReport();
		}
	case sendvideomsg:
		{
/*			if (Gra_videoFlag)
				SetTimer(1,VEDIOPITCHTIME,NULL);
			else
			{
				KillTimer(1);
				memset(TxData,0,sizeof(TxData));
				memset(GraBuf,0,sizeof(GraBuf));
			}
*/			break;
		}

	default:
		AfxMessageBox(_T("Please Send Graphic Flag Correctly"));
		break;

	}

	return 0;
}

LRESULT CPCRProjectDlg::OnTrimDlg(WPARAM wParam, LPARAM lParam)
{
	switch(TrimFlag)
	{
	case sendtrimmsg:
		{
//			CommSend(dNum);		//�򴮿ڷ�������
			WriteHIDOutputReport();		// ��HID��������

			//�����־������buffer����
			TrimFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(TrimBuf,0,sizeof(TrimBuf));

			SendHIDRead();		// ��ȡHID��������
			break;
		}
	case sendeeprommsg:			// only for eeprom read
	{
		WriteHIDOutputReport();		//

		TrimFlag = 0;
		memset(TxData, 0, sizeof(TxData));
		memset(TrimBuf, 0, sizeof(TrimBuf));

		for (int i = 0; i<NUM_EPKT; i++)			// read 4 packets
			ReadHIDInputReport();

		break;
	}
	default:
		AfxMessageBox(_T("Please Send Trim Flag Correctly"));
		break;

	}

	return 0;
}


LRESULT CPCRProjectDlg::OnOperDlg(WPARAM wParam, LPARAM iParam)
{
	switch(OperDlgFlag)
	{
	case READSTATUS:
		{
			//			CommSend(dNum);		//�򴮿ڷ�������
			WriteHIDOutputReport();		// ��HID��������

			//�����־������buffer����
			OperDlgFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(OperBuf,0,sizeof(OperBuf));
			SendHIDRead();		// ��ȡHID��������
			break;
		}
	case sendopemsg:
		{
			WriteHIDOutputReport();		// ��HID��������

			//�����־������buffer����
			OperDlgFlag = 0;
			memset(TxData,0,sizeof(TxData));
			memset(OperBuf,0,sizeof(OperBuf));
			break;
		}
	default:
		AfxMessageBox(_T("Please Send Graphic Flag Correctly"));
		break;

	}
	return 0;
}


LRESULT CPCRProjectDlg::OnParsaveDlg(WPARAM wParam, LPARAM iParam)
{
	CString sTemp = "builder:";
	if (sFileName != "")
	{
//		sBuilderName += (sTemp + "\r\n");
		sBuilderName += (sTemp + " ");
		sBuilderName += sSave;
		sBuilderName += "\0";
		sFileName += ".txt";
		CFile m_statusFile(sFileName,CFile::modeCreate|CFile::modeWrite);
		m_statusFile.Write(sBuilderName,sBuilderName.GetLength());
	}
	else
		AfxMessageBox("Please fill the filename");

	return 0;
}


void CPCRProjectDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	CRect tabRect;    // ��ǩ�ؼ��ͻ�����Rect     

	// ��ȡ��ǩ�ؼ��ͻ���Rect����������������ʺϷ��ñ�ǩҳ     
	m_tab.GetClientRect(&tabRect);     
	tabRect.left += 1;     
	tabRect.right -= 1;     
	tabRect.top += 25;     
	tabRect.bottom -= 1; 


#if PAGENUM == 2
	switch (m_tab.GetCurSel())     
	{     
	case 0:     
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW); 
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		break;
	case 1:
		m_GraDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_HIDEWINDOW);
		m_TrimDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		break;
	default:     
		break;     
	}
#endif

#if PAGENUM == 1
	switch (m_tab.GetCurSel())     
	{     
		case 0:     
			m_OperDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width(), tabRect.Height(), SWP_SHOWWINDOW);
		break;
		default:     
		break;     
	}
#endif
	
}

void CPCRProjectDlg::GetCom()
{
	HANDLE  hCom;
	int i;
	CString str;
	BOOL flag;

	flag = FALSE;	//�����Ƿ��ҵ���־

	for (i = 1;i <= 16;i++)
	{//�˳���֧��16�����ڣ���1��16���β���

		str.Format(_T("\\\\.\\COM%d"),i);		//����Ҫ���ҵĴ��ڵ�ַ

		hCom = CreateFile(str, 0, 0, 0, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);	//��ָ����ַ�Ĵ���
		if(INVALID_HANDLE_VALUE != hCom )
		{//�ܴ򿪸ô��ڣ�����Ӹô���
			CloseHandle(hCom);
			str = str.Mid(4);		//���ַ�����ʽȡ�����ڵ�����COMX
			m_StaticOpenComm = str;	//��ȡ���Ĵ����ַ������ָ�ֵ���ı��ؼ�
			flag = TRUE;
		}
		else
			{//���ܴ򿪽��򿪱�ʾ���false
				if (flag == TRUE)
					flag = TRUE;
				else
					flag = FALSE;
			}
	}

	if (flag == TRUE)	//�ҵ��˿�
	{
		CString str1;

		str1 = m_StaticOpenComm.GetAt(3);	//�������ַ������֣�COMX���е�����Xȡ��
		pnum = atoi(str1);	//ת��ʮ������ֵ

//		m_StaticOpenComm = _T("Connect ") + m_StaticOpenComm + str1;
//		UpdateData(FALSE);
	}
	else	//û�ҵ��˿�
		AfxMessageBox(_T("Can't find the comm port"));	
}

void CPCRProjectDlg::OnBnClickedBtnOpencomm()
{
	// TODO: Add your control notification handler code here
}


void CPCRProjectDlg::OnBnClickedBtnOpenhid()
{
	// TODO: Add your control notification handler code here
	if(CPCRProjectDlg::FindTheHID()) m_TrimDlg.ResetTrim();
}


void CPCRProjectDlg::OnBnClickedBtnSendhid()
{
	// TODO: Add your control notification handler code here
	ReadAndWriteToDevice();
}


void CPCRProjectDlg::OnBnClickedBtnReadhid()
{
	// TODO: Add your control notification handler code here
}


void CPCRProjectDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here

	//��ʼ��������ʱ�õ�Overlapped�ṹ��
	//ƫ��������Ϊ0
	ReadOverlapped.Offset=0;
	ReadOverlapped.OffsetHigh=0;
	//����һ���¼����ṩ��ReadFileʹ�ã���ReadFile���ʱ��
	//�����ø��¼�Ϊ����״̬��
	ReadOverlapped.hEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	//����һ����������̣߳����ڹ���״̬��
	pReadReportThread=AfxBeginThread(ReadReportThread,
		this,
		THREAD_PRIORITY_NORMAL,
		0,
		CREATE_SUSPENDED,
		NULL);
	//��������ɹ�����ָ����̵߳�����
	if(pReadReportThread!=NULL)
	{
		pReadReportThread->ResumeThread();
	}
}

void CPCRProjectDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	EndDialog(IDCANCEL);
	CDialogEx::OnClose();
}


void CPCRProjectDlg::GetFWVersion()
{
	// read HID firmware version
	TxData[0] = 0xaa;		//preamble code
	TxData[1] = 0x04;		//command  TXC
	TxData[2] = 0x05;		//data length
	TxData[3] = 0x27;		//data type
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

	if (g_DeviceDetected)
	{
		WriteHIDOutputReport();		// ��graphic HID��������

		ReadHIDInputReport();		// ��graphic ȡHID��������

	}
	else
	{
		sGraFirmwareVer = "Device not detected";
	}


}

void CAboutDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);

	// TODO: Add your message handler code here

	memset(TxData,0,sizeof(TxData));	//����buffer����

	SetDlgItemText(IDC_STATIC_FWVERSION,sGraFirmwareVer);
	
}




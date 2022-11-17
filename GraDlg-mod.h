#pragma once

//***************************************************************
//Constant definition
//***************************************************************
#define UM_GRAPROCESS WM_USER+104	//Graphic dialog自定义消息序号

#define sendgramsg		1		// ReadRow button
#define sendpagemsg		2		// 画页命令
#define sendvideomsg	3		// video命令
#define readgramsg      5		// read commad
#define gradatanum 200		   // graphic dialog transmitted data number

#define RowNum24 24		// display row number
#define ColNum24 24		// display column number
#define RowNum12 12		// display row number
#define ColNum12 12		// display column number

//#define pixelsize24 5	// display size for one pixel
//#define pixelsize12 10	// display size for one pixel

#define pixelsize24 10	// display size for one pixel - 5 for small pixel
#define pixelsize12 20	// display size for one pixel - 10 for small pixel

#define dprow12 0x01		// display one line with 12 pixel
#define dppage12 0x02		// display one page with 12 pixel
#define dpvideo12 0x03		// display video with 12 pixel
#define spiread	0x04		// only SPI read
#define adccvt	0x05		// only ADC conversion and SPI read
#define ndread	0x06		// non-destructive read
#define dprow24	0x07		// display one line with 24 pixel
#define dppage24 0x08		// display one page with 24 pixel
#define heatstr 0x09		// start auto-heating
#define heatstop 0x0a		// stop auto-heating
#define dpvideo24 0x0b		// display video with 24 pixel

#define EditClearNum	192		// 显示数据编辑框变量清零行数

#define DRAWPAGE		1		// OnPaint中画页
#define DRAWROW			2		// OnPaint中画行

//*****************************************************************
//External variable
//*****************************************************************
extern byte PCRType;				// PCR select type
extern int PCRCycCnt;				// PCR 依次发送计数
extern int PCRNum;					// 各PCR序号

extern BYTE PCRTypeFilterClass;		// Graphic command返回type 类别区分，如画行、页、vedio（返回type byte的低4位）
extern BYTE PCRTypeFilterNum;		// Graphic command返回type 各PCR区分（返回type byte的高4位）

extern int Page12_24Flag;			// 12行/24行画页标志
extern int Vedio12_24Flag;			// 12行/24行Vedio标志




// CGraDlg dialog

class CGraDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGraDlg)

public:
	CGraDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGraDlg();

// Dialog Data
	enum { IDD = IDD_GRAPHIC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL m_PipelineMode;

	int		m_EndCycle;

	DECLARE_MESSAGE_MAP()
public:
	CString m_EditReadRow;
	afx_msg void OnClickedBtnReadrow();
	void GraCalMainMsg();
	afx_msg LRESULT OnGraProcess(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnDprow24();
	afx_msg void OnBnClickedBtnDppage12();
	afx_msg void OnBnClickedBtnDppage24();
	afx_msg void OnBnClickedBtnDpvedio();
	afx_msg void OnBnClickedBtnStopvideo();
	CButton m_ShowAllData;
	int m_ShowAllDataInt;
	CButton m_ReadHex;
	int m_ReadHexInt;
	CButton m_ReadDec;
	CButton m_ADCData;
	CButton m_ShowValidData;
	CString m_PixelData;
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnAdcconvert();
	CString m_ADCRecdata;
	afx_msg void OnBnClickedBtnClearadc();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedBtnDpvideo24();
	afx_msg void OnBnClickedRadioAdcdata();
	void SetGainMode(int gain);
	void DisplayAVG(unsigned int);
	void DisplayAVG2(unsigned int, unsigned int);
//	afx_msg void OnThumbposchangingSlider1(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
//	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void DisplayHDR24(void);
	int m_GainMode;
	afx_msg void OnClickedRadiolowgain();
//	afx_msg void OnUpdateRadiohdr(CCmdUI *pCmdUI);
//	afx_msg void OnUpdateRadiohighgain(CCmdUI *pCmdUI);
	afx_msg void OnRadiohighgain();
	afx_msg void OnRadiohdr();
	int m_FrameSize;
	afx_msg void OnClickedRadio12();
	afx_msg void OnRadio24();
	afx_msg void OnBnClickedButtonCapture();
	void CaptureFrame(void);
	void CaptureFrame12(void);
	void CaptureFrame24(void);
	BOOL m_PixOut;
	afx_msg void OnClickedCheckPixout();
	BOOL m_OrigOut;
	afx_msg void OnClickedCheckOrigout();
	void GraDlgDrawPattern();
	void MakeGraPacket(byte pCmd, byte pType, byte pData);
	void DrawRaw(CDC *pBufDC, CRect pRect,CBrush pBrush[RowNum24][ColNum24], int pRowNum, int pColNum, int pPixelSize, int pPCRNum);
	void DrawPage(CDC *pBufDC, CRect pRect, CBrush pBrush[RowNum24][ColNum24], int pPixelSize, int pPCRNum);
	
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	int cyclePage12(void);
	int cyclePage24(void);
	void cycleRow12(void);
	void cycleRow24(void);
	afx_msg void OnEnChangeEditAdcdata();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnCopy();
//	afx_msg void OnBnClickedBtnDpvideo25();
	afx_msg void OnBnClickedCheckImgen();

	void PreRead12(void);
	void PreRead24(void);
	afx_msg void OnBnClickedCheckCh1();
	afx_msg void OnBnClickedCheckCh2();
	int ADCCorrection(int NumData, BYTE HighByte, BYTE LowByte);
	afx_msg void OnBnClickedButtonSaveimage();

	void RenderPattern();
	void DisplayPattern(int chip);
	void CGraDlg::CalculateAVG(unsigned int chip, unsigned int cnt);
	afx_msg void OnBnClickedLedIndvEn();
	afx_msg void OnBnClickedLed1En();
	afx_msg void OnBnClickedLedSwitch();
	afx_msg void OnBnClickedCheckLogen();
	afx_msg void OnBnClickedCheckCh3();
	afx_msg void OnBnClickedCheckCh4();
	void SetChannelMask(void);
	afx_msg void OnBnClickedCheckPipe();
	BOOL m_KeepHistory;
	afx_msg void OnClickedCheckKeephistory();
	void UpdatePCRCurve(int, int);
};

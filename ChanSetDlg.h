#pragma once


// CChanSetDlg dialog

class CChanSetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChanSetDlg)

public:
	CChanSetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChanSetDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_CHANSET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
//	int m_ChanIndex;
	int m_ChipIndex;
	int m_ChanIndex;
	int m_IntIndex;
	int m_LedIndex;
	int m_NumSample;
//	float m_IntTime;
//	float m_IntTime;
	BOOL m_RemapEn;
	float m_IntTime;
	int m_ChanIndex1;
	int m_ChipIndex1;
	int m_LedIndex1;
	float m_IntTime1;
};

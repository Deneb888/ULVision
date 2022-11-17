#pragma once


// CDisplayProperty dialog

class CDisplayProperty : public CDialogEx
{
	DECLARE_DYNAMIC(CDisplayProperty)

public:
	CDisplayProperty(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDisplayProperty();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG4 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_flip1;
	BOOL m_flip2;
	BOOL m_flip3;
	BOOL m_flip4;
	BOOL m_row_number;
	BOOL m_time_stamp;
	BOOL m_capture_setting;
	BOOL m_frame_average;
};

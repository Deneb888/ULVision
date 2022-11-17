#pragma once


// CKineticDlg dialog

class CKineticDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CKineticDlg)

public:
	CKineticDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CKineticDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	UINT m_Interval;
	UINT m_Maxcnt;
};

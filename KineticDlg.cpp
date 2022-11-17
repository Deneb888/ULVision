// KineticDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "KineticDlg.h"
#include "afxdialogex.h"


// CKineticDlg dialog

IMPLEMENT_DYNAMIC(CKineticDlg, CDialogEx)

CKineticDlg::CKineticDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKineticDlg::IDD, pParent)
	, m_Interval(0)
	, m_Maxcnt(0)
{

}

CKineticDlg::~CKineticDlg()
{
}

void CKineticDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Interval);
	DDV_MinMaxUInt(pDX, m_Interval, 1, 1000);
	DDX_Text(pDX, IDC_EDIT2, m_Maxcnt);
	DDV_MinMaxUInt(pDX, m_Maxcnt, 2, 5000);
}


BEGIN_MESSAGE_MAP(CKineticDlg, CDialogEx)
END_MESSAGE_MAP()


// CKineticDlg message handlers

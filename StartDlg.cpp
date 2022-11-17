// StartDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "StartDlg.h"
#include "afxdialogex.h"


// CStartDlg dialog

IMPLEMENT_DYNAMIC(CStartDlg, CDialogEx)

CStartDlg::CStartDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG3, pParent)
	, m_NumChipsInstalled(0)
{

}

CStartDlg::~CStartDlg()
{
}

void CStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_NumChipsInstalled);
	DDV_MinMaxInt(pDX, m_NumChipsInstalled, 1, 4);
}


BEGIN_MESSAGE_MAP(CStartDlg, CDialogEx)
END_MESSAGE_MAP()


// CStartDlg message handlers

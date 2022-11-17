// CDisplayProperty.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "CDisplayProperty.h"
#include "afxdialogex.h"


// CDisplayProperty dialog

IMPLEMENT_DYNAMIC(CDisplayProperty, CDialogEx)

CDisplayProperty::CDisplayProperty(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG4, pParent)
	, m_flip1(FALSE)
	, m_flip2(FALSE)
	, m_flip3(FALSE)
	, m_flip4(FALSE)
	, m_row_number(FALSE)
	, m_time_stamp(FALSE)
	, m_capture_setting(FALSE)
	, m_frame_average(FALSE)
{

}

CDisplayProperty::~CDisplayProperty()
{
}

void CDisplayProperty::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_CHANFLIP1, m_flip1);
	DDX_Check(pDX, IDC_CHECK_CHANFLIP2, m_flip2);
	DDX_Check(pDX, IDC_CHECK_CHANFLIP3, m_flip3);
	DDX_Check(pDX, IDC_CHECK_CHANFLIP4, m_flip4);
	DDX_Check(pDX, IDC_CHECK_SHOWROWNUM, m_row_number);
	DDX_Check(pDX, IDC_CHECK_TIMESTAMP, m_time_stamp);
	DDX_Check(pDX, IDC_CHECK_CAPTURESETTINGS, m_capture_setting);
	DDX_Check(pDX, IDC_CHECK6, m_frame_average);
}


BEGIN_MESSAGE_MAP(CDisplayProperty, CDialogEx)
END_MESSAGE_MAP()


// CDisplayProperty message handlers

// ChanSetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PCRProject.h"
#include "ChanSetDlg.h"
#include "afxdialogex.h"


// CChanSetDlg dialog

IMPLEMENT_DYNAMIC(CChanSetDlg, CDialogEx)

CChanSetDlg::CChanSetDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_CHANSET, pParent)
	, m_ChanIndex(0)
	, m_ChipIndex(0)
	, m_NumSample(0)
//	, m_IntTime(0)
// , m_IntTime(0)
, m_RemapEn(FALSE)
, m_IntTime(1)
, m_ChanIndex1(0)
, m_ChipIndex1(0)
, m_LedIndex1(0)
, m_IntTime1(0)
{

	m_ChanIndex = 0;
	m_IntIndex = 0;
	m_LedIndex = 0;
}

CChanSetDlg::~CChanSetDlg()
{
}

void CChanSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//  DDX_CBIndex(pDX, IDC_COMBO_CH, m_ChanIndex);
	//  DDV_MinMaxInt(pDX, m_ChanIndex, 1, 8);
	DDX_CBIndex(pDX, IDC_COMBO_CHIP, m_ChipIndex);
	DDV_MinMaxInt(pDX, m_ChipIndex, 0, 3);
	DDX_CBIndex(pDX, IDC_COMBO_CH, m_ChanIndex);
	DDV_MinMaxInt(pDX, m_ChanIndex, 0, 7);
	//	DDX_CBIndex(pDX, IDC_COMBO_INTT, m_IntIndex);
	//	DDV_MinMaxInt(pDX, m_IntIndex, 0, 7);
	DDX_CBIndex(pDX, IDC_COMBO_LED, m_LedIndex);
	DDV_MinMaxInt(pDX, m_LedIndex, 0, 3);
	//	DDX_Text(pDX, IDC_EDIT1, m_NumSample);
	//	DDV_MinMaxInt(pDX, m_NumSample, 1, 4);
	DDX_Check(pDX, IDC_CHECK_REMAP, m_RemapEn);
	DDX_Text(pDX, IDC_EDIT_INTT, m_IntTime);
	DDV_MinMaxFloat(pDX, m_IntTime, 1, 65000); 
	DDX_CBIndex(pDX, IDC_COMBO_CH2, m_ChanIndex1);
	DDX_CBIndex(pDX, IDC_COMBO_CHIP2, m_ChipIndex1);
	DDX_CBIndex(pDX, IDC_COMBO_LED2, m_LedIndex1);
	DDX_Text(pDX, IDC_EDIT_INTT2, m_IntTime1);
	DDV_MinMaxFloat(pDX, m_IntTime1, 1, 66000);
}


BEGIN_MESSAGE_MAP(CChanSetDlg, CDialogEx)
END_MESSAGE_MAP()


// CChanSetDlg message handlers

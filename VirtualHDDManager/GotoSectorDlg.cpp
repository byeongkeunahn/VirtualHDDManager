// GotoSectorDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VirtualHDDManager.h"
#include "GotoSectorDlg.h"
#include "afxdialogex.h"

#define INIT_MIN_VAL		0x01
#define INIT_MAX_VAL		0x02
#define INIT_MASK			(INIT_MIN_VAL | INIT_MAX_VAL)

// CGotoSectorDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CGotoSectorDlg, CDialog)

CGotoSectorDlg::CGotoSectorDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CGotoSectorDlg::IDD, pParent)
{
	m_uiInit = 0;
	m_uiMinRange = 0;
	m_uiMaxRange = (UINT64)-1;
	m_uiValue = 0;
}

CGotoSectorDlg::~CGotoSectorDlg()
{
}

void CGotoSectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_LBANUM, m_uiValue);
	DDV_MinMaxULongLong(pDX, m_uiValue, m_uiMinRange, m_uiMaxRange);

}

BEGIN_MESSAGE_MAP(CGotoSectorDlg, CDialog)
END_MESSAGE_MAP()

// CGotoSectorDlg 함수입니다.

void CGotoSectorDlg::SetMinRange(UINT64 uiNumber)
{
	m_uiMinRange = uiNumber;
	m_uiInit |= INIT_MIN_VAL;
}

void CGotoSectorDlg::SetMaxRange(UINT64 uiNumber)
{
	m_uiMaxRange = uiNumber;
	m_uiInit |= INIT_MAX_VAL;
}

void CGotoSectorDlg::SetValue(UINT64 uiNumber)
{
	if (uiNumber < m_uiMinRange || uiNumber > m_uiMaxRange)
		return;

	m_uiValue = uiNumber;
}

UINT64 CGotoSectorDlg::GetValue()
{
	return m_uiValue;
}

// CGotoSectorDlg 메시지 처리기 및 재정의 함수입니다.

BOOL CGotoSectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
	str.Format(_T("%llu - %llu"), m_uiMinRange, m_uiMaxRange);
	CWnd *pTextRange = GetDlgItem(IDC_TEXT_RANGE);
	pTextRange->SetWindowTextW(str);

	return TRUE;
}

INT_PTR CGotoSectorDlg::DoModal()
{
	if ((m_uiInit & INIT_MASK) != INIT_MASK)
		return -1;

	return CDialog::DoModal();
}

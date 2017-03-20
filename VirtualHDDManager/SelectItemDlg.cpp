// SelectItemDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VirtualHDDManager.h"
#include "SelectItemDlg.h"
#include "afxdialogex.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/HandleManagement.h"

#define SIDD_GROWTH_SIZE		20
#define SIDD_TYPE_MAX			100

#define SIDD_UNIT_SIZE			(sizeof(void *))
#define SIDD_TOTAL_SIZE(ccType)	(SIDD_UNIT_SIZE*(ccType) + sizeof(LPARAM))

// CSelectItemDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSelectItemDlg, CDialogEx)

CSelectItemDlg::CSelectItemDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(CSelectItemDlg::IDD, pParent)
{
	m_bInit = FALSE;
	m_lParamSelected = SID_INVALID_LPARAM;
}

CSelectItemDlg::~CSelectItemDlg()
{
	DestroyDataStorage();
}

void CSelectItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSelectItemDlg, CDialogEx)
END_MESSAGE_MAP()


// CSelectItemDlg 구현입니다.


void CSelectItemDlg::InitDataStorage(UINT ccType)
{
	if (m_bInit) return;
	if (!ccType || ccType > SIDD_TYPE_MAX) return;

	vhmsize_t szDataStorage;
	szDataStorage = SIDD_TOTAL_SIZE(ccType);

	m_hHandleID = HandleIDCreate(szDataStorage, SIDD_GROWTH_SIZE);
	m_ccType = ccType;

	m_pszTypeString = new CString[ccType];

	m_bInit = TRUE;
}

void CSelectItemDlg::SetTypeString(UINT uiIndex, LPCTSTR lpszTypeString)
{
	if (!m_bInit) return;
	if (uiIndex >= m_ccType) return;

	m_pszTypeString[uiIndex] = lpszTypeString;
}

void CSelectItemDlg::DestroyDataStorage()
{
	if (!m_bInit) return;
	
	RemoveAllData();

	HandleIDDestroy((HHandleID)m_hHandleID);
	m_hHandleID = nullptr;

	delete[] m_pszTypeString;

	m_bInit = FALSE;
}

QWORD CSelectItemDlg::CreateData()
{
	if (!m_bInit) return (QWORD)-1;

	QWORD qwHandle = HandleAdd((HHandleID)m_hHandleID);
	CString *pString;
	for (UINT i = 0; i < m_ccType; ++i)
	{
		pString = new CString;
		SetData(qwHandle, i, pString);
	}

	return qwHandle;
}

void CSelectItemDlg::SetDataBuffer(QWORD qwHandle, UINT uiIndex, LPCTSTR lpszData)
{
	if (!m_bInit) return;
	if (uiIndex >= m_ccType) return;

	((CString *)GetData(qwHandle, uiIndex))->Format(_T("%s"), lpszData);
}

void CSelectItemDlg::SetData(QWORD qwHandle, UINT uiIndex, void *pData)
{
	if (!m_bInit) return;
	if (uiIndex >= m_ccType) return;

	HandleSetData((HHandleID)m_hHandleID, qwHandle,
		uiIndex * SIDD_UNIT_SIZE, sizeof(void *), sizeof(void *), &pData);
}

void *CSelectItemDlg::GetData(QWORD qwHandle, UINT uiIndex)
{
	if (!m_bInit) return nullptr;
	if (uiIndex >= m_ccType) return nullptr;

	void *pData;
	HandleGetData((HHandleID)m_hHandleID, qwHandle,
		uiIndex * SIDD_UNIT_SIZE, sizeof(void *), sizeof(void *), &pData);

	return pData;
}

void CSelectItemDlg::SetUserParam(QWORD qwHandle, LPARAM lParam)
{
	if (!m_bInit) return;

	HandleSetData((HHandleID)m_hHandleID, qwHandle,
		SIDD_UNIT_SIZE * m_ccType, sizeof(LPARAM), sizeof(LPARAM), &lParam);
}

LPARAM CSelectItemDlg::GetUserParam(QWORD qwHandle)
{
	if (!m_bInit) return SID_INVALID_LPARAM;

	LPARAM lParam;
	HandleGetData((HHandleID)m_hHandleID, qwHandle,
		SIDD_UNIT_SIZE * m_ccType, sizeof(LPARAM), sizeof(LPARAM), &lParam);

	return lParam;
}

QWORD CSelectItemDlg::GetDataCount()
{
	if (!m_bInit) return (UINT)-1;
	return HandleGetHandleCount((HHandleID)m_hHandleID);
}

void CSelectItemDlg::RemoveAllData()
{
	if (!m_bInit) return;

	HHandleID hHandleID = (HHandleID)m_hHandleID;

	QWORD qwHandle;
	UINT i;
	CString *pString;

	for (qwHandle = HandleGetFirstHandle(hHandleID); qwHandle != (QWORD)-1;
		qwHandle = HandleGetNextHandle(hHandleID, qwHandle))
	for (i = 0; i < m_ccType; ++i)
	{
		pString = (CString *)GetData(qwHandle, i);
		delete pString;
	}

	HandleRemoveAll((HHandleID)m_hHandleID);
}

LPARAM CSelectItemDlg::GetSelectedUserParam()
{
	return m_lParamSelected;
}

// CSelectItemDlg 메시지 처리기 및 재정의 함수입니다.


INT_PTR CSelectItemDlg::DoModal()
{
	if (!m_bInit) return -1;
	if (HandleGetHandleCount((HHandleID)m_hHandleID) == 0) return -1;

	return CDialogEx::DoModal();
}

BOOL CSelectItemDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CListCtrl *pListCtrl;
	pListCtrl = (CListCtrl *)GetDlgItem(IDC_LIST_SELECT_ITEM);

	// 종류(열) 삽입
	for (UINT i = 0; i < m_ccType; ++i)
		pListCtrl->InsertColumn(i, (LPCTSTR)m_pszTypeString[i], LVCFMT_LEFT, 100, -1);

	// 항목(행) 삽입
	HHandleID hHandleID = (HHandleID)m_hHandleID;
	QWORD qwHandleCount;
	QWORD qwHandle;
	CString *pString;
	LPCTSTR lpszData;
	int nItem;

	qwHandleCount = HandleGetHandleCount(hHandleID);
	qwHandle = HandleGetFirstHandle(hHandleID);

	for (UINT i = 0; i < qwHandleCount; ++i)
	{
		//HandleGetData(hHandleID, qwHandle, 0,
		//	sizeof(LPCTSTR), sizeof(LPCTSTR), &lpszData);

		nItem = pListCtrl->InsertItem(i, nullptr);
		for (UINT j = 0; j < m_ccType; ++j)
		{
			pString = (CString *)GetData(qwHandle, j);
			lpszData = pString->GetBuffer();
			pListCtrl->SetItem(nItem, j, LVIF_TEXT, lpszData, 0, 0, 0, 0);
		}
		pListCtrl->SetItemData(nItem, GetUserParam(qwHandle));

		qwHandle = HandleGetNextHandle(hHandleID, qwHandle);
	}
	
	pListCtrl->SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	
	return TRUE;
}

void CSelectItemDlg::OnOK()
{
	CListCtrl *pListCtrl;
	int nItem;
	LPARAM lParam;

	pListCtrl = (CListCtrl *)GetDlgItem(IDC_LIST_SELECT_ITEM);
	nItem = pListCtrl->GetNextItem(-1, LVNI_SELECTED);

	if (nItem != -1)
	{
		lParam = pListCtrl->GetItemData(nItem);
	}
	else
		lParam = SID_INVALID_LPARAM;

	m_lParamSelected = lParam;	

	CDialogEx::OnOK();
}

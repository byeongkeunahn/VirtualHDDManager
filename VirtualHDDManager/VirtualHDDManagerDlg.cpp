
// VirtualHDDManagerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMCommon/VHMApiWrapper.h"
#include "VHMCommon/Stream.h"
#include "VHMCommon/StreamMemFixed.h"
#include "VHMCommon/StreamMemDyn.h"
#include "VirtualHDDManager.h"
#include "SectorViewWnd.h"
#include "VirtualHDDManagerDlg.h"
#include "afxdialogex.h"
#include "GotoSectorDlg.h"
#include "SelectItemDlg.h"

#include "VHMIOWrapper.h"
#include "VHMDisk.h"
#include "VHMDiskFile.h"
#include "VHMDiskRam.h"
#include "VHMDiskVHD.h"
#include "VHMDiskVHDX.h"
#include "VHMDiskVMDK.h"
#include "VHMDiskVDI.h"
#include "VHMPartition.h"
#include "VHMFilesystem.h"
#include "VHMRoot.h"

//#include "../VHMScript/VHMScriptBase.h"
//#include "../VHMScript/VHMScriptStack.h"
//#include "../VHMScript/VHMScriptEngine.h"
#include "VHMScript.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVirtualHDDManagerDlg ��ȭ ����


CVirtualHDDManagerDlg::CVirtualHDDManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CVirtualHDDManagerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bMRULoaded = FALSE;

	m_pVHMDisk = nullptr;
}

CVirtualHDDManagerDlg::~CVirtualHDDManagerDlg()
{
	CloseDisk();

	MRUFinialize();
}

void CVirtualHDDManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVirtualHDDManagerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_VHM_NEW, &CVirtualHDDManagerDlg::OnVhmNew)
	ON_COMMAND(ID_VHM_OPEN, &CVirtualHDDManagerDlg::OnVhmOpen)
	ON_COMMAND(ID_VHM_CLOSE, &CVirtualHDDManagerDlg::OnVhmClose)
	ON_COMMAND_RANGE(ID_VHM_MRU_BASE, (ID_VHM_MRU_BASE + VHM_MRU_COUNT_MAX), &CVirtualHDDManagerDlg::OnVhmOpenMRU)
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_GOTO_SECTOR, &CVirtualHDDManagerDlg::OnGotoSector)
	ON_COMMAND(ID_VHM_NEWRD, &CVirtualHDDManagerDlg::OnVhmNewrd)
	ON_COMMAND(ID_VBR_WRITE, &CVirtualHDDManagerDlg::OnVbrWrite)
	ON_COMMAND(ID_SELECT_PARTITON, &CVirtualHDDManagerDlg::OnSelectPartiton)
	ON_COMMAND(ID_ABOUT, &CVirtualHDDManagerDlg::OnAbout)
	ON_COMMAND(ID_SCRIPT_EXECUTE, &CVirtualHDDManagerDlg::OnScriptExecute)
	ON_COMMAND(ID_EXIT, &CVirtualHDDManagerDlg::OnExit)
	ON_COMMAND(ID_TEST_1, &CVirtualHDDManagerDlg::OnTest1)
END_MESSAGE_MAP()


// CVirtualHDDManagerDlg �޽��� ó����


BOOL CVirtualHDDManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && m_wndSectorView.GetSafeHwnd() != nullptr)
	{
		m_wndSectorView.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CVirtualHDDManagerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// ���� ǥ�� ������ ����
	CSize sz;
	m_wndSectorView.CalculateAreaSize(SECTORSIZE, &sz);
	m_wndSectorView.Create(nullptr, nullptr, WS_VISIBLE|WS_CHILD, CRect(CPoint(0,0), sz), this, 0x3F3F);

	// MRU (Most Recent Used) ��� �ε�
	MRUInitialize();

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CVirtualHDDManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CVirtualHDDManagerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CVirtualHDDManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CVirtualHDDManagerDlg::SVWCallbackProc(UINT64 uiCommand, VPARAM vparam1, VPARAM vparam2, VPARAM vuserparam)
{
	CVirtualHDDManagerDlg *pDlg = (CVirtualHDDManagerDlg *) vuserparam;
	switch(uiCommand)
	{
	case SVWM_READ:
		{
			BYTE *pBuffer = (BYTE *) vparam1;
			SVWM_DATA_PACKET *pDataPacket = (SVWM_DATA_PACKET *) vparam2;
			return pDlg->Read(pDataPacket->uiStartSector, pDataPacket->uiSectorCount, pBuffer, pDataPacket->uiSectorCount * pDataPacket->uiSectorSize);
		}
		break;
	case SVWM_WRITE:
		{
			BYTE *pBuffer = (BYTE *) vparam1;
			SVWM_DATA_PACKET *pDataPacket = (SVWM_DATA_PACKET *) vparam2;
			return pDlg->Write(pDataPacket->uiStartSector, pDataPacket->uiSectorCount, pBuffer);
		}
		break;
	case SVWM_QUERY:
		{
			SVWM_QUERY_PACKET *pQueryPacket = (SVWM_QUERY_PACKET *) vparam1;
			pQueryPacket->uiFirstSector = 0;
			pQueryPacket->uiLastSector = pDlg->GetVHMDiskClass()->GetSectorCount() - 1;
		}
		break;
	default:
		return -1; // Unsupported
	}

	return 0;
}

int CVirtualHDDManagerDlg::CreateDisk()
{
	if(m_pVHMDisk != nullptr)
		return -1;

	CFileDialog dlg(FALSE, _T("*.vhm"), nullptr, OFN_OVERWRITEPROMPT, _T("���� ��ũ ����|*.vhm|��� ����|*.*|"));
	if(dlg.DoModal() != IDOK) return -1;

	AfxMessageBox(dlg.GetFileName());
	/*
	CVHMDiskFile *pVHMDiskFile = new CVHMDiskFile;
	if (pVHMDiskFile->CreateDisk(dlg.GetOFN().lpstrFile, SECTORSIZE, 1024) == -1)
	{
		AfxMessageBox(_T("Fail"));
		delete pVHMDiskFile;
		return -1;
	}

	// MRU�� �߰�
	MRUAdd(dlg.GetOFN().lpstrFile);

	m_pVHMDisk = pVHMDiskFile;

	m_wndSectorView.SetCallbackFunc(CVirtualHDDManagerDlg::SVWCallbackProc, (VPARAM) this);
	m_wndSectorView.SetSectorSize(SECTORSIZE);
	m_wndSectorView.SetOffset(0);

	m_qwPartitionID = INVALID_PARTITION_ID;

	return VHM_ERROR_SUCCESS;
*/
	return CreateDisk(dlg.GetOFN().lpstrFile, SECTORSIZE, 1024, VIRTUAL_DISK_FORMAT_VHM);
}

int CVirtualHDDManagerDlg::CreateDisk(LPCTSTR lpszPath, DWORD dwSectorSize, QWORD qwSectorCount, DWORD dwDiskType, QWORD qwFlags)
{
	int err_code;
	CVHMDisk *pVHMDisk;

	switch (dwDiskType)
	{
	case VIRTUAL_DISK_FORMAT_VHM:
		pVHMDisk = (CVHMDisk *) new CVHMDiskFile;
		err_code = ((CVHMDiskFile *)pVHMDisk)->CreateDisk(lpszPath, dwSectorSize, qwSectorCount);
		break;
	default:
		err_code = VHM_ERROR_UNSUPPORTED;
		break;
	}

	if (err_code != VHM_ERROR_SUCCESS)
		return err_code;

	m_pVHMDisk = pVHMDisk;

	m_wndSectorView.SetCallbackFunc(CVirtualHDDManagerDlg::SVWCallbackProc, (VPARAM) this);
	m_wndSectorView.SetSectorSize(SECTORSIZE);
	m_wndSectorView.SetOffset(0);

	m_qwPartitionID = INVALID_PARTITION_ID;

	MRUAdd(lpszPath);

	return VHM_ERROR_SUCCESS;
}

int CVirtualHDDManagerDlg::OpenDisk()
{
	if(m_pVHMDisk != nullptr)
		return -1;

	CFileDialog dlg(TRUE, _T("*.vhm"), nullptr, 0, _T("���� ��ũ ����|*.vhm|��� ����|*.*|"));
	if(dlg.DoModal() != IDOK) return -1;

	AfxMessageBox(dlg.GetFileName());

	return OpenDisk(dlg.GetOFN().lpstrFile);
	/*
	BOOL bSuccess = FALSE;

	// ���� ��ũ VHD
	CVHMDiskVHD *pVHMDiskVHD = new CVHMDiskVHD;
	if (pVHMDiskVHD->OpenDisk(dlg.GetOFN().lpstrFile) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VHD"));
		m_pVHMDisk = pVHMDiskVHD;
		goto final_1;
	}
	delete pVHMDiskVHD;

	// ���� ��ũ VHDX
	CVHMDiskVHDX *pVHMDiskVHDX = new CVHMDiskVHDX;
	if (pVHMDiskVHDX->OpenDisk(dlg.GetOFN().lpstrFile) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VHDX"));
		m_pVHMDisk = pVHMDiskVHDX;
		goto final_1;
	}
	delete pVHMDiskVHDX;

	// ���� ��ũ VMDK
	CVHMDiskVMDK *pVHMDiskVMDK = new CVHMDiskVMDK;
	if (pVHMDiskVMDK->OpenDisk(dlg.GetOFN().lpstrFile) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VDI"));
		m_pVHMDisk = pVHMDiskVMDK;
		goto final_1;
	}
	delete pVHMDiskVMDK;

	// ���� ��ũ VDI
	CVHMDiskVDI *pVHMDiskVDI = new CVHMDiskVDI;
	if (pVHMDiskVDI->OpenDisk(dlg.GetOFN().lpstrFile) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VDI"));
		m_pVHMDisk = pVHMDiskVDI;
		goto final_1;
	}
	delete pVHMDiskVDI;

	// ���� ��ũ VHM
	CVHMDiskFile *pVHMDiskFile = new CVHMDiskFile;
	if(pVHMDiskFile->OpenDisk(dlg.GetOFN().lpstrFile) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VHM"));
		m_pVHMDisk = pVHMDiskFile;
		goto final_1;
	}
	// ���� ��ũ (raw)
	if(pVHMDiskFile->OpenDiskRaw(dlg.GetOFN().lpstrFile) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as RAW format"));
		m_pVHMDisk = pVHMDiskFile;
		goto final_1;
	}
	delete pVHMDiskFile;

final_1:
	if(!bSuccess)
		return FALSE;

	// MRU�� �߰�
	MRUAdd(dlg.GetOFN().lpstrFile);
        
	m_wndSectorView.SetCallbackFunc(CVirtualHDDManagerDlg::SVWCallbackProc, (VPARAM) this);
	m_wndSectorView.SetSectorSize(m_pVHMDisk->GetSectorSize());
	m_wndSectorView.SetOffset(0);

	m_qwPartitionID = INVALID_PARTITION_ID;

	return 0;
*/
}

int CVirtualHDDManagerDlg::OpenDisk(LPCTSTR lpszPath, BOOL bMRU)
{
	BOOL bSuccess = FALSE;
	int err_code;

	err_code = VHM_ERROR_GENERIC;

	if (CheckFileExistence(lpszPath) == FALSE)
	{
		if (bMRU)
		{
			if (AfxMessageBox(_T("Can't open file\r\nDo you want to remove this file from MRU list?"), MB_ICONEXCLAMATION|MB_YESNO) == IDYES)
				MRUDelete(MRUCheckExistence(lpszPath));
		}
		else
			AfxMessageBox(_T("Can't open file"));

		goto final_1;
	}

	// ���� ��ũ VHD
	CVHMDiskVHD *pVHMDiskVHD = new CVHMDiskVHD;
	if (pVHMDiskVHD->OpenDisk(lpszPath) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VHD"));
		m_pVHMDisk = pVHMDiskVHD;
		goto final_1;
	}
	delete pVHMDiskVHD;

	// ���� ��ũ VHDX
	CVHMDiskVHDX *pVHMDiskVHDX = new CVHMDiskVHDX;
	if (pVHMDiskVHDX->OpenDisk(lpszPath) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VHDX"));
		m_pVHMDisk = pVHMDiskVHDX;
		goto final_1;
	}
	delete pVHMDiskVHDX;

	// ���� ��ũ VMDK
	CVHMDiskVMDK *pVHMDiskVMDK = new CVHMDiskVMDK;
	if (pVHMDiskVMDK->OpenDisk(lpszPath) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VDI"));
		m_pVHMDisk = pVHMDiskVMDK;
		goto final_1;
	}
	delete pVHMDiskVMDK;

	// ���� ��ũ VDI
	CVHMDiskVDI *pVHMDiskVDI = new CVHMDiskVDI;
	if (pVHMDiskVDI->OpenDisk(lpszPath) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VDI"));
		m_pVHMDisk = pVHMDiskVDI;
		goto final_1;
	}
	delete pVHMDiskVDI;

	// ���� ��ũ VHM
	CVHMDiskFile *pVHMDiskFile = new CVHMDiskFile;
	if (pVHMDiskFile->OpenDisk(lpszPath) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as VHM"));
		m_pVHMDisk = pVHMDiskFile;
		goto final_1;
	}
	// ���� ��ũ (raw)
	if (pVHMDiskFile->OpenDiskRaw(lpszPath) == 0)
	{
		bSuccess = TRUE;
		AfxMessageBox(_T("Disk file recognized as RAW format"));
		m_pVHMDisk = pVHMDiskFile;
		goto final_1;
	}
	delete pVHMDiskFile;

final_1:
	if (!bSuccess)
		return VHM_ERROR_GENERIC;

	// MRU�� �߰�
	MRUAdd(lpszPath);

	m_wndSectorView.SetCallbackFunc(CVirtualHDDManagerDlg::SVWCallbackProc, (VPARAM) this);
	m_wndSectorView.SetSectorSize(m_pVHMDisk->GetSectorSize());
	m_wndSectorView.SetOffset(0);

	m_qwPartitionID = INVALID_PARTITION_ID;

	return VHM_ERROR_SUCCESS;
}

int CVirtualHDDManagerDlg::Read(UINT64 nStartSector, UINT64 nSectorCount, BYTE *pBuffer, UINT64 nBufferSize)
{
	if(!m_pVHMDisk)
		return -1;

	return m_pVHMDisk->ReadSector(nStartSector, nSectorCount, pBuffer, nBufferSize);
}

int CVirtualHDDManagerDlg::Write(UINT64 nStartSector, UINT64 nSectorCount, BYTE *pBuffer)
{
	if(!m_pVHMDisk)
		return -1;

	return m_pVHMDisk->WriteSector(nStartSector, nSectorCount, pBuffer);
}

CVHMDisk *CVirtualHDDManagerDlg::GetVHMDiskClass()
{
	return m_pVHMDisk;
}

int CVirtualHDDManagerDlg::CloseDisk()
{
	if(!m_pVHMDisk)
		return 0;

	m_pVHMDisk->CloseDisk();
	delete m_pVHMDisk;
	m_pVHMDisk = nullptr;

	m_wndSectorView.NotifyDiskClosed();

	return 0;
}
void CVirtualHDDManagerDlg::MRUInitialize()
{
	UINT uiMRUCount;
	UINT i;
	
	// MRU �׸� ���� �ε�

	uiMRUCount = VHMRegReadInt(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), _T("Count"), VHM_MRU_NONE);

	if (uiMRUCount == VHM_MRU_NONE)
		uiMRUCount = 10;
	else if (uiMRUCount > VHM_MRU_COUNT_MAX)
		uiMRUCount = VHM_MRU_COUNT_MAX;

	VHMRegWriteInt(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), _T("Count"), uiMRUCount);
	m_uiMRUCount = uiMRUCount;

	// MRU �׸� �޸� �Ҵ�
	m_pstrMRU = new CString[m_uiMRUCount];

	for (i = 0; i < m_uiMRUCount; ++i)
		m_pstrMRU[i].Empty();

	// MRU �׸� �ε�
	MRULoad();

	// MRU�� �޴��� ������Ʈ
	MRUUpdateMenu();
}

void CVirtualHDDManagerDlg::MRUFinialize()
{
	if (!m_bMRULoaded)
		return;

	MRUFlush();

	delete[] m_pstrMRU;

	m_bMRULoaded = FALSE;
}

void CVirtualHDDManagerDlg::MRULoad()
{
	CString strItem;
	BYTE *pbMRU;
	TCHAR *pszMRU;
	DWORD cbData;
	UINT uiMRUEntryCount;
	UINT i;

	uiMRUEntryCount = 0;
	for (i = 0; i < m_uiMRUCount; ++i)
	{
		strItem.Format(_T("MRU%02d"), i);
		VHMRegReadStringGetSize(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), strItem.GetBuffer(), &cbData);
		if (!cbData) continue;

		pbMRU = new BYTE[cbData];
		pszMRU = (TCHAR *)pbMRU;
		if (VHMRegReadString(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), strItem.GetBuffer(),
			_T(""), pszMRU, cbData) != TRUE
			|| pszMRU[0] == _T('\0'))
		{
			delete[] pbMRU;
			continue;
		}

		m_pstrMRU[uiMRUEntryCount] = pszMRU;
		delete[] pbMRU;

		TRACE(_T("MRU: Key: %s, Path: %s\n"), strItem, m_pstrMRU[uiMRUEntryCount]);

		++uiMRUEntryCount;
	}

	m_uiMRUEntryCount = uiMRUEntryCount;
	m_bMRULoaded = TRUE;

	// MRU �ߺ� ����
	MRUDeduplicate();
}

void CVirtualHDDManagerDlg::MRUDeduplicate()
{
	if (!m_bMRULoaded)
		return;

	if (m_uiMRUEntryCount <= 1)
		return;

	UINT i;
	UINT uiIndex;

	for (i = 0; i < m_uiMRUEntryCount - 1; ++i)
	{
		for (;;)
		{
			uiIndex = MRUCheckExistence(m_pstrMRU[i], i + 1);
			if (uiIndex == VHM_MRU_NONE) break;

			MRUDelete(uiIndex);
		}
	}

}

void CVirtualHDDManagerDlg::MRUFlush()
{
	if (!m_bMRULoaded)
		return;

	CString strItem;
	UINT uiIndex;
	UINT i;

	// ���� �� ����
	RegDeleteKeyValue(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), _T("Count"));
	for (i = 0; i < m_uiMRUCount; ++i)
	{
		strItem.Format(_T("MRU%02d"), i);
		RegDeleteKeyValue(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), strItem);
	}

	// �� �� ����
	VHMRegWriteInt(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"), _T("Count"), m_uiMRUCount);

	uiIndex = 0;
	for (i = 0; i < m_uiMRUCount; ++i)
	{
		if (!m_pstrMRU[i].IsEmpty())
		{
			strItem.Format(_T("MRU%02d"), uiIndex);
			VHMRegWriteString(HKCU, _T("Software\\AhnOS\\VirtualHDDManager\\MRU"),
				strItem, m_pstrMRU[uiIndex].GetBuffer());
			++uiIndex;
		}
	}
}

void CVirtualHDDManagerDlg::MRUSetMaxItemCount(UINT ccMax)
{
	if (!m_bMRULoaded)
		return;

}

void CVirtualHDDManagerDlg::MRUAdd(LPCTSTR lpszString)
{
	if (!m_bMRULoaded)
		return;

	CString strAdd;
	UINT uiMRUEntryCountNew;
	UINT i;

	strAdd.Format(_T("%s"), lpszString);

	// �̹� �׸��� �����ϴ°�?
	i = MRUCheckExistence(lpszString);
	if (i != -1)
		MRUDelete(i);

	uiMRUEntryCountNew = min(m_uiMRUEntryCount + 1, m_uiMRUCount);

	for (i = uiMRUEntryCountNew - 1; i > 0; --i)
		m_pstrMRU[i].Format(_T("%s"), m_pstrMRU[i - 1]);

	m_pstrMRU[0].Format(_T("%s"), strAdd);
	m_uiMRUEntryCount = uiMRUEntryCountNew;

	// �޴� ������Ʈ
	MRUUpdateMenu();
}

void CVirtualHDDManagerDlg::MRUDelete(UINT uiIndex)
{
	if (!m_bMRULoaded)
		return;

	if (!m_uiMRUEntryCount || uiIndex >= m_uiMRUEntryCount)
		return;

	UINT i;

	// ���� �׸���� �ϳ��� ��ܿ�
	for (i = uiIndex; i < m_uiMRUEntryCount - 1; ++i)
		m_pstrMRU[i].Format(_T("%s"), m_pstrMRU[i + 1]);

	// ������ �ߺ��� ����
	m_pstrMRU[m_uiMRUEntryCount - 1].Empty();

	// MRU ��Ʈ�� �� ������Ʈ
	--m_uiMRUEntryCount;

	// �޴� ������Ʈ
	MRUUpdateMenu();
}

UINT CVirtualHDDManagerDlg::MRUCheckExistence(LPCTSTR lpszString, UINT uiIndexToStart)
{
	if (!m_bMRULoaded)
		return FALSE;

	if (uiIndexToStart >= m_uiMRUEntryCount)
		return VHM_MRU_NONE;

	UINT i;
	for (i = uiIndexToStart; i < m_uiMRUEntryCount; ++i)
	{
		// �׸��� �����ϸ� index ����
		if (m_pstrMRU[i] == lpszString)
			return i;
	}

	return VHM_MRU_NONE;
}

BOOL CVirtualHDDManagerDlg::MRUIsFull()
{
	if (!m_bMRULoaded)
		return FALSE;

	return (m_uiMRUCount == m_uiMRUEntryCount);
}

void CVirtualHDDManagerDlg::MRUUpdateMenu()
{
	if (!m_bMRULoaded || !GetSafeHwnd())
		return;

	CMenu *pMenuTemp;
	CMenu *pSubMenu;
	UINT uiCount;
	UINT i;
	
	// �޴� �ڵ� ���
	pMenuTemp = GetMenu();
	if (!pMenuTemp)	return;

	pMenuTemp = pMenuTemp->GetSubMenu(0);
	if (!pMenuTemp) return;

	pSubMenu = pMenuTemp->GetSubMenu(5);
	if (!pSubMenu) return;

	// ���� �׸� ����
	uiCount = pSubMenu->GetMenuItemCount();
	for (i = 0; i < uiCount; ++i)
		pSubMenu->DeleteMenu(0, MF_BYPOSITION);

	// MRU �׸� �߰�
	if (!m_uiMRUEntryCount)
	{
		pSubMenu->AppendMenu(MF_STRING | MF_GRAYED, ID_VHM_MRU_BASE, _T("(None)"));
	}
	else
	{
		for (i = 0; i < m_uiMRUEntryCount; ++i)
			pSubMenu->AppendMenu(MF_STRING, ID_VHM_MRU_BASE + i, m_pstrMRU[i]);
	}
}

void CVirtualHDDManagerDlg::OnVhmNew()
{
	if(!m_pVHMDisk)	CreateDisk();
}

void CVirtualHDDManagerDlg::OnVhmNewrd()
{
	if (m_pVHMDisk) return;

	CVHMDiskRam *pVHMDiskRam = new CVHMDiskRam;
	if (pVHMDiskRam->CreateDisk(SECTORSIZE, 1048576) == -1)
	{
		AfxMessageBox(_T("Fail"));
		delete pVHMDiskRam;
		return;
	}

	m_pVHMDisk = pVHMDiskRam;

	m_wndSectorView.SetCallbackFunc(CVirtualHDDManagerDlg::SVWCallbackProc, (VPARAM) this);
	m_wndSectorView.SetSectorSize(SECTORSIZE);
	m_wndSectorView.SetOffset(0);
}

void CVirtualHDDManagerDlg::OnVhmOpen()
{
	if (m_pVHMDisk)
		return;
	
	OpenDisk();
}

void CVirtualHDDManagerDlg::OnVhmClose()
{
	if (m_pVHMDisk) CloseDisk();
}

void CVirtualHDDManagerDlg::OnExit()
{
	OnOK();
}


void CVirtualHDDManagerDlg::OnVhmOpenMRU(UINT nID)
{
	if (m_pVHMDisk)
		return;

	if (!m_bMRULoaded)
		return;

	UINT uiIndex;

	uiIndex = nID - ID_VHM_MRU_BASE;
	if (uiIndex >= m_uiMRUEntryCount)
		return;

	OpenDisk(m_pstrMRU[uiIndex], TRUE);
}

void CVirtualHDDManagerDlg::OnGotoSector()
{
	if (!m_pVHMDisk)
		return;

	CGotoSectorDlg dlg;
	dlg.SetMinRange(0);
	dlg.SetMaxRange(m_pVHMDisk->GetSectorCount()-1);
	dlg.SetValue(m_wndSectorView.GetOffset());

	if (dlg.DoModal() == IDOK)
	{
		m_wndSectorView.SetOffset(dlg.GetValue());
	}
}

void CVirtualHDDManagerDlg::OnSelectPartiton()
{
	if (!m_pVHMDisk)
		return;

	CSelectItemDlg dlg;
	CString str;
	CVHMPartition *pVHMPartition;
	QWORD qwHandle;
	QWORD qwPartitionID;
	UINT i;
	LPARAM lParam;
	PARTITION_DESCRIPTOR pd;

	LPCTSTR ppszTypeString[3] = { _T("Volume Label"), _T("Starting LBA"), _T("Sector Count") };

	pVHMPartition = m_pVHMDisk->GetVHMPartition();
	if (!pVHMPartition) return;

	WCHAR pwszBuff[40];

	dlg.InitDataStorage(3);
	dlg.SetTypeString(0, ppszTypeString[0]);
	dlg.SetTypeString(1, ppszTypeString[1]);
	dlg.SetTypeString(2, ppszTypeString[2]);

	qwPartitionID = m_pVHMDisk->GetFirstPartitionID();
	for (i = 0; i < m_pVHMDisk->GetPartitionCount(); qwPartitionID = m_pVHMDisk->GetNextPartitionID(qwPartitionID), ++i)
	{
		if (pVHMPartition->GetPartitionDescriptor(qwPartitionID, &pd) != VHM_ERROR_SUCCESS)
			continue;
		
		// �ڵ� ����
		qwHandle = dlg.CreateData();

		// ��Ƽ�� ID ����
		dlg.SetUserParam(qwHandle, qwPartitionID);

		// ���� ���̺� ����
		if (pd.pVHMFilesystem)
		{
			pd.pVHMFilesystem->GetVolumeLabel(pwszBuff, 40);
			dlg.SetDataBuffer(qwHandle, 0, pwszBuff);
		}
		else
			dlg.SetDataBuffer(qwHandle, 0, _T("(Unrecognized)"));

		// ���� LBA ����
		str.Format(_T("%lld"), pd.StartLBA);
		dlg.SetDataBuffer(qwHandle, 1, str);
		
		// ���� �� ����
		str.Format(_T("%lld"), pd.SectorCountLBA);
		dlg.SetDataBuffer(qwHandle, 2, str);		
	}

	if (dlg.DoModal() != IDOK) return;
	lParam = dlg.GetSelectedUserParam();

	if (lParam == SID_INVALID_LPARAM)
		qwPartitionID = INVALID_PARTITION_ID;
	else
		qwPartitionID = (QWORD)lParam;

	m_qwPartitionID = qwPartitionID;
}

void CVirtualHDDManagerDlg::OnVbrWrite()
{
	if (!m_pVHMDisk) return;
	if (m_qwPartitionID == INVALID_PARTITION_ID) return;

	CFile fileVbr;
	BYTE *pVbr;
	DWORD dwSectorSize;
	CVHMPartition *pVHMPartition;
	CVHMFilesystem *pVHMFilesystem;
	int err_code;

	// ��Ƽ�ǿ� ���Ͻý����� �ε�Ǿ����� Ȯ��
	pVHMPartition = m_pVHMDisk->GetVHMPartition();
	if (!pVHMPartition) return;

	pVHMFilesystem = pVHMPartition->GetVHMFilesystem(m_qwPartitionID);
	if (!pVHMFilesystem) return;

	pVbr = nullptr;

	// ���� ����
	CFileDialog dlg(TRUE, _T("*.*"), nullptr, 0, _T("VBR(Volume Boot Record) File|*.*|"));
	if (dlg.DoModal() != IDOK) return;

	// ���� ����
	if (fileVbr.Open(dlg.GetOFN().lpstrFile, CFile::modeRead | CFile::shareDenyWrite) == FALSE)
	{
		AfxMessageBox(_T("Failed to open file."));
		return;
	}

	dwSectorSize = m_pVHMDisk->GetSectorSize();

	// ���� �˻�
	if (fileVbr.GetLength() < dwSectorSize)
	{
		AfxMessageBox(_T("File is too small."));
		goto cleanup;
	}

	// VBR �б�
	pVbr = new BYTE[dwSectorSize];
	fileVbr.Read(pVbr, dwSectorSize);

	// ���Ͻý����� ���� VBR ����
	err_code = pVHMFilesystem->WriteBootCode(pVbr);
	if (err_code != VHM_ERROR_SUCCESS)
	{
		AfxMessageBox(VHMErrorCodeToMessage(err_code));
	}

	goto cleanup;

cleanup:
	// ����
	if (pVbr) delete[] pVbr;
	fileVbr.Close();
}

void CVirtualHDDManagerDlg::OnScriptExecute()
{
	CVHMRoot vhmRoot;

	// ���� ����
	CFileDialog dlg(TRUE, _T("*.*"), nullptr, 0, _T("VHMScript File|*.vhmscript|All Files|*.*|"), this);
	if (dlg.DoModal() != IDOK) return;

	//CVHMScriptEngine vhmScriptEngine;
	//vhmScriptEngine.Create();
	//vhmScriptEngine.RunScript(dlg.GetOFN().lpstrFile);
	//vhmScriptEngine.Destroy();

    CVHMScript vhmScript;
    vhmScript.ExecuteScript(dlg.GetOFN().lpstrFile);
}

void CVirtualHDDManagerDlg::OnTest1()
{
#if 0
	/* CStream �迭 Class Test */
	const vhmsize_t stream_sz = 1024 * 1024 * 8;

	CStreamMemFixed smf;
	CStreamMemDyn smd;

	BYTE *test = (BYTE *)malloc(stream_sz);
	BYTE *smf_1 = (BYTE *)malloc(stream_sz);
	BYTE *smd_1 = (BYTE *)malloc(stream_sz);
	for (vhmsize_t i = 0; i < stream_sz; ++i)
		//test[i] = (BYTE)rand();
		test[i] = i * 16;

	smf.Create(stream_sz);
	smf.Write(test, stream_sz);
	smf.Seek(0, SSEEK_BEGIN);
	smf.Read(smf_1, stream_sz);

	smd.Create(stream_sz);
	for (vhmsize_t i = 0; i < stream_sz; i += STREAM_MEM_DYN_DEF_GRANULARITY)
		smd.Write(test + i, STREAM_MEM_DYN_DEF_GRANULARITY);
	smd.Seek(0, SSEEK_BEGIN);
	smd.Read(smd_1, stream_sz);

	for (vhmsize_t i = 0; i < stream_sz; ++i)
	{
		if (test[i] != smf_1[i])
			TRACE(_T("Error: smf: %d doesn't match\r\n"), i);
	}

	for (vhmsize_t i = 0; i < stream_sz; ++i)
	{
		if (test[i] != smd_1[i])
			TRACE(_T("Error: smd: %d doesn't match\r\n"), i);
	}

	free(test);
	free(smf_1);
	free(smd_1);
#endif

#if 0
	/* Stack Test */
	
	const vhmsize_t multiple_sz = 1024*1024;
	const vhmsize_t push_sz = 5;
	const vhmsize_t con_sz = push_sz * multiple_sz;
	CVHMScriptStack stack;
	stack.Create(con_sz);

	BYTE *test = (BYTE *)malloc(con_sz);
	BYTE *popped = (BYTE *)malloc(con_sz);
	srand(time(NULL));

	for (vhmsize_t i = 0; i < con_sz; ++i)
		test[i] = (BYTE)rand();

	for (vhmsize_t i = 0; i < con_sz; i += push_sz)
		stack.push(test + i, push_sz);

	for (vhmsize_t i = 0; i < con_sz; i += push_sz)
		stack.pop(popped + (con_sz - i - push_sz), push_sz);

	for (vhmsize_t i = 0; i < con_sz; ++i)
	{
		if (test[i] != popped[i])
			TRACE(_T("Error: %d doesn't match\r\n"), i);
	}

	free(test);
	free(popped);
#endif
}

void CVirtualHDDManagerDlg::OnAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

// VirtualHDDManagerDlg.h : ��� ����
//

#pragma once

#define VHM_MRU_NONE -1
#define VHM_MRU_COUNT_MAX 100

class CVHMDisk;

// CVirtualHDDManagerDlg ��ȭ ����
class CVirtualHDDManagerDlg : public CDialog
{
// �����Դϴ�.
public:
	CVirtualHDDManagerDlg(CWnd* pParent = nullptr);	// ǥ�� �������Դϴ�.
	virtual ~CVirtualHDDManagerDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

public:

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_VIRTUALHDDMANAGER_DIALOG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// ��ũ �Լ�
	static int SVWCallbackProc(UINT64 uiCommand, VPARAM vparam1, VPARAM vparam2, VPARAM vuserparam);
	int CreateDisk();
	int CreateDisk(LPCTSTR lpszPath, DWORD dwSectorSize, QWORD qwSectorCount, DWORD dwDiskType, QWORD qwFlags = 0);
	int OpenDisk();
	int OpenDisk(LPCTSTR lpszPath, BOOL bMRU = FALSE);
	int Read(UINT64 nStartSector, UINT64 nSectorCount, BYTE *pBuffer, UINT64 nBufferSize);
	int Write(UINT64 nStartSector, UINT64 nSectorCount, BYTE *pBuffer);
	CVHMDisk *GetVHMDiskClass();
	int CloseDisk();

protected:

	/* MRU ���� */
	// NOTE: �ֽ� �׸��ϼ��� Index�� ���� ������

	void MRUInitialize(); // MRU ��ƾ �ʱ�ȭ
	void MRUFinialize(); // MRU ��ƾ ����
	void MRULoad(); // MRU �ε�
	void MRUDeduplicate(); // MRU�� �ߺ� �׸� ����
	void MRUFlush(); // MRU�� ����
	void MRUSetMaxItemCount(UINT ccMax); // MRU �ִ� �׸� �� ����
	void MRUAdd(LPCTSTR lpszString); // MRU�� �׸� �߰�
	void MRUDelete(UINT uiIndex); // MRU���� �׸� ���� �� �������� �׸� ��ü �̵�
	UINT MRUCheckExistence(LPCTSTR lpszString, UINT uiIndexToStart = 0); // MRU�� �׸��� �ִ��� Ȯ�� (������ index ����, ������ VHM_MRU_NONE ����)
	BOOL MRUIsFull(); // MRU�� �� á���� TRUE
	void MRUUpdateMenu(); // MRU�� �޴��� ������Ʈ

	BOOL m_bMRULoaded;
	UINT m_uiMRUCount; // MRU �޸� �Ҵ� ��
	UINT m_uiMRUEntryCount; // ���� ���� �����ϴ� MRU �� (<= m_uiMRUCount)
	CString *m_pstrMRU;

	/* ��Ÿ */

	CVHMDisk *m_pVHMDisk;
	QWORD m_qwPartitionID; // ���õ� ��Ƽ���� ID
	CSectorViewWnd m_wndSectorView;

// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnVhmNew();
	afx_msg void OnVhmNewrd();
	afx_msg void OnVhmOpen();
	afx_msg void OnVhmClose();
	afx_msg void OnVhmOpenMRU(UINT nID);
	afx_msg void OnExit();
	afx_msg void OnGotoSector();
	afx_msg void OnSelectPartiton();
	afx_msg void OnVbrWrite();

	afx_msg void OnScriptExecute();
	afx_msg void OnTest1();
	afx_msg void OnAbout();
};

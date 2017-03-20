
// VirtualHDDManagerDlg.h : 헤더 파일
//

#pragma once

#define VHM_MRU_NONE -1
#define VHM_MRU_COUNT_MAX 100

class CVHMDisk;

// CVirtualHDDManagerDlg 대화 상자
class CVirtualHDDManagerDlg : public CDialog
{
// 생성입니다.
public:
	CVirtualHDDManagerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	virtual ~CVirtualHDDManagerDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

public:

// 대화 상자 데이터입니다.
	enum { IDD = IDD_VIRTUALHDDMANAGER_DIALOG };

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// 디스크 함수
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

	/* MRU 관리 */
	// NOTE: 최신 항목일수록 Index가 낮게 저장함

	void MRUInitialize(); // MRU 루틴 초기화
	void MRUFinialize(); // MRU 루틴 정리
	void MRULoad(); // MRU 로드
	void MRUDeduplicate(); // MRU의 중복 항목 제거
	void MRUFlush(); // MRU를 저장
	void MRUSetMaxItemCount(UINT ccMax); // MRU 최대 항목 수 지정
	void MRUAdd(LPCTSTR lpszString); // MRU에 항목 추가
	void MRUDelete(UINT uiIndex); // MRU에서 항목 제거 후 앞쪽으로 항목 전체 이동
	UINT MRUCheckExistence(LPCTSTR lpszString, UINT uiIndexToStart = 0); // MRU에 항목이 있는지 확인 (있으면 index 리턴, 없으면 VHM_MRU_NONE 리턴)
	BOOL MRUIsFull(); // MRU가 꽉 찼으면 TRUE
	void MRUUpdateMenu(); // MRU를 메뉴에 업데이트

	BOOL m_bMRULoaded;
	UINT m_uiMRUCount; // MRU 메모리 할당 수
	UINT m_uiMRUEntryCount; // 현재 실제 존재하는 MRU 수 (<= m_uiMRUCount)
	CString *m_pstrMRU;

	/* 기타 */

	CVHMDisk *m_pVHMDisk;
	QWORD m_qwPartitionID; // 선택된 파티션의 ID
	CSectorViewWnd m_wndSectorView;

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
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

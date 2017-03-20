
#pragma once

#define SID_INVALID_LPARAM		((LPARAM)-1)

// CSelectItemDlg 대화 상자입니다.

class CSelectItemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectItemDlg)

public:
	CSelectItemDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CSelectItemDlg();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_SELECT_ITEM };

	virtual INT_PTR DoModal();
	virtual BOOL OnInitDialog();

public:

	void InitDataStorage(UINT ccType); // 데이터 저장소 초기화
	void SetTypeString(UINT uiIndex, LPCTSTR lpszTypeString); // 종류 문자열 설정
	void DestroyDataStorage(); // 데이터 저장소 파괴

	QWORD CreateData(); // 데이터 추가
	void SetDataBuffer(QWORD qwHandle, UINT uiIndex, LPCTSTR lpszData); // 데이터 문자열 설정
	void SetUserParam(QWORD qwHandle, LPARAM lParam); // 사용자 매개변수 설정
	LPARAM GetUserParam(QWORD qwHandle); // 사용자 매개변수 얻기
	QWORD GetDataCount(); // 데이터 개수 얻기
	void RemoveAllData(); // 데이터 모두 제거

	LPARAM GetSelectedUserParam(); // 선택된 항목의 사용자 매개변수 얻기

protected:

	void SetData(QWORD qwHandle, UINT uiIndex, void *pData); // 데이터 설정
	void *GetData(QWORD qwHandle, UINT uiIndex); // 데이터 얻기

protected:

	BOOL m_bInit;
	UINT m_ccType;
	void *m_hHandleID;
	CString *m_pszTypeString;
	LPARAM m_lParamSelected;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
};

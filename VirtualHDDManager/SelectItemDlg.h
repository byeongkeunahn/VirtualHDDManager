
#pragma once

#define SID_INVALID_LPARAM		((LPARAM)-1)

// CSelectItemDlg ��ȭ �����Դϴ�.

class CSelectItemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSelectItemDlg)

public:
	CSelectItemDlg(CWnd* pParent = nullptr);   // ǥ�� �������Դϴ�.
	virtual ~CSelectItemDlg();

	// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SELECT_ITEM };

	virtual INT_PTR DoModal();
	virtual BOOL OnInitDialog();

public:

	void InitDataStorage(UINT ccType); // ������ ����� �ʱ�ȭ
	void SetTypeString(UINT uiIndex, LPCTSTR lpszTypeString); // ���� ���ڿ� ����
	void DestroyDataStorage(); // ������ ����� �ı�

	QWORD CreateData(); // ������ �߰�
	void SetDataBuffer(QWORD qwHandle, UINT uiIndex, LPCTSTR lpszData); // ������ ���ڿ� ����
	void SetUserParam(QWORD qwHandle, LPARAM lParam); // ����� �Ű����� ����
	LPARAM GetUserParam(QWORD qwHandle); // ����� �Ű����� ���
	QWORD GetDataCount(); // ������ ���� ���
	void RemoveAllData(); // ������ ��� ����

	LPARAM GetSelectedUserParam(); // ���õ� �׸��� ����� �Ű����� ���

protected:

	void SetData(QWORD qwHandle, UINT uiIndex, void *pData); // ������ ����
	void *GetData(QWORD qwHandle, UINT uiIndex); // ������ ���

protected:

	BOOL m_bInit;
	UINT m_ccType;
	void *m_hHandleID;
	CString *m_pszTypeString;
	LPARAM m_lParamSelected;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
};

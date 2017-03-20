
#pragma once

// CGotoSectorDlg ��ȭ �����Դϴ�.

class CGotoSectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CGotoSectorDlg)

public:
	CGotoSectorDlg(CWnd* pParent = nullptr);   // ǥ�� �������Դϴ�.
	virtual ~CGotoSectorDlg();

	// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_GOTO_SECTOR };

	virtual BOOL OnInitDialog();
	virtual INT_PTR DoModal();

public:
	void SetMinRange(UINT64 uiNumber);
	void SetMaxRange(UINT64 uiNumber);
	void SetValue(UINT64 uiNumber);
	UINT64 GetValue();

protected:
	UINT64 m_uiInit;
	UINT64 m_uiMinRange;
	UINT64 m_uiMaxRange;
	UINT64 m_uiValue;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP();
};

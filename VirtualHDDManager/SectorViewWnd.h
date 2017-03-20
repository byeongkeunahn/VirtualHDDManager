#pragma once


// CSectorViewWnd


#pragma once

typedef int (*CALLBACKFUNC_SVW)(UINT64 uiCommand, VPARAM vparam1, VPARAM vparam2, VPARAM vuserparam);

// SVW Message: Read, Write
// vparam1: Address of buffer
// vparam2: Address of data packet (POINTER to SVWM_DATA_PACKET structure)
typedef struct _svwm_data_packet
{
	UINT64	uiStartSector;
	UINT64	uiSectorCount;
	UINT64	uiSectorSize;

}SVWM_DATA_PACKET;
#define SVWM_READ 0x1
#define SVWM_WRITE 0x2

// SVW Message: Query
// vparam1: Address of query packet (POINTER to SVWM_QUERY_PACKET structure)
// vparam2: Unused
typedef struct _svwm_query_packet
{
	UINT64	uiFirstSector;
	UINT64	uiLastSector;

}SVWM_QUERY_PACKET;
#define SVWM_QUERY 0x3

class CSectorViewWnd : public CWnd
{
	DECLARE_DYNAMIC(CSectorViewWnd)

public:
	CSectorViewWnd();
	virtual ~CSectorViewWnd();

public:
	void SetOffset(UINT64 uiSector);
	UINT64 GetOffset();
	void SetSectorSize(UINT64 uiByteSize);
	UINT64 GetSectorSize();
	void SetCallbackFunc(CALLBACKFUNC_SVW pfnCallback, VPARAM vparam);
	static void CalculateAreaSize(UINT64 uiByteSize, SIZE *pSize);

	UINT64 GetFirstSector();
	UINT64 GetLastSector();

	void SetCaretPos(UINT64 uiCaretPos);
	UINT64 GetCaretPos();

	BOOL IsCurrentDataValid();
	void NotifyDiskClosed();

protected:
	// Internal functions
	int Read(BYTE *pBuffer, UINT64 uiStartSector, UINT64 uiSectorCount);
	int Write(BYTE *pBuffer, UINT64 uiStartSector, UINT64 uiSectorCount);
	int Move(BOOL bLeft);
	int MoveCaret(UINT nDirection);
	void SetData(BYTE *pData);

	int Save();
	int Discard();

	// Drawing functions
	void DrawBackground(CDC *pDC);
	void DrawOffset(CDC *pDC);
	void DrawData(CDC *pDC);
	void DrawDataASCII(CDC *pDC);
	void DrawButton(CDC *pDC);
	void DrawCaret(CDC *pDC);

protected:
	BYTE *m_pSectorData;
	UINT64 m_uiOffset;
	UINT64 m_uiByteSize; // Sector size
	BOOL m_bIsValidData; // TRUE if sector data is valid
	CALLBACKFUNC_SVW m_pfnCallback; // Callback function
	VPARAM m_vparamCallback; // User defined parameter (Callback function)
	SVWM_QUERY_PACKET m_pckDiskInfo; // Disk info packet data
	UINT64 m_uiCaretPos; // Caret Position

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};



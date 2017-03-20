// SectorViewWnd.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VirtualHDDManager.h"
#include "VHMCommon/VHMBase.h"
#include "SectorViewWnd.h"

// CSectorViewWnd

IMPLEMENT_DYNAMIC(CSectorViewWnd, CWnd)

CSectorViewWnd::CSectorViewWnd()
{
	m_bIsValidData = FALSE;
	m_pSectorData = nullptr;
	m_uiByteSize = 0;
	m_pfnCallback = nullptr;
	m_uiCaretPos = 0;
}

CSectorViewWnd::~CSectorViewWnd()
{
	if(m_pSectorData) free(m_pSectorData);
}


BEGIN_MESSAGE_MAP(CSectorViewWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
END_MESSAGE_MAP()

// CSectorViewWnd 메시지 처리기입니다.

void CSectorViewWnd::SetOffset(UINT64 uiSector)
{
	if(!m_pfnCallback || !m_uiByteSize)
		return;

	if(uiSector < GetFirstSector() || uiSector > GetLastSector())
		return;

	if(Read(m_pSectorData, uiSector, 1) == -1)
	{
		AfxMessageBox(_T("Callback Read() returned -1"));
		return;
	}

	CString str;
	str.Format(_T("New Offset: %016X"), uiSector);

	m_bIsValidData = TRUE;
	m_uiOffset = uiSector;

	Invalidate(TRUE);
}

UINT64 CSectorViewWnd::GetOffset()
{
	return m_uiOffset;
}

void CSectorViewWnd::SetData(BYTE *pData)
{
	if(!m_uiByteSize) return;

	if(pData != nullptr)
	{
		for(INT32 i=0; i<m_uiByteSize; i++) m_pSectorData[i] = pData[i];
	}
	else /* pData is nullptr */
	{
		SecureZeroMemory(m_pSectorData, m_uiByteSize);
	}

	Invalidate(TRUE);
}

void CSectorViewWnd::SetSectorSize(UINT64 uiByteSize)
{
	if(uiByteSize == 0 || uiByteSize == m_uiByteSize) return;

	if(m_pSectorData) delete[] m_pSectorData;
	m_pSectorData = new BYTE[uiByteSize];
	SecureZeroMemory(m_pSectorData, uiByteSize);

	m_uiOffset = 0;
	m_uiByteSize = uiByteSize;
	Invalidate(TRUE);
}

UINT64 CSectorViewWnd::GetSectorSize()
{
	return m_uiByteSize;
}

void CSectorViewWnd::SetCallbackFunc(CALLBACKFUNC_SVW pfnCallback, VPARAM vparam)
{
	if(!pfnCallback)
	{
		m_pfnCallback = nullptr;
		m_vparamCallback = NULL;
		m_uiOffset = 0;
		m_uiByteSize = 0;
		m_bIsValidData = FALSE;
		return;
	}

	m_pfnCallback = pfnCallback;
	m_vparamCallback = vparam;

	if((pfnCallback)(SVWM_QUERY, (VPARAM) &m_pckDiskInfo, 0, vparam) != VHM_ERROR_SUCCESS)
	{
		m_pfnCallback = nullptr;
		m_vparamCallback = NULL;
		return;
	}
}

void CSectorViewWnd::CalculateAreaSize(UINT64 uiByteSize, SIZE *pSize)
{
	INT64 x = 724;
	INT64 y = 16;
	if(uiByteSize > 0) y += ((uiByteSize - 1) / 16) * 16 + 16;
	y += 16;
	
	pSize->cx = (LONG)x;
	pSize->cy = (LONG)y;
}

UINT64 CSectorViewWnd::GetFirstSector()
{
	return m_pckDiskInfo.uiFirstSector;
}

UINT64 CSectorViewWnd::GetLastSector()
{
	return m_pckDiskInfo.uiLastSector;
}

void CSectorViewWnd::SetCaretPos(UINT64 uiCaretPos)
{
	if(!IsCurrentDataValid())
		return;

	if(uiCaretPos >= GetSectorSize()*2 && uiCaretPos != (UINT64)-1/*No caret*/)
		return;

	m_uiCaretPos = uiCaretPos;
	Invalidate(TRUE);
}

UINT64 CSectorViewWnd::GetCaretPos()
{
	return m_uiCaretPos;
}

BOOL CSectorViewWnd::IsCurrentDataValid()
{
	if(!m_bIsValidData || !m_pSectorData || !m_uiByteSize)
		return FALSE;

	return TRUE;
}

void CSectorViewWnd::NotifyDiskClosed()
{
	if (m_bIsValidData)
	{
		m_bIsValidData = FALSE;
		if (m_pSectorData)
		{
			delete[] m_pSectorData;
			m_pSectorData = nullptr;
		}
		m_uiByteSize = 0;
	}

	if (::IsWindow(GetSafeHwnd()))
		Invalidate(FALSE);
}

int CSectorViewWnd::Read(BYTE *pBuffer, UINT64 uiStartSector, UINT64 uiSectorCount)
{
	if(!m_pfnCallback)
		return -1;

	SVWM_DATA_PACKET pck;
	pck.uiStartSector = uiStartSector;
	pck.uiSectorCount = uiSectorCount;
	pck.uiSectorSize = GetSectorSize();

	return (m_pfnCallback)(SVWM_READ, (VPARAM) pBuffer, (VPARAM) &pck, m_vparamCallback);
}

int CSectorViewWnd::Write(BYTE *pBuffer, UINT64 uiStartSector, UINT64 uiSectorCount)
{
	if(!m_pfnCallback)
		return -1;

	SVWM_DATA_PACKET pck;
	pck.uiStartSector = uiStartSector;
	pck.uiSectorCount = uiSectorCount;

	return (m_pfnCallback)(SVWM_WRITE, (VPARAM) pBuffer, (VPARAM) &pck, m_vparamCallback);
}

int CSectorViewWnd::Move(BOOL bLeft)
{
	if(!m_bIsValidData || !m_pfnCallback)
		return -1;

	UINT64 uiNewOffset = GetOffset();

	if(bLeft)
	{
		if(uiNewOffset <= GetFirstSector()) return -1;
		uiNewOffset--;
	}
	else
	{
		if(uiNewOffset >= GetLastSector()) return -1;
		uiNewOffset++;
	}

	SetOffset(uiNewOffset);

	return 0;
}

int CSectorViewWnd::MoveCaret(UINT nDirection)
{
	if(!IsCurrentDataValid())
		return -1;

	INT64 iNewCaretPos = (INT64) GetCaretPos();

	switch(nDirection)
	{
	case VK_LEFT: iNewCaretPos--; break;
	case VK_RIGHT: iNewCaretPos++; break;
	case VK_UP: iNewCaretPos -= 32; break;
	case VK_DOWN: iNewCaretPos += 32; break;
	default:
		return -1;
	}

	if(iNewCaretPos < 0 || (UINT64)iNewCaretPos >= (GetSectorSize() * 2))
		return -1;

	SetCaretPos((UINT64)iNewCaretPos);

	return 0;
}

void CSectorViewWnd::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	CBitmap bitmap;
	CBitmap *pBmpOld;
	CDC dcMem;

	dcMem.CreateCompatibleDC(&dc);
	GetClientRect(&rc);
	bitmap.CreateCompatibleBitmap(&dc, rc.right+1, rc.bottom+1);
	pBmpOld = dcMem.SelectObject(&bitmap);
	
	dcMem.SelectStockObject(SYSTEM_FIXED_FONT);

	DrawBackground(&dcMem);
	DrawOffset(&dcMem);
	
	if(m_bIsValidData)
	{
		DrawButton(&dcMem);
		DrawCaret(&dcMem);
		DrawData(&dcMem);
		DrawDataASCII(&dcMem);
	}

	dc.BitBlt(0, 0, rc.right, rc.bottom, &dcMem, 0, 0, SRCCOPY);
	dcMem.SelectObject(pBmpOld);
}

void CSectorViewWnd::DrawBackground(CDC *pDC)
{
	CRect rc;
	GetClientRect(&rc);
	++rc.right;
	++rc.bottom;
	CGdiObject *pOldBrush = pDC->SelectStockObject(WHITE_BRUSH);
	CGdiObject *pOldPen = pDC->SelectStockObject(NULL_PEN);
	pDC->Rectangle(&rc);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CSectorViewWnd::DrawOffset(CDC *pDC)
{
	CString str;
	UINT64 i;
	pDC->SetBkMode(TRANSPARENT);

	if(m_uiByteSize == 0) return;

	UINT64 uiOffsetByte = m_uiOffset * GetSectorSize();

	for(i=0; i<(((m_uiByteSize-1)/16)+1); i++)
	{
		str.Format(_T("%016X"), uiOffsetByte + i*16);
		pDC->DrawText(str, CRect(CPoint(0, 16+(int)i*16), CSize(192,16)), DT_CENTER|DT_VCENTER);
	}

	for(i=0; i<16; i++)
	{
		str.Format(_T("%02X"), i);
		pDC->DrawText(str, CRect(CPoint(192+(int)i*24, 0), CSize(24, 16)), DT_CENTER|DT_VCENTER);
	}

	pDC->DrawText(_T("Offset"), CRect(0, 0, 192, 16), DT_CENTER|DT_VCENTER);

	pDC->SelectStockObject(BLACK_PEN);

	pDC->MoveTo(0, 16);
	pDC->LineTo(576, 16);

	pDC->MoveTo(192, 0);
	pDC->LineTo(192, (((m_uiByteSize-1)/16)+2)*16);

	pDC->MoveTo(384, 0);
	pDC->LineTo(384, (((m_uiByteSize-1)/16)+2)*16);
}

void CSectorViewWnd::DrawData(CDC *pDC)
{
	if(!m_bIsValidData) return;

	CString str;
	UINT64 i, j;
	pDC->SetBkMode(TRANSPARENT);

	for(i=0; i<(m_uiByteSize/16); i++)
	{
		for(j=0; j<16; j++)
		{
			str.Format(_T("%02X"), m_pSectorData[i*16+j]);
			pDC->DrawText(str, CRect(CPoint(192+(int)j*24, 16+i*16), CSize(24, 16)), DT_CENTER|DT_VCENTER);
		}
	}

	j = (m_uiByteSize / 16) * 16;

	for(i=0; i<(m_uiByteSize % 16); i++)
	{
		str.Format(_T("%02X"), m_pSectorData[i+j]);
		pDC->DrawText(str, CRect(CPoint(192+i*16+(int)j*24, 0), CSize(24, 16)), DT_CENTER|DT_VCENTER);
	}
}

#define BYTE2ASCII(c) ((((BYTE)(c))>=0x20 && (c)<=0x7F) ? ((BYTE)(c)) : _T('.'))

void CSectorViewWnd::DrawDataASCII(CDC *pDC)
{
	if (!m_bIsValidData) return;

	CString str;
	UINT64 i, j;
	pDC->SetBkMode(TRANSPARENT);

	for (i = 0; i<(m_uiByteSize / 16); i++)
	{
		for (j = 0; j<16; j++)
		{
			str.Format(_T("%c"), BYTE2ASCII(m_pSectorData[i * 16 + j]));
			pDC->DrawText(str, CRect(CPoint(588 + (int)j * 8, 16 + i * 16), CSize(8, 16)), DT_CENTER | DT_VCENTER);
		}
	}

	j = (m_uiByteSize / 16) * 16;

	for (i = 0; i<(m_uiByteSize % 16); i++)
	{
		str.Format(_T("%c"), BYTE2ASCII(m_pSectorData[i + j]));
		pDC->DrawText(str, CRect(CPoint(588 + i * 16 + (int)j * 8, 0), CSize(8, 16)), DT_CENTER | DT_VCENTER);
	}
}

void CSectorViewWnd::DrawButton(CDC *pDC)
{
	if(!m_bIsValidData) return;

	CGdiObject *pOldBrush = pDC->SelectStockObject(GRAY_BRUSH);
	CGdiObject *pOldPen = pDC->SelectStockObject(BLACK_PEN);	

	pDC->Rectangle(CRect(2, 2, 14, 14));
	pDC->Rectangle(CRect(18, 2, 30, 14));

	pDC->DrawText(_T("<"), CRect(2, 2, 14, 14), DT_CENTER|DT_VCENTER);
	pDC->DrawText(_T(">"), CRect(18, 2, 30, 14), DT_CENTER|DT_VCENTER);

	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CSectorViewWnd::DrawCaret(CDC *pDC)
{
	UINT64 uiCaretPos = GetCaretPos();
	if(uiCaretPos == (UINT64)-1)
		return;

	UINT64 x = uiCaretPos % 32;
	UINT64 y = uiCaretPos / 32;

	CPoint point;
	point.x = 192 + x*12;
	point.y = 16 + y*16 + 1;

	if(x % 2 == 0)
		point.x += 3;

	if(y == 0)
		point.y++;
	
	CBrush brush;
	brush.CreateSolidBrush(RGB(0,162,232));
	
	CBrush *pOldBrush = pDC->SelectObject(&brush);
	CGdiObject *pOldPen = pDC->SelectStockObject(NULL_PEN);

	CRect rc(point, CSize(9, (y == 0) ? 13 : 14));

	pDC->Rectangle(rc);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

void CSectorViewWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	static CRect rcLeft(2, 2, 14, 14);
	static CRect rcRight(18, 2, 30, 14);

	if(m_bIsValidData)
	{
		if(rcLeft.PtInRect(point)) Move(TRUE);
		else if(rcRight.PtInRect(point)) Move(FALSE);
	}

	SetFocus();

	CWnd::OnLButtonUp(nFlags, point);
}

BOOL CSectorViewWnd::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		MoveCaret((UINT) pMsg->wParam);
		SetFocus();
	}
	if(pMsg->message == WM_KEYUP)
		SetFocus();

	return CWnd::PreTranslateMessage(pMsg);
}

void CSectorViewWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(!IsCurrentDataValid())
		goto final;

	BYTE bChar = 0xFF;
	if(nChar >= 'A' && nChar <= 'F')
	{
		bChar = nChar + 0x0A - 'A';
	}
	else if(nChar >= '0' && nChar <= '9')
	{
		bChar = nChar - '0';
	}

	UINT64 ui;

	if(bChar != 0xFF)
	{
		UINT64 uiCaretPos = GetCaretPos();
		if((uiCaretPos % 2) == 0)
		{
			ui = uiCaretPos / 2;
			m_pSectorData[uiCaretPos / 2] &= 0x0F;
			m_pSectorData[uiCaretPos / 2] |= (bChar & 0x0F) << 4;
		}
		else if((uiCaretPos % 2) == 1)
		{
			ui = uiCaretPos / 2;
			m_pSectorData[uiCaretPos / 2] &= 0xF0;
			m_pSectorData[uiCaretPos / 2] |= (bChar & 0x0F);
		}
		MoveCaret(VK_RIGHT);
		Invalidate(TRUE);
		TRACE(_T("uiCaretPos = %d, bChar = %02X\r\n"), ui, bChar);
	}


final:
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CSectorViewWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	SetFocus();
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
	SetFocus();
}

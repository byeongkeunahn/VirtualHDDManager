
#include "stdafx.h"
#include "VHMBase.h"
#include "VHMUtility.h"
#include "Stream.h"
#include "StreamMemFixed.h"

CStreamMemFixed::CStreamMemFixed()
{

}

CStreamMemFixed::~CStreamMemFixed()
{

}

int CStreamMemFixed::Create(vhmsize_t szStream)
{
	if (IsOpened())
		return VHM_ERROR_ALREADY_OPENED;
	
	BYTE *pBuffer = (BYTE *)malloc(szStream);
	if (!pBuffer)
		return VHM_ERROR_MEMORY_ALLOCATION_FAILED;

	m_uiPointer = 0;
	m_pBuffer = pBuffer;
	m_szBuffer = szStream;

	SetOpenState(TRUE);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemFixed::Close()
{
	if (!IsOpened())
		return VHM_ERROR_SUCCESS;

	free(m_pBuffer);
	SetOpenState(FALSE);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemFixed::Read(void *pBuffer, vhmsize_t szBufRead)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if (!szBufRead)
		return VHM_ERROR_INVALID_PARAMETER;

	// 마지막 바이트가 버퍼를 넘어가면 오류
	vhmsize_t szLastByteToRead;
	szLastByteToRead = m_uiPointer + szBufRead;
	if (szLastByteToRead < m_uiPointer || szLastByteToRead > m_szBuffer)
		return VHM_ERROR_INVALID_PARAMETER;

	// 데이터 복사
	CopyMemoryBlock(pBuffer, m_pBuffer + m_uiPointer, szBufRead);
	
	// 포인터 이동
	Seek(szBufRead, SSEEK_CUR);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemFixed::Write(const void *pBuffer, vhmsize_t szBufWrite)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if (!szBufWrite)
		return VHM_ERROR_INVALID_PARAMETER;

	// 마지막 바이트가 버퍼를 넘어가면 오류
	vhmsize_t szLastByteToWrite;
	szLastByteToWrite = m_uiPointer + szBufWrite;
	if (szLastByteToWrite < m_uiPointer || szLastByteToWrite > m_szBuffer)
		return VHM_ERROR_INVALID_PARAMETER;

	// 데이터 복사
	CopyMemoryBlock(m_pBuffer + m_uiPointer, pBuffer, szBufWrite);

	// 포인터 이동
	Seek(szBufWrite, SSEEK_CUR);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemFixed::Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	vhmsize_t uiNewPointer = m_uiPointer;
	switch (nMoveMethod)
	{
	case SSEEK_BEGIN:
		uiNewPointer = iOff;
		break;
	case SSEEK_CUR:
		uiNewPointer += (vhmsize_t)iOff;
		break;
	case SSEEK_END:
		uiNewPointer = m_szBuffer - iOff;
		break;
	default:
		return VHM_ERROR_INVALID_PARAMETER;
	}

	if (uiNewPointer > m_szBuffer)
		return VHM_ERROR_INVALID_PARAMETER;

	m_uiPointer = uiNewPointer;
	if (pNewPointer != nullptr) *pNewPointer = m_uiPointer;

	return VHM_ERROR_SUCCESS;
}

UINT64 CStreamMemFixed::Tell()
{
	if (!IsOpened())
		return VHM_INVALID_VALUE;

	return m_uiPointer;
}

UINT64 CStreamMemFixed::GetSize()
{
	if (!IsOpened())
		return VHM_INVALID_VALUE;

	return m_szBuffer;
}

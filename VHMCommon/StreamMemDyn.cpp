
#include "stdafx.h"
#include "VHMBase.h"
#include "VHMUtility.h"
#include "Stream.h"
#include "StreamMemDyn.h"

CStreamMemDyn::CStreamMemDyn()
{

}

CStreamMemDyn::~CStreamMemDyn()
{

}

int CStreamMemDyn::Create(vhmsize_t szMax, vhmsize_t szGranularity)
{
	if (IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	if (!szGranularity)
		szGranularity = STREAM_MEM_DYN_DEF_GRANULARITY;

	m_uiPointer = 0;
	m_pBuffer = NULL;
	m_szBuffer = 0;
	m_szBufferMax = szMax;
	m_szGranularity = szGranularity;

	SetOpenState(TRUE);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemDyn::Close()
{
	if (!IsOpened())
		return VHM_ERROR_SUCCESS;

	free(m_pBuffer);
	SetOpenState(FALSE);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemDyn::Read(void *pBuffer, vhmsize_t szBufRead)
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

int CStreamMemDyn::Write(const void *pBuffer, vhmsize_t szBufWrite)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if (!szBufWrite)
		return VHM_ERROR_INVALID_PARAMETER;

	// 마지막 바이트가 버퍼를 넘어가면 오류
	vhmsize_t szLastByteToWrite;
	szLastByteToWrite = m_uiPointer + szBufWrite;
	if (m_szBufferMax && (szLastByteToWrite < m_uiPointer || szLastByteToWrite > m_szBufferMax))
		return VHM_ERROR_INVALID_PARAMETER;

	if (szLastByteToWrite > m_szBuffer)
		if (AdjustBufferSize(szLastByteToWrite) != VHM_ERROR_SUCCESS)
			return VHM_ERROR_MEMORY_ALLOCATION_FAILED;

	// 데이터 복사
	CopyMemoryBlock(m_pBuffer + m_uiPointer, pBuffer, szBufWrite);

	// 포인터 이동
	Seek(szBufWrite, SSEEK_CUR);

	return VHM_ERROR_SUCCESS;
}

int CStreamMemDyn::Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer)
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

	if (m_szBufferMax && (uiNewPointer > m_szBufferMax))
		return VHM_ERROR_INVALID_PARAMETER;

	if (uiNewPointer > m_szBuffer)
		if (AdjustBufferSize(uiNewPointer) != VHM_ERROR_SUCCESS)
			return VHM_ERROR_MEMORY_ALLOCATION_FAILED;

	m_uiPointer = uiNewPointer;
	if (pNewPointer != nullptr) *pNewPointer = m_uiPointer;

	return VHM_ERROR_SUCCESS;
}

int CStreamMemDyn::AdjustBufferSize(vhmsize_t szNew)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if (szNew == m_szBuffer)
		return VHM_ERROR_SUCCESS;

	if (m_szBufferMax && szNew > m_szBufferMax)
		return VHM_ERROR_INVALID_PARAMETER;

	if (!szNew)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}
	else
	{
		szNew = RoundUp(szNew, m_szGranularity);
		if (m_szBufferMax > 0) szNew = min(szNew, m_szBufferMax);

		BYTE *pBufferNew = (BYTE *)realloc(m_pBuffer, szNew);
		if (!pBufferNew)
			return VHM_ERROR_MEMORY_ALLOCATION_FAILED;

		m_pBuffer = pBufferNew;
	}

	m_szBuffer = szNew;
	return VHM_ERROR_SUCCESS;
}

UINT64 CStreamMemDyn::Tell()
{
	if (!IsOpened())
		return VHM_INVALID_VALUE;

	return m_uiPointer;
}

UINT64 CStreamMemDyn::GetSize()
{
	if (!IsOpened())
		return VHM_INVALID_VALUE;

	return m_szBuffer;
}

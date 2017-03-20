
#include "stdafx.h"
#include "VHMBase.h"
#include "Stream.h"

CStream::CStream()
{
	SetOpenState(FALSE);
	m_uiFlags = 0;
	m_uiSFlags = 0;
}

CStream::~CStream()
{
	if (IsOpened())	Close();
}

int CStream::Close()
{
	return VHM_ERROR_UNSUPPORTED;
}

int CStream::IsOpened()
{
	return m_bOpened;
}

int CStream::SetOpenState(BOOL bOpened)
{
	m_bOpened = bOpened;
	return VHM_ERROR_SUCCESS;
}

int CStream::GetFlags(UINT32 *puiFlags)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if (!puiFlags)
		return VHM_ERROR_INVALID_PARAMETER;

	*puiFlags = m_uiFlags;
	return VHM_ERROR_SUCCESS;
}

int CStream::SetFlags(UINT32 uiFlags)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	m_uiFlags = uiFlags;
	return VHM_ERROR_SUCCESS;
}

int CStream::GetSFlags(UINT32 *puiSFlags)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if (!puiSFlags)
		return VHM_ERROR_INVALID_PARAMETER;

	*puiSFlags = m_uiSFlags;
	return VHM_ERROR_SUCCESS;
}

int CStream::SetSFlags(UINT32 uiSFlags)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	m_uiSFlags = uiSFlags;
	return VHM_ERROR_SUCCESS;
}

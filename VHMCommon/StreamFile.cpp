
#include "stdafx.h"
#include "VHMBase.h"
#include "Stream.h"
#include "StreamFile.h"

CStreamFile::CStreamFile()
{

}

CStreamFile::~CStreamFile()
{

}

int CStreamFile::Create(LPCTSTR lpszFilePath)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CStreamFile::Close()
{
	return VHM_ERROR_UNSUPPORTED;
}

int CStreamFile::Read(void *pBuffer, vhmsize_t szBufRead)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CStreamFile::Write(const void *pBuffer, vhmsize_t szBufWrite)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CStreamFile::Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer)
{
	return VHM_ERROR_UNSUPPORTED;
}

UINT64 CStreamFile::Tell()
{
	return VHM_INVALID_VALUE;
}

UINT64 CStreamFile::GetSize()
{
	return VHM_INVALID_VALUE;
}

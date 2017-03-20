
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMDiskVDI.h"

CVHMDiskVDI::CVHMDiskVDI(void)
{

}

CVHMDiskVDI::~CVHMDiskVDI(void)
{

}

int CVHMDiskVDI::CreateDisk(const wchar_t *wszFilePath, DWORD dwSectorSize, QWORD qwSectorCount)
{
	return VHM_ERROR_INTERNAL;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	if(dwSectorSize < 512 || !IsPowerOfTwo(dwSectorSize))
		return VHM_ERROR_INVALID_PARAMETER;

	// 섹터 수는 0이 아니어야 한다.
	if(qwSectorCount == 0)
		return VHM_ERROR_INVALID_PARAMETER;

	// wszFilePath에 주어진 매개변수로 VDI 파일을 생성한다.
	if(m_fileVDI.Open(wszFilePath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
	{
		return VHM_ERROR_DISK_IO;
	}

	// 멤버 변수 설정
	m_bOpened = TRUE;
	m_dwSectorSize = dwSectorSize;
	m_qwSectorCount = qwSectorCount;

	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVDI::OpenDisk(const wchar_t *wszFilePath)
{
	return VHM_ERROR_INTERNAL;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	// wszFilePath에 주어진 매개변수로 VDI 파일을 연다.
	if(m_fileVDI.Open(wszFilePath, CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
		return VHM_ERROR_DISK_IO;

	// 헤더를 확인한다.

	// 멤버 변수 설정
	m_bOpened = TRUE;
	
	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVDI::CloseDisk()
{
	return VHM_ERROR_INTERNAL;

	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UninitializeVHMPartition();
	UninitializeVHMIOWrapper();

	m_fileVDI.Close();
	m_bOpened = FALSE;

	m_dwSectorSize = 0;
	m_qwSectorCount = 0;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVDI::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	return ReadSector(uiSectorLBA, 1, pBuffer, szBuffer);
}

int CVHMDiskVDI::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	return WriteSector(uiSectorLBA, 1, pBuffer);
}

int CVHMDiskVDI::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	return VHM_ERROR_INTERNAL;
}

int CVHMDiskVDI::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	return VHM_ERROR_INTERNAL;
}

DWORD CVHMDiskVDI::GetDiskType()
{
	if(!IsOpened())
		return VIRTUAL_DISK_FORMAT_INVALID;

	return VIRTUAL_DISK_FORMAT_VDI;
}

#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMDiskVHDX.h"

CVHMDiskVHDX::CVHMDiskVHDX(void)
{

}

CVHMDiskVHDX::~CVHMDiskVHDX(void)
{

}

int CVHMDiskVHDX::CreateDisk(const wchar_t *wszFilePath, DWORD dwSectorSize, QWORD qwSectorCount)
{
	return VHM_ERROR_INTERNAL;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	if(dwSectorSize < 512 || !IsPowerOfTwo(dwSectorSize))
		return VHM_ERROR_INVALID_PARAMETER;

	// ���� ���� 0�� �ƴϾ�� �Ѵ�.
	if(qwSectorCount == 0)
		return VHM_ERROR_INVALID_PARAMETER;

	// wszFilePath�� �־��� �Ű������� VHDX ������ �����Ѵ�.
	if(m_fileVHDX.Open(wszFilePath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
	{
		return VHM_ERROR_DISK_IO;
	}

	// ��� ���� ����
	m_bOpened = TRUE;
	m_dwSectorSize = dwSectorSize;
	m_qwSectorCount = qwSectorCount;

	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHDX::OpenDisk(const wchar_t *wszFilePath)
{
	return VHM_ERROR_INTERNAL;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	// wszFilePath�� �־��� �Ű������� VHDX ������ ����.
	if(m_fileVHDX.Open(wszFilePath, CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
		return VHM_ERROR_DISK_IO;

	// ����� Ȯ���Ѵ�.

	// ��� ���� ����
	m_bOpened = TRUE;
	
	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHDX::CloseDisk()
{
	return VHM_ERROR_INTERNAL;

	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UninitializeVHMPartition();
	UninitializeVHMIOWrapper();

	m_fileVHDX.Close();
	m_bOpened = FALSE;

	m_dwSectorSize = 0;
	m_qwSectorCount = 0;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHDX::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	return ReadSector(uiSectorLBA, 1, pBuffer, szBuffer);
}

int CVHMDiskVHDX::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	return WriteSector(uiSectorLBA, 1, pBuffer);
}

int CVHMDiskVHDX::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	return VHM_ERROR_INTERNAL;
}

int CVHMDiskVHDX::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	return VHM_ERROR_INTERNAL;
}

DWORD CVHMDiskVHDX::GetDiskType()
{
	if(!IsOpened())
		return VIRTUAL_DISK_FORMAT_INVALID;

	return VIRTUAL_DISK_FORMAT_VHDX;
}
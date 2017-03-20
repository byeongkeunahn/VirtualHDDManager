
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMDiskRam.h"
#include "VHMCommon/VHMUtility.h"

CVHMDiskRam::CVHMDiskRam(void)
{
	m_pDiskData = nullptr;
}

CVHMDiskRam::~CVHMDiskRam(void)
{
	CloseDisk();
}

int CVHMDiskRam::CreateDisk(DWORD dwSectorSize, QWORD qwSectorCount)
{
	if(IsOpened())
		return -1;

	if(dwSectorSize < 512 || !IsPowerOfTwo(dwSectorSize))
		return -1;

	if(!qwSectorCount)
		return -1;

	QWORD qwDiskSizeInBytes;
	qwDiskSizeInBytes = dwSectorSize * qwSectorCount;
	if(qwDiskSizeInBytes >= TwoExp32)
		return -1;

	BYTE *pMemory;
	if((pMemory = (BYTE *) malloc(qwDiskSizeInBytes)) == nullptr)
		return -1;

	m_dwSectorSize = dwSectorSize;
	m_qwSectorCount = qwSectorCount;
	m_pDiskData = pMemory;
	m_bOpened = TRUE;

	if(InitializeVHMIOWrapper() != VHM_ERROR_SUCCESS || 
		InitializeVHMPartition() != VHM_ERROR_SUCCESS)
	{
		UninitializeVHMPartition();
		UninitializeVHMIOWrapper();

		m_bOpened = FALSE;
		m_dwSectorSize = 0;
		m_qwSectorCount = 0;
		m_pVHMPartition = nullptr;
		m_pDiskData = nullptr;

		free(pMemory);

		return -1;
	}
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskRam::CloseDisk()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UninitializeVHMPartition();
	UninitializeVHMIOWrapper();

	free(m_pDiskData);
	m_pDiskData = nullptr;
	
	m_bOpened = FALSE;

	m_dwSectorSize = 0;
	m_qwSectorCount = 0;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskRam::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	return ReadSector(uiSectorLBA, 1, pBuffer, szBuffer);
}

int CVHMDiskRam::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	return WriteSector(uiSectorLBA, 1, pBuffer);
}

int CVHMDiskRam::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;
	
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	UINT64 u64BytesToRead = GetSectorSize() * uiSectorCount;

	if(szBuffer < u64BytesToRead)
		return -1;

	UINT64 uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if(uiLastSector >= GetSectorCount())
		return -1;

	CopyMemoryBlock(pBuffer, m_pDiskData + uiStartingLBA * GetSectorSize(), u64BytesToRead);
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskRam::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{	
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	UINT64 u64BytesToWrite = GetSectorSize() * uiSectorCount;

	UINT64 uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if(uiLastSector >= GetSectorSize())
		return -1;

	CopyMemoryBlock(m_pDiskData + uiStartingLBA * GetSectorSize(), pBuffer, u64BytesToWrite);
	
	return VHM_ERROR_SUCCESS;
}

DWORD CVHMDiskRam::GetDiskType()
{
	if(!IsOpened())
		return VIRTUAL_DISK_FORMAT_INVALID;

	return VIRTUAL_DISK_FORMAT_RAM;
}

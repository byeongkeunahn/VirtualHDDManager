
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMIOWrapper.h"
#include "VHMPartition.h"
#include "VHMDisk.h"

CVHMDisk::CVHMDisk(void)
{
	m_bOpened = FALSE;
	m_dwSectorSize = 0;
	m_qwSectorCount = 0;
	m_pVHMIOWrapper = nullptr;
	m_pVHMPartition = nullptr;
}

CVHMDisk::~CVHMDisk(void)
{
	UninitializeVHMPartition();
	UninitializeVHMIOWrapper();
	CloseDisk();
}

int CVHMDisk::OpenDisk()
{
	return -1;
}

int CVHMDisk::CloseDisk()
{
	return -1;
}

int CVHMDisk::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	return VHM_ERROR_INTERNAL;
}

int CVHMDisk::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	return VHM_ERROR_INTERNAL;
}

int CVHMDisk::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	DWORD dwSectorSize = GetSectorSize();

	/* 섹터 개수는 0이 될 수 없음 */
	if(uiSectorCount <= 0)
		return VHM_ERROR_INVALID_PARAMETER;

	/* 마지막 섹터가 디스크의 마지막 LBA를 넘어가면 오류 */
	UINT64 uiLastLBAToRead;
	uiLastLBAToRead = uiStartingLBA + uiSectorCount - 1;
	if(uiLastLBAToRead >= GetSectorCount())
		return VHM_ERROR_INVALID_PARAMETER;

	/* 섹터 1개 읽기 함수를 uiSectorCount 번 호출 */
	BYTE *pBufferTemp = (BYTE *)pBuffer;
	UINT64 i;
	for(i = uiStartingLBA; i <= uiLastLBAToRead; ++i)
	{
		if(ReadOneSector(i, pBufferTemp, dwSectorSize) != VHM_ERROR_SUCCESS)
			return VHM_ERROR_DISK_IO;

		pBufferTemp += dwSectorSize;
	}

	return VHM_ERROR_SUCCESS;
}

int CVHMDisk::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	DWORD dwSectorSize = GetSectorSize();

	/* 섹터 개수는 0이 될 수 없음 */
	if(uiSectorCount <= 0)
		return VHM_ERROR_INVALID_PARAMETER;

	/* 마지막 섹터가 디스크의 마지막 LBA를 넘어가면 오류 */
	UINT64 uiLastLBAToWrite;
	uiLastLBAToWrite = uiStartingLBA + uiSectorCount - 1;
	if(uiLastLBAToWrite >= GetSectorCount())
		return VHM_ERROR_INVALID_PARAMETER;

	/* 섹터 1개 읽기 함수를 uiSectorCount 번 호출 */
	BYTE *pBufferTemp = (BYTE *)pBuffer;
	UINT64 i;
	for(i = uiStartingLBA; i <= uiLastLBAToWrite; ++i)
	{
		if(WriteOneSector(i, pBufferTemp) != VHM_ERROR_SUCCESS)
			return VHM_ERROR_DISK_IO;

		pBufferTemp += dwSectorSize;
	}

	return VHM_ERROR_SUCCESS;
}

int CVHMDisk::WipeSector(UINT64 uiStartSector, UINT64 uiSectorCount)
{
	if(!IsOpened())
		return -1;

	UINT64 i;
	UINT32 uBuffer = 2^24; // 8MB

	BYTE *pNullMem = (BYTE *) malloc(uBuffer);
	memset(pNullMem, 0, uBuffer);

	for(i=0; i<(uiSectorCount / uBuffer); i++)
	{
		WriteSector(uiStartSector + i * uBuffer, uBuffer, pNullMem);
	}

	WriteSector(uiStartSector + (uiSectorCount / uBuffer) * uBuffer, uiSectorCount % uBuffer, pNullMem);

	free(pNullMem);
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDisk::WipeDisk()
{
	if(!IsOpened())
		return -1;
	
	return WipeSector(0, m_qwSectorCount);
}

BOOL CVHMDisk::IsOpened()
{
	return m_bOpened;
}

DWORD CVHMDisk::GetSectorSize()
{
	if(!IsOpened())
		return (DWORD) -1;

	return m_dwSectorSize;
}

QWORD CVHMDisk::GetSectorCount()
{
	if(!IsOpened())
		return (DWORD) -1;

	return m_qwSectorCount;
}

DWORD CVHMDisk::GetPartitioningType()
{
	if (!IsOpened() || !m_pVHMPartition)
		return -1;

	return m_pVHMPartition->GetPartitioningType();
}

UINT64 CVHMDisk::GetPartitionCount()
{
	if (!IsOpened() || !m_pVHMPartition)
		return -1;

	return m_pVHMPartition->GetPartitionCount();
}

QWORD CVHMDisk::GetFirstPartitionID()
{
	if (!IsOpened() || !m_pVHMPartition)
		return -1;

	return m_pVHMPartition->GetFirstPartitionID();
}

QWORD CVHMDisk::GetPrevPartitionID(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_pVHMPartition)
		return -1;

	return m_pVHMPartition->GetPrevPartitionID(qwPartitionID);
}

QWORD CVHMDisk::GetNextPartitionID(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_pVHMPartition)
		return -1;

	return m_pVHMPartition->GetNextPartitionID(qwPartitionID);
}

QWORD CVHMDisk::GetLastPartitionID()
{
	if (!IsOpened() || !m_pVHMPartition)
		return -1;

	return m_pVHMPartition->GetLastPartitionID();
}

DWORD CVHMDisk::GetDiskType()
{
	return VIRTUAL_DISK_FORMAT_INVALID;
}

CVHMIOWrapper *CVHMDisk::GetVHMIOWrapper()
{
	if(!IsOpened())
		return nullptr;

	return m_pVHMIOWrapper;
}

CVHMPartition *CVHMDisk::GetVHMPartition()
{
	if (!IsOpened())
		return nullptr;

	return m_pVHMPartition;
}

int CVHMDisk::InitializeVHMPartition()
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if(m_pVHMPartition != nullptr)
		return VHM_ERROR_SUCCESS;

	if(m_pVHMIOWrapper == nullptr)
		return VHM_ERROR_INTERNAL;

	CVHMPartition *pVHMPartition = new CVHMPartition;
	if(pVHMPartition->Create(m_pVHMIOWrapper) != VHM_ERROR_SUCCESS)
	{
		delete pVHMPartition;
		return VHM_ERROR_INTERNAL;
	}

	m_pVHMPartition = pVHMPartition;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDisk::UninitializeVHMPartition()
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if(m_pVHMPartition == nullptr)
		return VHM_ERROR_SUCCESS;

	if(m_pVHMPartition->Destroy() != VHM_ERROR_SUCCESS)
		return VHM_ERROR_INTERNAL;

	delete m_pVHMPartition;
	m_pVHMPartition = nullptr;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDisk::InitializeVHMIOWrapper()
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if(m_pVHMIOWrapper != nullptr)
		return VHM_ERROR_SUCCESS;

	CVHMIOWrapper *pVHMIOWrapper = new CVHMIOWrapper;
	if(pVHMIOWrapper->Initialize(this, 0, GetSectorCount()) != VHM_ERROR_SUCCESS)
		return VHM_ERROR_INTERNAL;

	m_pVHMIOWrapper = pVHMIOWrapper;
	return VHM_ERROR_SUCCESS;
}

int CVHMDisk::UninitializeVHMIOWrapper()
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	if(m_pVHMIOWrapper == nullptr)
		return VHM_ERROR_SUCCESS;

	if(m_pVHMPartition != nullptr)
		return VHM_ERROR_INTERNAL;

	if(m_pVHMIOWrapper->Uninitialize() != VHM_ERROR_SUCCESS)
		return VHM_ERROR_INTERNAL;

	delete m_pVHMIOWrapper;
	m_pVHMIOWrapper = nullptr;

	return VHM_ERROR_SUCCESS;
}

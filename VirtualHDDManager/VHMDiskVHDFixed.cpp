
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMDiskVHDSpec.h"
#include "VHMDiskVHD.h"

int CVHMDiskVHD::FixedCreateDisk(CFile *pFile, DWORD dwSectorSize, QWORD qwSectorCount)
{
	return VHM_ERROR_UNSUPPORTED;	
}

int CVHMDiskVHD::FixedOpenDisk(CFile *pFile, VHD_FOOTER *vhd_footer)
{
	if (!pFile || !vhd_footer)
		return VHM_ERROR_INVALID_PARAMETER;

	UINT64 uiDiskSize;

	if (vhd_footer->major_ver > VHD_MAJOR_VER)
		return VHM_ERROR_UNSUPPORTED;

	uiDiskSize = pFile->GetLength();
	if (uiDiskSize % VHD_SECTOR_SZ == 0)
		uiDiskSize -= VHD_SECTOR_SZ;
	else
		uiDiskSize -= VHD_SECTOR_SZ - 1;

	if (vhd_footer->current_sz != uiDiskSize)
		return VHM_ERROR_UNSUPPORTED;

	// 디스크 정보 설정
	m_dwDiskType = VHD_TYPE_FIXED;
	m_dwSectorSize = VHD_SECTOR_SZ;
	m_qwSectorCount = uiDiskSize / VHD_SECTOR_SZ;
	m_pData = malloc(sizeof(VHD_FOOTER));
	CopyMemoryBlock(m_pData, vhd_footer, sizeof(VHD_FOOTER));

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::FixedCloseDisk()
{
	free(m_pData);
	m_pData = nullptr;

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::FixedReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	return FixedReadSector(uiSectorLBA, 1, pBuffer, szBuffer);
}

int CVHMDiskVHD::FixedWriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	return FixedWriteSector(uiSectorLBA, 1, pBuffer);
}

int CVHMDiskVHD::FixedReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	UINT64 u64BytesToRead = GetSectorSize() * uiSectorCount;
	if (u64BytesToRead >= TwoExp32)
		return -1;

	UINT32 uBytesToRead = (UINT32)u64BytesToRead;

	if (szBuffer < uBytesToRead)
		return -1;

	UINT64 uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if (uiLastSector >= GetSectorCount())
		return -1;

	m_fileVHD.Seek(uiStartingLBA * GetSectorSize(), CFile::begin);
	if (m_fileVHD.Read(pBuffer, uBytesToRead) != uBytesToRead)
	{
		AfxMessageBox(_T("CFile::Read() returned -1"));
		return -1;
	}

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::FixedWriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	UINT64 u64BytesToWrite = GetSectorSize() * uiSectorCount;
	if (u64BytesToWrite >= TwoExp32)
		return -1;

	UINT64 uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if (uiLastSector >= GetSectorSize())
		return -1;

	UINT32 uBytesToWrite = (UINT32)u64BytesToWrite;

	m_fileVHD.Seek(uiStartingLBA * GetSectorSize(), CFile::begin);
	m_fileVHD.Write(pBuffer, uBytesToWrite);

	return VHM_ERROR_SUCCESS;
}
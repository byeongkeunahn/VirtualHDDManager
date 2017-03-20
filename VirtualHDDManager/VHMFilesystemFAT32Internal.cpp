
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMIOWrapper.h"
#include "VHMDisk.h"
#include "VHMPartition.h"
#include "VHMCommon/HandleManagement.h"
#include "VHMFilesystem.h"
#include "VHMFilesystemFATStructure.h"
#include "VHMFilesystemFATCommon.h"
#include "VHMFilesystemFAT32Internal.h"
#include "VHMFilesystemFAT32.h"

DWORD CVHMFilesystemFAT32::GetSecPerClus()
{
	/* Ŭ������ �� ������ ������ ����� ������ ���� ���� */
	return m_pFATCommonHeader->BPB_SecPerClus;
}

DWORD CVHMFilesystemFAT32::GetClusterSize()
{
	/* Ŭ�������� ũ�Ⱑ ����� ������ ���� ���� */
	return m_dwClusterSize;
}

DWORD CVHMFilesystemFAT32::GetClusterCount()
{
	/* Ŭ������ ������ ����� ������ ���� ���� */
	return m_dwClusterCount;
}

BOOL CVHMFilesystemFAT32::IsBasicInformationLoaded()
{
	/* �⺻ ���� �ε� ���θ� ��Ÿ���� ������ ���� ���� */
	return m_bBasicInformationLoaded;
}

int CVHMFilesystemFAT32::InitializeHandleManagement()
{
	/* VHMIOWrapper ��ü�� ��ȿ�� �˻� */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;
	
	/* ���Ͻý����� ����Ʈ�Ǿ����� Ȯ�� */
	if(!IsMounted())
		return VHM_ERROR_GENERIC;

	/* �ڵ� ���� ��ƾ�� �̹� �ʱ�ȭ�� -> ������ �۾� ���� */
	if(IsHandleManagementInitialized())
		return VHM_ERROR_SUCCESS;

	/* �ڵ� ���� ���� �Ҵ� */
	m_hHandleID = HandleIDCreate(sizeof(FAT32_FOBJ_DESC_INTERNAL), HANDLE_ALLOCATION_UNIT_FAT32);

	m_bHandleManagementInitialized = TRUE;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::UninitializeHandleManagement()
{
	/* VHMIOWrapper ��ü�� ��ȿ�� �˻� */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;
	
	/* ���Ͻý����� ����Ʈ�Ǿ����� Ȯ�� */
	if(!IsMounted())
		return VHM_ERROR_GENERIC;

	/* �ڵ� ���� ��ƾ�� �ʱ�ȭ���� ���� -> ������ �۾� ���� */
	if(!IsHandleManagementInitialized())
		return VHM_ERROR_SUCCESS;
	
	m_bHandleManagementInitialized = FALSE;

	/* ��� ���� �ݱ� */

	/* �ڵ� ���� ���� �ı� �� nullptr�� ���� */
	HandleIDDestroy(m_hHandleID);
	m_hHandleID = nullptr;

	return VHM_ERROR_SUCCESS;
}

BOOL CVHMFilesystemFAT32::IsHandleManagementInitialized()
{
	/* �ڵ� ���� ��ƾ�� �ʱ�ȭ ���θ� ��Ÿ���� ������ ���� ���� */
	return m_bHandleManagementInitialized;
}

int CVHMFilesystemFAT32::ReadCluster(DWORD dwStartCluster, DWORD dwClusterCount, BYTE *pBuffer)
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	// Ŭ������ �ε����� ���� �˻�
	if (dwStartCluster < 2 || dwStartCluster + dwClusterCount > GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	QWORD qwLBAFirstSector;
	DWORD dwSectorCountToRead;
	
	ClusterIndexToLBA(dwStartCluster, &qwLBAFirstSector);
	dwSectorCountToRead = dwClusterCount * GetSecPerClus();

	return ReadSector(qwLBAFirstSector, dwSectorCountToRead, pBuffer, dwSectorCountToRead * GetSectorSize());
}

int CVHMFilesystemFAT32::WriteCluster(DWORD dwStartCluster, DWORD dwClusterCount, BYTE *pBuffer)
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	// Ŭ������ �ε����� ���� �˻�
	if (dwStartCluster < 2 || dwStartCluster + dwClusterCount > GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	QWORD qwLBAFirstSector;
	DWORD dwSectorCountToWrite;

	ClusterIndexToLBA(dwStartCluster, &qwLBAFirstSector);
	dwSectorCountToWrite = dwClusterCount * GetSecPerClus();

	return WriteSector(qwLBAFirstSector, dwSectorCountToWrite, pBuffer);
}

int CVHMFilesystemFAT32::FindFreeCluster(DWORD *pdwIndex)
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	DWORD dwSectorSize;
	BYTE *pBuffer;
	DWORD *pdwFATBuff;
	QWORD qwFATStart;
	QWORD qwFATCurSec;
	DWORD dwIndex;
	int err_code;

	dwSectorSize = GetSectorSize();

	// FAT ���� ������ ���� LBA �ּ� ���
	// �̷��� ��� -> FAT 0
	// Ȱ�� FAT ��� -> �ش� FAT
	if (m_qwVolumeAttributes & FAT32_VOL_ATTR_FAT_MODE_MASK)
		qwFATStart = m_qwFATAreaStartSector + m_qwFATAreaSectorCount * m_dwActiveFAT;
	else
		qwFATStart = m_qwFATAreaStartSector;

	pBuffer = (BYTE *)malloc(dwSectorSize);

	dwIndex = 0xFFFFFFFF;
	for (DWORD dwLBA = 0; dwLBA < m_qwFATAreaSectorCount; ++dwLBA)
	{
		qwFATCurSec = qwFATStart + dwLBA;
		err_code = ReadSector(qwFATCurSec, 1, pBuffer, dwSectorSize);
		if (err_code != VHM_ERROR_SUCCESS)
			goto cleanup;

		pdwFATBuff = (DWORD *)pBuffer;
		for (DWORD i = 0; i < dwSectorSize / 4; ++i)
		{
			if (!(pdwFATBuff[i] & FAT32_DATA_MASK_DATA))
			{
				dwIndex = dwLBA * dwSectorSize / 4 + i;
				goto cleanup;
			}	
		}
	}

cleanup:
	if (dwIndex == 0xFFFFFFFF && err_code == VHM_ERROR_SUCCESS)
		err_code = VHM_ERROR_NOT_FOUND;
	else
		*pdwIndex = dwIndex;

	free(pBuffer);
	return err_code;
}

int CVHMFilesystemFAT32::AllocateCluster(DWORD dwIndex)
{
	DWORD dwValue;
	ReadFAT(dwIndex, &dwValue);

	if ((dwValue & FAT32_DATA_MASK_DATA) == FAT32_BAD_CLUSTER ||
		(dwValue & FAT32_DATA_MASK_DATA) >= FAT32_END_OF_CLUSTER_CHAIN_THRESHOLD)
		return VHM_ERROR_NOT_AVAILABLE;

	return WriteFAT(dwIndex, FAT32_END_OF_CLUSTER_CHAIN);
}

int CVHMFilesystemFAT32::FreeCluster(DWORD dwIndex)
{
	return WriteFAT(dwIndex, FAT32_FREE_CLUSTER);
}	

int CVHMFilesystemFAT32::LinkCluster(DWORD dwFirst, DWORD dwLast)
{
	return WriteFAT(dwFirst, dwLast);
}

int CVHMFilesystemFAT32::UnlinkCluster(DWORD dwIndex)
{
	return WriteFAT(dwIndex, FAT32_END_OF_CLUSTER_CHAIN);
}

int CVHMFilesystemFAT32::MarkAsBadCluster(DWORD dwIndex)
{
	return WriteFAT(dwIndex, FAT32_BAD_CLUSTER);
}

int CVHMFilesystemFAT32::ReadFAT(DWORD dwIndex, DWORD *pdwValue)
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	/* dwIndex�� ���� �˻� */
	if (dwIndex >= GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	int err_code;
	QWORD qwFATLBA_Relative;
	QWORD qwFATLBA_Absolute;
	DWORD dwOffsetInSector;
	BYTE *pBuffer;

	// ���� LBA �ּ� ���
	// �̷��� ��� -> FAT 0
	// Ȱ�� FAT ��� -> �ش� FAT
	ClusterToFATLBA(dwIndex, &qwFATLBA_Relative, &dwOffsetInSector);
	if (m_qwVolumeAttributes & FAT32_VOL_ATTR_FAT_MODE_MASK)
		qwFATLBA_Absolute = m_qwFATAreaStartSector + m_qwFATAreaSectorCount * m_dwActiveFAT + qwFATLBA_Relative;
	else
		qwFATLBA_Absolute = m_qwFATAreaStartSector + qwFATLBA_Relative;

	// FAT �б�
	pBuffer = (BYTE *)malloc(GetSectorSize());
	if ((err_code = ReadSector(qwFATLBA_Absolute, 1, pBuffer, GetSectorSize())) != VHM_ERROR_SUCCESS)
	{
		free(pBuffer);
		return err_code;
	}

	*pdwValue = (*(DWORD *)(pBuffer + dwOffsetInSector)) & FAT32_DATA_MASK_DATA;

	free(pBuffer);
	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::WriteFAT(DWORD dwIndex, DWORD dwValue)
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	/* dwIndex�� ���� �˻� */
	if (dwIndex >= GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	// ���� FAT ��忡 ���� WriteFAT�� ȣ��
	int err_code, err_code_temp;
	if (m_qwVolumeAttributes & FAT32_VOL_ATTR_FAT_MODE_MASK)
	{
		// Ȱ�� FAT�� ���� �۾� ����
		err_code = WriteFAT(m_dwActiveFAT, dwIndex, dwValue);
	}
	else
	{
		// ��� FAT(�̷���)�� ���� �۾� ����
		err_code = VHM_ERROR_SUCCESS;
		for (DWORD i = 0; i < m_dwFATCount; ++i)
		{
			err_code_temp = WriteFAT(i, dwIndex, dwValue);
			if (err_code_temp != VHM_ERROR_SUCCESS)
				err_code = err_code_temp;
		}
	}

	return err_code;
}

int CVHMFilesystemFAT32::WriteFAT(DWORD dwFATIndex, DWORD dwIndex, DWORD dwValue)
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	/* dwFATIndex�� dwIndex�� ���� �˻� */
	if (dwFATIndex >= m_dwFATCount || dwIndex >= GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	int err_code;
	QWORD qwFATLBA_Relative;
	QWORD qwFATLBA_Absolute;
	DWORD dwOffsetInSector;
	BYTE *pBuffer;
	DWORD *pdwValue;

	// ���� LBA �ּ� ���
	pBuffer = (BYTE *)malloc(GetSectorSize());
	ClusterToFATLBA(dwIndex + 2, &qwFATLBA_Relative, &dwOffsetInSector);
	qwFATLBA_Absolute = m_qwFATAreaStartSector + m_qwFATAreaSectorCount * dwFATIndex + qwFATLBA_Relative;

	// FAT�� ����
	if ((err_code = ReadSector(qwFATLBA_Absolute, 1, pBuffer, GetSectorSize())) != VHM_ERROR_SUCCESS)
	{
		free(pBuffer);
		return err_code;
	}

	pdwValue = (DWORD *)(pBuffer + dwOffsetInSector);
	*pdwValue = (*pdwValue & FAT32_DATA_MASK_PRESERVE) | (dwValue & FAT32_DATA_MASK_DATA);

	if ((err_code = WriteSector(qwFATLBA_Absolute, 1, pBuffer)) != VHM_ERROR_SUCCESS)
	{
		free(pBuffer);
		return err_code;
	}

	free(pBuffer);
	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::ClusterToFATLBA(DWORD dwClusterIndex, QWORD *pqwFATLBA, DWORD *pdwOffsetInSector)
{
	/* VHMIOWrapper ��ü�� ��ȿ�� �˻� */
	if (!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	/* �⺻ ���� �ε� ���� �˻� */
	if (!IsBasicInformationLoaded())
		return VHM_ERROR_GENERIC;

	// Ŭ������ �ε����� ���� �˻�
	// Ư���� ��Ȳ���� FAT�� 0,1�� Ŭ������ �����͸� Read/Write�� �� ����
	if (dwClusterIndex > GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	// Ŭ������ �ε����� dwFATLBA��° ������ ������ dwOffsetInSector�� ��ȯ
	dwClusterIndex = dwClusterIndex * 4;
	*pqwFATLBA = dwClusterIndex / GetSectorSize();
	*pdwOffsetInSector = dwClusterIndex % GetSectorSize();

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::ClusterIndexToLBA(DWORD dwClusterIndex, QWORD *pqwLBA)
{
	/* VHMIOWrapper ��ü�� ��ȿ�� �˻� */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	/* �⺻ ���� �ε� ���� �˻� */
	if(!IsBasicInformationLoaded())
		return VHM_ERROR_GENERIC;

	// Ŭ������ �ε����� ���� �˻�
	if(dwClusterIndex < 2 || dwClusterIndex >= GetClusterCount() + 2)
		return VHM_ERROR_INVALID_PARAMETER;

	// Ŭ������ �ε����� LBA�� ��ȯ
	*pqwLBA = m_qwDataAreaStartSector + (dwClusterIndex - 2) * GetSecPerClus();

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::MarkHardwareError()
{
	/* ���Ͻý��� ����Ʈ ���� �˻� */
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	// �ϵ���� ���� ǥ��
	// �� FAT�� 0x1(zero-based)�� Ŭ�����Ϳ� HW_ERROR_MASK ����

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::AttrConvFAT32ToVhm(BYTE AttrFAT32, QWORD *pAttrVHM)
{
	QWORD AttrVHM = 0;
	
	if (AttrFAT32 & FAT_ATTR_DIRECTORY)
		AttrVHM |= FOBJ_ATTR_DIRECTORY;
	if (AttrFAT32 & FAT_ATTR_READ_ONLY)
		AttrVHM |= FOBJ_ATTR_READ_ONLY;
	if (AttrFAT32 & FAT_ATTR_HIDDEN)
		AttrVHM |= FOBJ_ATTR_HIDDEN;
	if (AttrFAT32 & FAT_ATTR_SYSTEM)
		AttrVHM |= FOBJ_ATTR_SYSTEM;
	if (AttrFAT32 & FAT_ATTR_ARCHIVE)
		AttrVHM |= FOBJ_ATTR_ARCHIVE;
	if (AttrFAT32 & FAT_ATTR_VOLUME_ID)
		AttrVHM |= FOBJ_ATTR_VOLUME_ID;

	*pAttrVHM = AttrVHM;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::AttrConvVhmToFAT32(QWORD AttrVHM, BYTE *pAttrFAT32)
{
	BYTE AttrFAT32 = 0;

	if (AttrVHM & FOBJ_ATTR_DIRECTORY)
		AttrFAT32 |= FAT_ATTR_DIRECTORY;
	if (AttrVHM & FOBJ_ATTR_READ_ONLY)
		AttrFAT32 |= FAT_ATTR_READ_ONLY;
	if (AttrVHM & FOBJ_ATTR_HIDDEN)
		AttrFAT32 |= FAT_ATTR_HIDDEN;
	if (AttrVHM & FOBJ_ATTR_SYSTEM)
		AttrFAT32 |= FAT_ATTR_SYSTEM;
	if (AttrVHM & FOBJ_ATTR_ARCHIVE)
		AttrFAT32 |= FAT_ATTR_ARCHIVE;
	if (AttrVHM & FOBJ_ATTR_VOLUME_ID)
		AttrFAT32 |= FAT_ATTR_VOLUME_ID;

	*pAttrFAT32 = AttrFAT32;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::GetDirItemInit(void *pDirEntBuf, QWORD qwDirEntBufSize, QWORD *pqwTempVal)
{
	if (!pDirEntBuf || qwDirEntBufSize == 0 || !pqwTempVal)
		return VHM_ERROR_INVALID_PARAMETER;

	*pqwTempVal = 0;
	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::GetNextDirItem(void *pDirEntBuf, QWORD qwDirEntBufSize, FAT32_FOBJ_DESC_INTERNAL **ppFObject, QWORD *pqwTempVal)
{
	if (!pDirEntBuf || qwDirEntBufSize == 0 || !pqwTempVal)
		return VHM_ERROR_INVALID_PARAMETER;

	if (!IsMultipleOf(*pqwTempVal, FAT_DIR_ENTRY_SIZE))
		return VHM_ERROR_INTERNAL;

	if (*pqwTempVal == qwDirEntBufSize)
		return VHM_ERROR_NOT_FOUND;

	int err_code;
	QWORD qwOffset;
	BYTE *pDirEntBuf2 = (BYTE *)pDirEntBuf;
	FAT_DIR_ENTRY *pDirEnt;
	FAT_LFN_ENTRY *pLfnEnt;
	FAT32_FOBJ_DESC_INTERNAL *pFObject;
	QWORD qwNameLen;


	// ���� ��Ʈ�� ���
	qwOffset = *pqwTempVal;

get_next_entry:

	err_code = GetNextValidDirEntry(pDirEntBuf, qwDirEntBufSize, &qwOffset, (void **)&pDirEnt);
	if (err_code != VHM_ERROR_SUCCESS)
		goto cleanup;

	pLfnEnt = (FAT_LFN_ENTRY *)pDirEnt;

	// LFN ��Ʈ���ΰ�?
	if ((pLfnEnt->LDIR_Attr & FAT_ATTR_LONG_NAME_MASK) == FAT_ATTR_LONG_NAME)
	{
		// LFN ��Ʈ���� ��ȿ�� �˻�

		goto get_next_entry; // �ӽ�(LFN����)

		err_code = VHM_ERROR_SUCCESS;
		goto cleanup;
	}

	switch(pDirEnt->DIR_Attr & (FAT_ATTR_DIRECTORY | FAT_ATTR_VOLUME_ID))
	{
	case 0x00:
	case FAT_ATTR_DIRECTORY:
	case FAT_ATTR_VOLUME_ID:
		break;
	default:
		// �ùٸ��� ���� ��Ʈ��
		goto get_next_entry;
	}

	NameLen83(pDirEnt, &qwNameLen);
	pFObject = (FAT32_FOBJ_DESC_INTERNAL *)malloc
		(sizeof(FAT32_FOBJ_DESC_INTERNAL) + sizeof(WCHAR)*12);

	pFObject->dwStartCluster = (((DWORD)pDirEnt->DIR_FstClusHI) << 16) | ((DWORD)pDirEnt->DIR_FstClusLO);
	pFObject->dwCurrentCluster = pFObject->dwStartCluster;
	pFObject->qwSize = pDirEnt->DIR_FileSize;
	pFObject->qwPointer = 0;
	AttrConvFAT32ToVhm(pDirEnt->DIR_Attr, &pFObject->qwAttributes);
	pFObject->qwNameLen = qwNameLen;
	Name83ToVhm(pDirEnt, pFObject->qwAttributes, pFObject->wName);

	*ppFObject = pFObject;

/*
	// �����ΰ�?
	if ((pDirEnt->DIR_Attr & (FAT_ATTR_DIRECTORY | FAT_ATTR_VOLUME_ID)) == 0x00)
	{
		// ������ ã��
		pFObject = (FAT32_FOBJ_DESC_INTERNAL *)AllocateMemory(sizeof(FOBJECT_DESCRIPTOR)+(sizeof(WCHAR)* 12));
		pFObject->dwStartCluster = (((DWORD)pDirEnt->DIR_FstClusHI) << 16) | ((DWORD)pDirEnt->DIR_FstClusLO);
		pFObject->dwCurrentCluster = pFObject->dwStartCluster;
		pFObject->qwSize = pDirEnt->DIR_FileSize;
		pFObject->qwPointer = 0;
		AttrConvFAT32ToVhm(pDirEnt->DIR_Attr, &pFObject->qwAttributes);
		pFObject->qwNameLen = 0;
	}

	// ���͸��ΰ�?
	if ((pDirEnt->DIR_Attr & (FAT_ATTR_DIRECTORY | FAT_ATTR_VOLUME_ID)) == FAT_ATTR_DIRECTORY)
	{
		// ���͸��� ã��
	}

	// ���� ���̺��ΰ�?
	if ((pDirEnt->DIR_Attr & (FAT_ATTR_DIRECTORY | FAT_ATTR_VOLUME_ID)) == FAT_ATTR_VOLUME_ID)
	{
	// ���� ���̺��� ã��
	// (���� ���̺��� LFN�� �������� �ʰ� MBCS 11byte�� ���ѵ�-SPEC����)


	}
	*/

	err_code = VHM_ERROR_SUCCESS;
	goto cleanup;

cleanup:
	*pqwTempVal = qwOffset;
	return err_code;
}

int CVHMFilesystemFAT32::GetNextValidDirEntry(void *pDirEntBuf, QWORD qwDirEntBufSize, QWORD *pqwCurPos, void **ppDirEnt)
{
	if (!pDirEntBuf || qwDirEntBufSize == 0 || !pqwCurPos)
		return VHM_ERROR_INVALID_PARAMETER;

	if (!IsMultipleOf(*pqwCurPos, FAT_DIR_ENTRY_SIZE))
		return VHM_ERROR_INTERNAL;

	if (*pqwCurPos == qwDirEntBufSize)
		return VHM_ERROR_NOT_FOUND;

	int err_code;
	QWORD qwOffset;
	BYTE *pDirEntBuf2 = (BYTE *)pDirEntBuf;
	FAT_DIR_ENTRY *pDirEnt;

	// ������ ��Ʈ��(0xE5)�� �ѱ�
	for (qwOffset = *pqwCurPos; qwOffset < qwDirEntBufSize; qwOffset += FAT_DIR_ENTRY_SIZE)
	{
		pDirEnt = (FAT_DIR_ENTRY *)(pDirEntBuf2 + qwOffset);

		if (pDirEnt->DIR_Name[0] == 0xE5)
			continue; // skip deleted entry
		else if (pDirEnt->DIR_Name[0] == 0x00)
		{
			err_code = VHM_ERROR_NOT_FOUND;
			goto cleanup;
		}
		else
		{
			*pqwCurPos = qwOffset + FAT_DIR_ENTRY_SIZE;
			*ppDirEnt = (void *)(pDirEntBuf2 + qwOffset);
			break; // found a valid entry
		}
	}

	err_code = VHM_ERROR_SUCCESS;

cleanup:
	return err_code;
}

int CVHMFilesystemFAT32::NameLen83(void *pDirEnt, QWORD *pqwLen)
{
	if (!pDirEnt || !pqwLen)
		return VHM_ERROR_INVALID_PARAMETER;

	FAT_DIR_ENTRY *pDirEnt2 = (FAT_DIR_ENTRY *)pDirEnt;
	QWORD qwLen = 0;
	QWORD i;

	// �̸� �κ�
	for (i = 7; i != 0; --i)
	{
		if (pDirEnt2->DIR_Name[i] != 0x20 && pDirEnt2->DIR_Name[i] != 0x00)
			break;
	}
	qwLen += i;

	// Ȯ���� �κ�
	for (i = 3; i != 0; --i)
	{
		if (pDirEnt2->DIR_Name[i + 8] != 0x20 && pDirEnt2->DIR_Name[i + 8] != 0x00)
			break;
	}
	qwLen += i;

	*pqwLen = qwLen;
	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::Name83ToVhm(void *pDirEnt, QWORD qwAttributes, WCHAR *pwOutBuf)
{
	if (!pDirEnt || !pwOutBuf)
		return VHM_ERROR_INVALID_PARAMETER;

	FAT_DIR_ENTRY *pDirEnt2 = (FAT_DIR_ENTRY *)pDirEnt;
	QWORD qwLen = 0;
	QWORD i;

	// ���� ���̺��� ��� �ٸ��� ó����
	if (qwAttributes & FOBJ_ATTR_VOLUME_ID)
	{
		for (i = 0; i < 11; ++i)
		{
			if (pDirEnt2->DIR_Name[i] == 0x00)
				break;

			*pwOutBuf++ = (WCHAR)pDirEnt2->DIR_Name[i];
		}
	}
	else
	{
		// �̸� �κ�

		i = 0;
		if (pDirEnt2->DIR_Name[0] == 0x05)
			/* DIR_Name[0]�� 0x05�̸� �̴� 0xE5�� ����-SPEC����*/
			*pwOutBuf++ = (WCHAR)(0xE5), ++i;

		for (; i < 8; ++i)
		{
			if (pDirEnt2->DIR_Name[i] == 0x20 || pDirEnt2->DIR_Name[i] == 0x00)
				break;

			*pwOutBuf++ = (WCHAR)pDirEnt2->DIR_Name[i];
		}
		qwLen += i;

		// Ȯ���� �κ�
		for (i = 0; i < 3; ++i)
		{
			if (pDirEnt2->DIR_Name[i + 8] == 0x20 || pDirEnt2->DIR_Name[i + 8] == 0x00)
				break;


			*pwOutBuf++ = (WCHAR)pDirEnt2->DIR_Name[i + 8];
		}
		qwLen += i;
	}

	// nullptr terminator
	*pwOutBuf = '\0';

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::NameLenLfn(void *pDirEnt, QWORD *pqwLen)
{
	if (!pDirEnt || !pqwLen)
		return VHM_ERROR_INVALID_PARAMETER;

	FAT_LFN_ENTRY *pDirEnt2 = (FAT_LFN_ENTRY *)pDirEnt;
	QWORD qwLen = 0;
	QWORD i;



	*pqwLen = qwLen;
	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::LoadVolumeLabel()
{
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	DWORD dwIndex;
	DWORD dwClusterSize;
	DWORD dwClusterCount;
	DWORD i;
	BYTE *pBuffer;
	QWORD qwRootSz;
	QWORD qwTempVal;
	FAT32_FOBJ_DESC_INTERNAL *pFObject;
	BOOL bFound;
	int err_code;

	dwIndex = m_pFAT32Header->BPB_RootClus;
	dwClusterSize = GetClusterSize();
	
	// ��Ʈ ���͸��� Ŭ������ �� ���
	dwClusterCount = 1;
	for (;;)
	{
		ReadFAT(dwIndex, &dwIndex);

		if (FAT32_IS_END_OF_CLUSTER_CHAIN(dwIndex))
			break;

		++dwClusterCount;
	}

	// ��Ʈ ���͸� �б�
	qwRootSz = dwClusterCount * dwClusterSize;
	pBuffer = new BYTE[qwRootSz];
	dwIndex = m_pFAT32Header->BPB_RootClus;

	for (i = 0; i < dwClusterCount; ++i)
	{
		ReadCluster(dwIndex, 1, pBuffer + (i * dwClusterSize));
		ReadFAT(dwIndex, &dwIndex);
	}

	// ���� ���̺� �˻�

	bFound = FALSE;
	TRACE(_T("Start output\r\n"));

	GetDirItemInit(pBuffer, qwRootSz, &qwTempVal);
	for (;;)
	{
		if (GetNextDirItem(pBuffer, qwRootSz, &pFObject, &qwTempVal) != VHM_ERROR_SUCCESS)
			break;

		TRACE(_T("%s\r\n"), pFObject->wName);

		if (pFObject->qwAttributes & FOBJ_ATTR_VOLUME_ID)
		{
			bFound = TRUE;
			m_pwszVolumeLabel[0] = _T('\0');
			_tcscpy_s(m_pwszVolumeLabel, 12, pFObject->wName);
			free(pFObject);

			break;
		}

		free(pFObject);
	}

	if (bFound)
		err_code = VHM_ERROR_SUCCESS;
	else
		err_code = VHM_ERROR_NOT_FOUND;
	
	delete[] pBuffer;

	return err_code;
}

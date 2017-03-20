
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/List.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMDisk.h"
#include "VHMPartition.h"
#include "VHMIOWrapper.h"
#include "VHMFilesystem.h"

CVHMPartition::CVHMPartition()
{
	m_pVHMIOWrapper = nullptr;
	m_pListPartition = nullptr;
	m_bOpened = FALSE;
	m_bPartitionTableLoaded = FALSE;
	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;
	m_pPartitioningTypeSpecificData = nullptr;
	m_qwPartitioningTypeSpecificDataSize = 0;
	
	m_bIDGeneratorInit = FALSE;
}

CVHMPartition::~CVHMPartition(void)
{
	Destroy();
}

DWORD CVHMPartition::DeterminePartitioningType(CVHMIOWrapper *pVHMIOWrapper)
{
	if(!pVHMIOWrapper)
		return PART_TYPE_INVALID_VHMDISK;

	if(DPT_GPT(pVHMIOWrapper))
		return PART_TYPE_GPT;

	if(DPT_MBR(pVHMIOWrapper))
		return PART_TYPE_MBR;

	return PART_TYPE_UNINITIALIZED;
}

BOOL CVHMPartition::DPT_GPT(CVHMIOWrapper *pVHMIOWrapper)
{
	DWORD dwSectorSize;
	QWORD qwGPTStartLBA;
	BYTE *pLBA0;
	BYTE *pLBA1;
	BYTE *pLBALast;
	QWORD *pTemp;
	MBR_STRUCTURE *pMBR;
	MBR_PARTITION_ENTRY *pMBRPartitionEntry;
	GPT_HEADER *pGPTHeaderPrimary, *pGPTHeaderBackup;
	
	DWORD i;

	dwSectorSize = pVHMIOWrapper->GetSectorSize();

	pLBA0 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(0, 1, pLBA0, dwSectorSize);

	pMBR = (MBR_STRUCTURE *) pLBA0;
	pMBRPartitionEntry = pMBR->mPartitionEntry;

	/* ��ũ ���� �˻� (0xAA55) */
	if(pMBR->wBootSignature != DISK_SIGNATURE)
	{
		free(pLBA0);
		return FALSE;
	}

	/* ��ũ�� Protective MBR ���� ���� Ȯ�� */
	DWORD dwProtectiveMBREntryIndex, dwMBREntryCount, dwProtectiveMBREntryCount;
	dwProtectiveMBREntryIndex = (DWORD) -1;
	dwMBREntryCount = 0;
	dwProtectiveMBREntryCount = 0;

	for(i=0; i<4; i++)
	{
		pTemp = (QWORD *) &pMBRPartitionEntry[i];
		if((*pTemp) || (*(pTemp+1)))
		{
			dwMBREntryCount++;
			if(pMBRPartitionEntry[i].bPartitionType == MBR_PARID_GPTPROTECTIVE)
			{
				dwProtectiveMBREntryCount++;
				dwProtectiveMBREntryIndex = i;
				qwGPTStartLBA = pMBRPartitionEntry[i].dwFirstSectorLBA;
			}
		}
	}

	free(pLBA0);

	if(dwProtectiveMBREntryCount != 1 ||
		dwProtectiveMBREntryCount == (DWORD) -1 ||
		qwGPTStartLBA != 1 ||
		dwMBREntryCount > 1)
		return FALSE;

	/* ��ũ�� GPT ��� Ȯ�� */

	pLBA1 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(qwGPTStartLBA, 1, pLBA1, dwSectorSize);
	pGPTHeaderPrimary = (GPT_HEADER *) pLBA1;

	pLBALast = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(pGPTHeaderPrimary->AlternateLBA, 1, pLBALast, dwSectorSize);
	pGPTHeaderBackup = (GPT_HEADER *) pLBALast;

	/* Primary GPT Header�� Backup GPT Header �� �ϳ��� �ùٸ��� ������ GPT ��ũ�� �ƴ� ������ ���� */

	if(!DPT_GPTHEADER(pVHMIOWrapper, pGPTHeaderPrimary, 1, TRUE) ||
		!DPT_GPTHEADER(pVHMIOWrapper, pGPTHeaderBackup, pGPTHeaderPrimary->AlternateLBA, FALSE))
	{
		free(pLBA1);
		free(pLBALast);
		return FALSE;
	}

	free(pLBA1);
	free(pLBALast);
	
	return TRUE;
}

BOOL CVHMPartition::DPT_GPTHEADER(CVHMIOWrapper *pVHMIOWrapper, GPT_HEADER *pGPTHeader, QWORD qwLBA, BOOL bFirst)
{
	DWORD dwTemp;
	DWORD dwCRC32;
	DWORD dwSectorCountOfPartitionEntries;
	DWORD dwSectorSize;
	DWORD dwSizeOfPartitionEntry;
	DWORD dwNumberOfPartitionEntries;
	QWORD qwPartitionEntryStartLBA;
	BYTE *pGPTPartitionEntries;

	/* GPT ��� ���� Ȯ�� */
	if(pGPTHeader->Signature != GPT_HEADER_SIGNATURE)
		return FALSE;
	
	/* GPT ����� MyLBA�� �� ����� LBA�� ����Ű�� �� Ȯ�� */
	if(pGPTHeader->MyLBA != qwLBA)
		return FALSE;

	/* GPT ����� CRC32 ���� 0���� ���� CRC32�� ����Ͽ� ������ ���� ��ġ�ϴ� �� Ȯ�� */
	dwTemp = pGPTHeader->HeaderCRC32;
	pGPTHeader->HeaderCRC32 = 0;

	dwCRC32 = CRC32Calc((BYTE *) pGPTHeader, sizeof(GPT_HEADER), 0);
	pGPTHeader->HeaderCRC32 = dwTemp;

	if(dwTemp != dwCRC32)
		return FALSE;

	/* ���� ũ��, GPT ��Ƽ�� ��Ʈ���� ũ��, GPT ��Ƽ�� ��Ʈ���� ���� ���� ������ ���� �� ��ȿ�� �˻� (����) */
	dwSectorSize = pVHMIOWrapper->GetSectorSize();
	dwSizeOfPartitionEntry = pGPTHeader->SizeOfPartitionEntry;
	dwNumberOfPartitionEntries = pGPTHeader->NumberOfPartitionEntries;

	if(!dwSizeOfPartitionEntry || dwSizeOfPartitionEntry < 128 || !IsPowerOfTwo(dwSizeOfPartitionEntry))
		return FALSE;

	dwSectorCountOfPartitionEntries = CVHMPartition::GPTCalculatePartitionEntrySectorCount(dwSizeOfPartitionEntry, dwNumberOfPartitionEntries, dwSectorSize);
	if(!dwSectorCountOfPartitionEntries)
		return FALSE;

	/* GPT ��Ƽ�� ��Ʈ���� CRC32�� ����Ͽ� GPT ����� ����� ���� �� */
	pGPTPartitionEntries = (BYTE *) malloc(dwSectorCountOfPartitionEntries * dwSectorSize);
	if(bFirst)
		qwPartitionEntryStartLBA = qwLBA + 1;
	else
		qwPartitionEntryStartLBA = qwLBA - dwSectorCountOfPartitionEntries;

	/* GPT ����� ��Ƽ�� ��Ʈ�� ���� LBA �ּҿ� ���� LBA �ּҰ� ��ġ�ؾ� �� */
	if(qwPartitionEntryStartLBA != pGPTHeader->PartitionEntryLBA)
	{
		free(pGPTPartitionEntries);
		return FALSE;
	}

	pVHMIOWrapper->ReadSector(pGPTHeader->PartitionEntryLBA, dwSectorCountOfPartitionEntries, pGPTPartitionEntries, dwSectorCountOfPartitionEntries * dwSectorSize);
	if(CRC32Calc(pGPTPartitionEntries, dwSizeOfPartitionEntry * dwNumberOfPartitionEntries, 0) != pGPTHeader->PartitionEntryArrayCRC32)
	{
		free(pGPTPartitionEntries);
		return FALSE;
	}

	free(pGPTPartitionEntries);

	return TRUE;
}

BOOL CVHMPartition::DPT_MBR(CVHMIOWrapper *pVHMIOWrapper)
{
	DWORD dwSectorSize;
	BYTE *pLBA0;
	MBR_STRUCTURE *pMBR;

	/* ��ũ ���� �������� */
	dwSectorSize = pVHMIOWrapper->GetSectorSize();
	
	/* �޸� �Ҵ� �� MBR �б� */
	pLBA0 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(0, 1, pLBA0, dwSectorSize);
	pMBR = (MBR_STRUCTURE *) pLBA0;

	/* ��ũ ���� Ȯ�� */
	if(pMBR->wBootSignature != DISK_SIGNATURE)
	{
		free(pLBA0);
		return FALSE;
	}

	free(pLBA0);

	return TRUE;
}

int CVHMPartition::Create(CVHMIOWrapper *pVHMIOWrapper)
{
	/* pVHMIOWrapper�� ��ȿ�ϰ�, �� VHMPartition ��ü�� �ٸ� ��ũ�� ��Ƽ���� �ε��� �����̸� �� �� */
	if(!pVHMIOWrapper)
		return VHM_ERROR_INVALID_VHMDISK;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	/* VHMIOWrapper�� �ε� (��������� �� ����) */
	m_pVHMIOWrapper = pVHMIOWrapper;
	m_bOpened = TRUE;

	/* ��Ƽ�Ŵ� Ÿ���� �Ǻ��ϰ� LoadPartition �Լ��� ȣ�� */
	m_dwPartitioningType = CVHMPartition::DeterminePartitioningType(pVHMIOWrapper);
	if(LoadPartition() != VHM_ERROR_SUCCESS)
		return VHM_ERROR_SUCCESS; /* �νĵ� ��Ƽ�Ŵ� Ÿ���� ������ ���Ͻý��� �ε尡 �Ұ��� */

	/* VHMFilesystem �ε� */
	LoadFilesystem();

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::LoadFilesystem()
{
	/* VHMIOWrapper ��ü�� ��ȿ�ؾ� ��*/
	if(!IsOpened())
		return -1;

	/* ��Ƽ�� ���̺��� �ε�Ǿ� �־�� ��*/
	if(!m_bPartitionTableLoaded || !m_pListPartition)
		return -1;

	/* ��Ƽ�� ����Ʈ���� ��Ƽ�� ������ �о� ���Ͻý����� �ε��ϰ� ���� ��Ƽ������ �Ѿ */
	/* ��Ƽ�� ����Ʈ�� ���� ������ ������ �ݺ� */
	LIST_STRUCT pListCurrent = ListGetHead(m_pListPartition);
	for(;;)
	{
		LoadFilesystem((PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pListCurrent));
		pListCurrent = ListGetNext(pListCurrent);
		if(pListCurrent == nullptr)
			break;
	}

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::UnloadFilesystem()
{
	/* VHMDisk�� ��ȿ���� ������ ��ε��� �� ���� (���Ͻý����� �ε���� ����) */
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	/* ��Ƽ�� ���̺��� �ε���� �ʾ����� ��ε��� �� ���� (���Ͻý����� �ε���� ����) */
	if(!m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	/* ��Ƽ�� ���̺��� ������� ��ε��� �� ���� (���Ͻý����� �ε���� ����) */
	if(!m_pListPartition || ListIsEmpty(m_pListPartition))
		return VHM_ERROR_SUCCESS;

	/* ��Ƽ�� ����Ʈ���� ��Ƽ�� ������ �о� ���Ͻý����� ��ε��ϰ� ���� ��Ƽ������ �Ѿ */
	/* ��Ƽ�� ����Ʈ�� ���� ������ ������ �ݺ� */
	/* ������ �߻��ϸ� ���� ���θ� iSuccess�� �����ϰ� ��� */
	int iSuccess = 0;
	LIST_STRUCT pListCurrent = ListGetHead(m_pListPartition);
	for(;;)
	{
		if(UnloadFilesystem((PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pListCurrent)) != VHM_ERROR_SUCCESS)
			iSuccess |= 0x01;
		pListCurrent = ListGetNext(pListCurrent);
		if(pListCurrent == nullptr)
			break;
	}

	/* ���Ͻý��� ��ε� �� ������ �߻������� ���� ���� */
	if(iSuccess & 0x01)
		return VHM_ERROR_INTERNAL;

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::LoadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	/* �Ű������� ��ȿ�� �˻�*/
	if(!pPartitionDescriptor)
		return VHM_ERROR_INVALID_PARAMETER;

	/* ���Ͻý����� �̹� �ε�� �����̸� �ε��� �� ����*/
	if(pPartitionDescriptor->pVHMFilesystem != nullptr)
		return VHM_ERROR_ALREADY_OPENED;

	/* VHMIOWrapper ��ü ���� */
	CVHMIOWrapper *pVHMIOWrapper = new CVHMIOWrapper;
	if(pVHMIOWrapper->Initialize(m_pVHMIOWrapper, pPartitionDescriptor->StartLBA, pPartitionDescriptor->SectorCountLBA) != VHM_ERROR_SUCCESS)
	{
		delete pVHMIOWrapper;
		return -1;
	}

	/* �ش� ��Ƽ�ǿ� ���Ͽ� VHMFilesystem ��ü�� �����ϴ� �Լ��� ȣ�� */
	/* iSuccess�� ���� �ڵ� -> ������ �߻������� iSuccess�� ���� */
	CVHMFilesystem *pVHMFilesystem = nullptr;
	int iSuccess = CVHMFilesystem::CreateVHMFilesystemObject(pVHMIOWrapper, &pVHMFilesystem);
	if(pVHMFilesystem == nullptr)
	{
		delete pVHMIOWrapper;
		return iSuccess;
	}

	/* ������ ��ü ���� */
	pPartitionDescriptor->pVHMIOWrapper = pVHMIOWrapper;
	pPartitionDescriptor->pVHMFilesystem = pVHMFilesystem;
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::UnloadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	/* �Ű������� ��ȿ�� �˻� */
	if(!pPartitionDescriptor)
		return VHM_ERROR_INVALID_PARAMETER;

	/* ���Ͻý����� �ε���� ���� �����̸� ��ε��� ������ ���¿� �����Ƿ� VHM_ERROR_SUCCESS ���� */
	if(pPartitionDescriptor->pVHMFilesystem == nullptr)
		return VHM_ERROR_SUCCESS;

	/* VHMFilesystem ��ü�� �ı��ϴ� �Լ��� ȣ�� */
	/* iSuccess�� ���� �ڵ� -> ������ �߻������� iSuccess�� ���� */
	int iSuccess;
	iSuccess = CVHMFilesystem::DestroyVHMFilesystemObject(pPartitionDescriptor->pVHMFilesystem);
	if(iSuccess != VHM_ERROR_SUCCESS)
		return iSuccess;

	/* VHMIOWrapper ��ü �ı� */
	delete pPartitionDescriptor->pVHMIOWrapper;

	/* ��ε��� ��ü���� ���� nullptr�� ���� */
	pPartitionDescriptor->pVHMFilesystem = nullptr;
	pPartitionDescriptor->pVHMIOWrapper = nullptr;

	return VHM_ERROR_SUCCESS;
}

BOOL CVHMPartition::IsVHMFilesystemLoaded()
{
	/* VHMDisk�� ��ȿ���� ���� ���¿����� ���Ͻý����� �ε�Ǿ� ���� �� ���� */
	if(!IsOpened())
		return FALSE;
	
	/* ��Ƽ�� ���̺��� ��ȿ���� ���� ���¿����� ���Ͻý����� �ε�Ǿ� ���� �� ���� */
	if(!m_bPartitionTableLoaded)
		return FALSE;

	/* ��Ƽ�� ���̺��� ����ٸ� ���Ͻý����� �ε�Ǿ� ���� �� ���� */
	if(ListIsEmpty(m_pListPartition))
		return FALSE;

	/* ��Ƽ�� ���̺��� �˻��Ͽ� ���Ͻý����� �ε�� ������ ��Ƽ���� 1�� �̻� �߰��ϸ� TRUE ���� */
	LIST_STRUCT pListCurrent = ListGetHead(m_pListPartition);
	for(;;)
	{
		if(IsVHMFilesystemLoaded((PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pListCurrent)))
			return TRUE;

		pListCurrent = ListGetNext(pListCurrent);
		if(pListCurrent == nullptr)
			break;
	}

	/* ��Ƽ�� ���̺��� ��� ���� ������ ���Ͻý����� �ε�� ��Ƽ���� ���� */
	return FALSE;
}

BOOL CVHMPartition::IsVHMFilesystemLoaded(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	/* �Ű������� ��ȿ�� �˻� */
	if(!pPartitionDescriptor)
		return FALSE;

	/* VHMFilesystem ��ü�� nullptr�� �����Ǿ� ������ ���Ͻý����� �ε���� ���� ���� */
	if(pPartitionDescriptor->pVHMFilesystem == nullptr)
		return FALSE;

	return TRUE;
}

int CVHMPartition::InitializeDiskAsGPT()
{
	if(!IsOpened())
		return -1;

	/* ���Ͻý��۰� ��Ƽ�� ���̺��� ��ε� �� ��Ƽ�Ŵ� Ÿ���� '��ũ�� �ʱ�ȭ���� ����' ���·� ����*/
	UnloadFilesystem();

	if(m_bPartitionTableLoaded)
		UnloadPartition();

	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;

	/* ��ũ�� GPT�� �ʱ�ȭ */
	DWORD dwSectorSize;
	QWORD qwSectorCount;

	dwSectorSize = m_pVHMIOWrapper->GetSectorSize();
	qwSectorCount = m_pVHMIOWrapper->GetSectorCount();

	m_dwPartitioningType = PART_TYPE_GPT;

	/* GPTLoadPartition() �� -1�� �����ϴ� ���� �Ͼ���� �� �� */
	if(GPTLoadPartition() != VHM_ERROR_SUCCESS)
		return -1;

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::InitializeDiskAsMBR()
{
	if(!IsOpened())
		return -1;
	
	/* ���Ͻý��۰� ��Ƽ�� ���̺��� ��ε� �� ��Ƽ�Ŵ� Ÿ���� '��ũ�� �ʱ�ȭ���� ����' ���·� ����*/
	UnloadFilesystem();

	if(m_bPartitionTableLoaded)
		UnloadPartition();
	
	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;

	/* ��ũ�� MBR�� �ʱ�ȭ */
	DWORD dwSectorSize;
	BYTE *pLBA0;
	MBR_STRUCTURE *pMBR;
	DWORD i;

	/* ��ũ�� LBA 0�� 0���� ä��� DISK_SIGNATURE�� ���� */
	dwSectorSize = m_pVHMIOWrapper->GetSectorSize();
	
	pLBA0 = (BYTE *) malloc(dwSectorSize);
	for(i=0; i<dwSectorSize; i++)
		pLBA0[i] = 0;

	pMBR = (MBR_STRUCTURE *) pLBA0;
	pMBR->wBootSignature = DISK_SIGNATURE;

	WriteSector(0, 1, pLBA0);

	free(pLBA0);

	m_dwPartitioningType = PART_TYPE_MBR;
	
	/* MBRLoadPartition() �� -1�� �����ϴ� ���� �Ͼ���� �� �� */
	if(MBRLoadPartition() != VHM_ERROR_SUCCESS)
		return -1;
	
	return VHM_ERROR_SUCCESS;
}

BOOL CVHMPartition::IsOpened()
{
	return m_bOpened;
}

DWORD CVHMPartition::GetPartitioningType()
{
	if(!IsOpened())
		return PART_TYPE_INVALID_VHMDISK;

	return m_dwPartitioningType;
}

UINT64 CVHMPartition::GetPartitionCount()
{
	if (!IsOpened())
		return (UINT64)-1;

	if (!m_bPartitionTableLoaded)
		return 0;

	return ListGetItemCount(m_pListPartition);
}

QWORD CVHMPartition::GetFirstPartitionID()
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return INVALID_PARTITION_ID;

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	pList = ListGetHead(m_pListPartition);
	pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);

	return pPartitionDescriptor->PartitionID;
}

QWORD CVHMPartition::GetPrevPartitionID(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return INVALID_PARTITION_ID;

	PARTITION_DESCRIPTOR *pPartitionDescriptor;
	pPartitionDescriptor = (PARTITION_DESCRIPTOR *)GetPrevPartitionDescriptor(qwPartitionID);

	if (!pPartitionDescriptor)
		return INVALID_PARTITION_ID;

	return pPartitionDescriptor->PartitionID;
}

QWORD CVHMPartition::GetNextPartitionID(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return INVALID_PARTITION_ID;

	PARTITION_DESCRIPTOR *pPartitionDescriptor;
	pPartitionDescriptor = (PARTITION_DESCRIPTOR *)GetNextPartitionDescriptor(qwPartitionID);

	if (!pPartitionDescriptor)
		return INVALID_PARTITION_ID;

	return pPartitionDescriptor->PartitionID;
}

QWORD CVHMPartition::GetLastPartitionID()
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return INVALID_PARTITION_ID;

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	pList = ListGetTail(m_pListPartition);
	pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);

	return pPartitionDescriptor->PartitionID;
}

CVHMFilesystem *CVHMPartition::GetVHMFilesystem(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return nullptr;

	PARTITION_DESCRIPTOR *pPartitionDescriptor;
	pPartitionDescriptor = GetPartitionDescriptor(qwPartitionID);

	if (!pPartitionDescriptor)
		return nullptr;

	return pPartitionDescriptor->pVHMFilesystem;
}

int CVHMPartition::GetPartitionDescriptor(QWORD qwPartitionID, PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	if (!pPartitionDescriptor)
		return VHM_ERROR_INVALID_PARAMETER;

	if (!IsOpened() || !m_bPartitionTableLoaded)
	{
		pPartitionDescriptor = nullptr;
		return VHM_ERROR_NOT_OPENED;
	}

	PARTITION_DESCRIPTOR *pPartitionDescriptorInternal;
	pPartitionDescriptorInternal = GetPartitionDescriptor(qwPartitionID);
	if (!pPartitionDescriptorInternal)
		return VHM_ERROR_INVALID_PARAMETER;

	pPartitionDescriptor->PartitionID = pPartitionDescriptorInternal->PartitionID;
	pPartitionDescriptor->StartLBA = pPartitionDescriptorInternal->StartLBA;
	pPartitionDescriptor->SectorCountLBA = pPartitionDescriptorInternal->SectorCountLBA;
	pPartitionDescriptor->AttributeGPT = pPartitionDescriptorInternal->AttributeGPT;
	pPartitionDescriptor->AttributeMBR = pPartitionDescriptorInternal->AttributeMBR;
	pPartitionDescriptor->pVHMFilesystem = pPartitionDescriptorInternal->pVHMFilesystem;

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::Destroy()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	/* VHMFilesystem ��ε� */
	UnloadFilesystem();

	/* ��Ƽ�� ���̺� ��ε� */
	UnloadPartition();

	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;

	/* VHMIOWrapper ��ε� */
	
	m_bOpened = FALSE;
	m_pVHMIOWrapper = nullptr;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	if (!IsOpened() || !m_pVHMIOWrapper)
		return -1;

	return m_pVHMIOWrapper->ReadOneSector(uiSectorLBA, pBuffer, szBuffer);

}
int CVHMPartition::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	if (!IsOpened() || !m_pVHMIOWrapper)
		return -1;

	return m_pVHMIOWrapper->WriteOneSector(uiSectorLBA, pBuffer);
}

int CVHMPartition::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened() || !m_pVHMIOWrapper)
		return -1;

	return m_pVHMIOWrapper->ReadSector(uiStartingLBA, uiSectorCount, pBuffer, szBuffer);
}

int CVHMPartition::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if(!IsOpened() || !m_pVHMIOWrapper)
		return -1;

	return m_pVHMIOWrapper->WriteSector(uiStartingLBA, uiSectorCount, pBuffer);
}

DWORD CVHMPartition::GetSectorSize()
{
	if(!IsOpened() || !m_pVHMIOWrapper)
		return -1;

	return m_pVHMIOWrapper->GetSectorSize();
}

QWORD CVHMPartition::GetSectorCount()
{
	if(!IsOpened() || !m_pVHMIOWrapper)
		return -1;

	return m_pVHMIOWrapper->GetSectorCount();
}

int CVHMPartition::GPTLoadPartition()
{
	if(!IsOpened())
		return -1;

	if(m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	// DeterminePartitioningType�� ���� GPT�� Ȯ�εǾ����Ƿ� Ȯ�� �۾��� ���� ����
	// Primary GPT ����� ��Ƽ�� ��Ʈ�� �б�

	BYTE *pLBA1;
	BYTE *pLBALast;
	GPT_HEADER *pGPTHeaderPrimary;
	GPT_HEADER *pGPTHeaderBackup;
	BYTE *pSectorGPTPartitionEntries;
	GPT_PARTITION_ENTRY *pGPTPartitionEntries;
	GPT_PARTITION_ENTRY *pGPTPartitionEntryTemp;
	DWORD dwSectorSize;
	DWORD dwSectorCountOfPartitionEntries;
	DWORD i;

	VHMPARTITION_INTERNAL_GPT *pGPTInternalData;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	LIST_HANDLE pListHandle;
	ListCreate(&pListHandle);

	// GPT ��� �б�
	dwSectorSize = GetSectorSize();
	pLBA1 = (BYTE *) malloc(dwSectorSize);
	ReadSector(1, 1, pLBA1, dwSectorSize);
	pGPTHeaderPrimary = (GPT_HEADER *) pLBA1;
	pLBALast = (BYTE *) malloc(dwSectorSize);
	ReadSector(pGPTHeaderPrimary->AlternateLBA, 1, pLBALast, dwSectorSize);
	pGPTHeaderBackup = (GPT_HEADER *) pLBALast;

	// GPT ��Ƽ�� ��Ʈ�� �б�
	dwSectorCountOfPartitionEntries = CVHMPartition::GPTCalculatePartitionEntrySectorCount(pGPTHeaderPrimary->SizeOfPartitionEntry, pGPTHeaderPrimary->NumberOfPartitionEntries, dwSectorSize);
	pSectorGPTPartitionEntries = (BYTE *) malloc(dwSectorCountOfPartitionEntries * dwSectorSize);
	ReadSector(2, dwSectorCountOfPartitionEntries, pSectorGPTPartitionEntries, dwSectorCountOfPartitionEntries * dwSectorSize);
	pGPTPartitionEntries = (GPT_PARTITION_ENTRY *) pSectorGPTPartitionEntries;

	// pGPTInternalData�� ����
	pGPTInternalData = (VHMPARTITION_INTERNAL_GPT *) malloc (sizeof(VHMPARTITION_INTERNAL_GPT));
	pGPTInternalData->dwSectorSize = dwSectorSize;
	pGPTInternalData->dwGPTPartitionEntrySize = pGPTHeaderPrimary->SizeOfPartitionEntry;
	pGPTInternalData->dwGPTPartitionEntryCount = pGPTHeaderPrimary->NumberOfPartitionEntries;
	pGPTInternalData->pGPTHeaderPrimary = pGPTHeaderPrimary;
	pGPTInternalData->pGPTHeaderBackup = pGPTHeaderBackup;
	pGPTInternalData->pGPTPartitionEntries = pGPTPartitionEntries;

	// GPT ��Ƽ�� ��Ʈ������ ��Ƽ�� ������ �����Ͽ� ����Ʈ ���·� ����
	for(i=0; i<pGPTInternalData->dwGPTPartitionEntryCount; i++)
	{
		pGPTPartitionEntryTemp = CVHMPartition::GPTGetPartitionEntryPointer(pGPTInternalData, i);

		/* �ش� GPT ��Ƽ�� ��Ʈ���� ��� ������ �ǳʶ� */
		if(CompareUuid(pGPTPartitionEntryTemp->PartitionTypeGUID, GPT_PARTUUID_FREE) == TRUE)
			continue;

		/* �� GPT ��Ƽ�� ��Ʈ���� �ƴ϶�� ����Ʈ�� �߰� */
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *) malloc(sizeof(PARTITION_DESCRIPTOR));
		pPartitionDescriptor->StartLBA = pGPTPartitionEntryTemp->StartingLBA;
		pPartitionDescriptor->SectorCountLBA = (pGPTPartitionEntryTemp->EndingLBA + 1) - pGPTPartitionEntryTemp->StartingLBA;
		pPartitionDescriptor->AttributeGPT = pGPTPartitionEntryTemp->Attributes;
		pPartitionDescriptor->AttributeMBR = (pPartitionDescriptor->AttributeGPT & GPT_PART_ATTR_BIOSBOOTABLE) ? 0x80 : 0x00;
		pPartitionDescriptor->pVHMIOWrapper = nullptr;
		pPartitionDescriptor->pVHMFilesystem = nullptr;

		ListInsertAtTail(pListHandle, pPartitionDescriptor);

		CString str;
		str.Format(_T("StartingLBA = %I64d, EndingLBA = %I64d"), pGPTPartitionEntryTemp->StartingLBA, pGPTPartitionEntryTemp->EndingLBA);
		AfxMessageBox(str);
	}

	m_pPartitioningTypeSpecificData = (POINTER) pGPTInternalData;
	m_qwPartitioningTypeSpecificDataSize = sizeof(VHMPARTITION_INTERNAL_GPT);
	m_pListPartition = pListHandle;

	m_bPartitionTableLoaded = TRUE;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::GPTUnloadPartition()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UnloadFilesystem();
	
	if(!m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	/* GPT ���� �Ҵ� �޸� ���� */
	VHMPARTITION_INTERNAL_GPT *pGPTInternalData = (VHMPARTITION_INTERNAL_GPT *) m_pPartitioningTypeSpecificData;
	free(pGPTInternalData->pGPTHeaderPrimary);
	free(pGPTInternalData->pGPTHeaderBackup);
	free(pGPTInternalData->pGPTPartitionEntries);
	free(pGPTInternalData);

	m_pPartitioningTypeSpecificData = nullptr;
	m_qwPartitioningTypeSpecificDataSize = 0;

	/* ��Ƽ�� ���̺� ��ε� */
	UnloadPartitionDescriptor();

	m_bPartitionTableLoaded = FALSE;
	
	return VHM_ERROR_SUCCESS;
}

DWORD CVHMPartition::GPTCalculatePartitionEntrySectorCount(DWORD dwSizeOfPartitionEntry, DWORD dwNumberOfPartitionEntries, DWORD dwSectorSize)
{
	return RoundUp(dwSizeOfPartitionEntry * dwNumberOfPartitionEntries, dwSectorSize) / dwSectorSize;
}

GPT_PARTITION_ENTRY *CVHMPartition::GPTGetPartitionEntryPointer(GPT_HEADER *pGPTHeader, GPT_PARTITION_ENTRY *pGPTPartitionEntryStartAddress, DWORD dwIndex)
{
	BYTE *pBytePointer = (BYTE *) pGPTPartitionEntryStartAddress;
	pBytePointer += pGPTHeader->SizeOfPartitionEntry * dwIndex;
	return (GPT_PARTITION_ENTRY *) pBytePointer;
}

GPT_PARTITION_ENTRY *CVHMPartition::GPTGetPartitionEntryPointer(VHMPARTITION_INTERNAL_GPT *pGPTInternalData, DWORD dwIndex)
{
	return CVHMPartition::GPTGetPartitionEntryPointer(pGPTInternalData->pGPTHeaderPrimary, pGPTInternalData->pGPTPartitionEntries, dwIndex);
}

int CVHMPartition::MBRLoadPartition()
{
	if(!IsOpened())
		return -1;

	if(m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	// DeterminePartitioningType�� ���� MBR�� Ȯ�εǾ����Ƿ� Ȯ�� �۾��� ���� ����
	
	DWORD dwSectorSize;
	QWORD qwSectorCount;
	DWORD i;
	MBR_STRUCTURE *pMBR;
	MBR_PARTITION_ENTRY *pMBRPartitionEntry;

	VHMPARTITION_INTERNAL_MBR *pMBRInternalData;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	LIST_HANDLE pListHandle;
	ListCreate(&pListHandle);

	// ���� �ε�
	dwSectorSize = GetSectorSize();
	qwSectorCount = GetSectorCount();

	// MBR �б�
	BYTE *pLBA0 = (BYTE *) malloc(dwSectorSize);
	ReadSector(0, 1, pLBA0, dwSectorSize);
	pMBR = (MBR_STRUCTURE *) pLBA0;

	// pMBRInternalData�� ����
	pMBRInternalData = (VHMPARTITION_INTERNAL_MBR *) malloc (sizeof(VHMPARTITION_INTERNAL_MBR));
	pMBRInternalData->dwSectorSize = dwSectorSize;
	pMBRInternalData->pMBR = pMBR;

	// MBR�� �� ��Ƽ�� ��Ʈ�� 4�� �м�
	// Ȯ�� ��Ƽ�� �����̳ʰ� �߰ߵǸ� ���� ó��
	for(i=0; i<4; i++)
	{
		pMBRPartitionEntry = &pMBR->mPartitionEntry[i];

		/* �ش� MBR ��Ƽ�� ��Ʈ���� ��ȿ�� �˻� */
		if(CVHMPartition::MBRIsPartitionEntryValid(pMBRPartitionEntry, qwSectorCount))
		{
			pPartitionDescriptor = (PARTITION_DESCRIPTOR *) malloc(sizeof(PARTITION_DESCRIPTOR));
			pPartitionDescriptor->StartLBA = pMBRPartitionEntry->dwFirstSectorLBA;
			pPartitionDescriptor->SectorCountLBA = pMBRPartitionEntry->dwSectorCountLBA;
			pPartitionDescriptor->AttributeGPT = (pMBRPartitionEntry->bBootable & 0x80) ? GPT_PART_ATTR_BIOSBOOTABLE : 0;
			pPartitionDescriptor->AttributeMBR = pMBRPartitionEntry->bBootable;
			pPartitionDescriptor->pVHMIOWrapper = nullptr;
			pPartitionDescriptor->pVHMFilesystem = nullptr;

			ListInsertAtTail(pListHandle, pPartitionDescriptor);

			CString str;
			str.Format(_T("StartLBA = %I64d, SectorCountLBA = %I64d"), pPartitionDescriptor->StartLBA, pPartitionDescriptor->SectorCountLBA);
			AfxMessageBox(str);
		}
	}

	m_pPartitioningTypeSpecificData = (POINTER) pMBRInternalData;
	m_qwPartitioningTypeSpecificDataSize = sizeof(VHMPARTITION_INTERNAL_MBR);
	m_pListPartition = pListHandle;

	m_bPartitionTableLoaded = TRUE;
	
	return VHM_ERROR_SUCCESS;
}

BOOL CVHMPartition::MBRIsPartitionEntryValid(MBR_PARTITION_ENTRY *pPartitionEntry, QWORD qwSectorCount)
{
	if(!pPartitionEntry)
		return FALSE;

	if(!pPartitionEntry->dwFirstSectorLBA || !pPartitionEntry->dwSectorCountLBA)
		return FALSE;

	// ������ ���Ͱ� ��ũ �ۿ� �ְų� 32��Ʈ ��巹�� ü�踦 ����� �� ��
	QWORD qwLastSectorPlus1 = pPartitionEntry->dwFirstSectorLBA;
	qwLastSectorPlus1 += pPartitionEntry->dwSectorCountLBA;
	if(qwLastSectorPlus1 > qwSectorCount || qwLastSectorPlus1 >= TwoExp32)
		return FALSE;

	return TRUE;
}

int CVHMPartition::MBRUnloadPartition()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UnloadFilesystem();
	
	if(!m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	/* MBR ���� �Ҵ� �޸� ���� */
	VHMPARTITION_INTERNAL_MBR *pMBRInternalData = (VHMPARTITION_INTERNAL_MBR *) m_pPartitioningTypeSpecificData;
	free(pMBRInternalData->pMBR);
	free(pMBRInternalData);

	m_pPartitioningTypeSpecificData = nullptr;
	m_qwPartitioningTypeSpecificDataSize = 0;

	/* ��Ƽ�� ���̺� ��ε� */
	UnloadPartitionDescriptor();

	m_bPartitionTableLoaded = FALSE;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::LoadPartition()
{
	int err_code;

	/* �Ǵܵ� ��Ƽ�Ŵ� Ÿ�Կ� �´� ��Ƽ�� ���̺��� �ε�*/

	switch (m_dwPartitioningType)
	{
	case PART_TYPE_GPT:
		err_code = GPTLoadPartition();
		break;
	case PART_TYPE_MBR:
		err_code = MBRLoadPartition();
		break;
	default:
		// ��Ƽ�Ŵ� Ÿ���� ���� �������� �ʰų� ��ũ�� �ʱ�ȭ���� ����
		return VHM_ERROR_UNSUPPORTED;
	}

	if (err_code != VHM_ERROR_SUCCESS)
		return err_code;

	/* ��Ƽ�ǿ� ID �ο� */
	InitIDGenerator();
	GenerateIDFirst();

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::UnloadPartition()
{
	DestroyIDGenerator();

	switch(m_dwPartitioningType)
	{
	case PART_TYPE_GPT:
		return GPTUnloadPartition();
	case PART_TYPE_MBR:
		return MBRUnloadPartition();
	}

	/* ��Ƽ�� ���̺��� �ε���� ���� ���� */
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::UnloadPartitionDescriptor()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UnloadFilesystem();

	if(!m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	if(!m_pListPartition)
		return VHM_ERROR_SUCCESS;

	PARTITION_DESCRIPTOR *pPartitionDescriptor;
	LIST_STRUCT pListTemp;

	for(;;)
	{
		/* ��Ƽ�� ����Ʈ�� ������� ���� */
		if(ListIsEmpty(m_pListPartition))
			break;

		/* ����Ʈ�� �� ���� �����͸� ���� */
		pListTemp = ListGetHead(m_pListPartition);
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *) ListGetAt(m_pListPartition, pListTemp);

		/* �ش� CVHMFilesystem ��ü�� �̹� ������ �����̹Ƿ� �� ��Ƽ�� ������ �޸� �Ҵ� ���� */
		free(pPartitionDescriptor);
		ListRemoveHead(m_pListPartition);
	}

	ListDestroy(m_pListPartition);
	m_pListPartition = nullptr;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::InitIDGenerator()
{
	if (!IsOpened())
		return VHM_ERROR_GENERIC;

	if (IsIDGeneratorInitialized())
		return VHM_ERROR_SUCCESS;

	m_qwLastAllocatedPartitionID = INVALID_PARTITION_ID;
	m_bIDGeneratorInit = TRUE;

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::DestroyIDGenerator()
{
	if (!IsOpened())
		return VHM_ERROR_GENERIC;

	if (!IsIDGeneratorInitialized())
		return VHM_ERROR_SUCCESS;

	m_bIDGeneratorInit = FALSE;
	return VHM_ERROR_SUCCESS;
}

BOOL CVHMPartition::IsIDGeneratorInitialized()
{
	return m_bIDGeneratorInit;
}

int CVHMPartition::GenerateIDFirst()
{
	if (!IsIDGeneratorInitialized() || !m_bPartitionTableLoaded)
		return VHM_ERROR_GENERIC;

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	for (pList = ListGetHead(m_pListPartition); pList != nullptr; pList = ListGetNext(pList))
	{
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		pPartitionDescriptor->PartitionID = GenerateID();
	}

	return VHM_ERROR_SUCCESS;
}

QWORD CVHMPartition::GenerateID()
{
	if (!IsIDGeneratorInitialized() || !m_bPartitionTableLoaded)
		return INVALID_PARTITION_ID;

	UINT64 uiPartitionCount;
	QWORD i;

	uiPartitionCount = ListGetItemCount(m_pListPartition);

	if (m_qwLastAllocatedPartitionID < PARTITION_ID_MAX)
	{
		/* ������ �Ҵ��� ��Ƽ�� ID���� ��� ���� ���θ� �˻� */
		for (i = m_qwLastAllocatedPartitionID; i <= PARTITION_ID_MAX; ++i)
		{
			if (IsIDAvailable(i))
			{
				m_qwLastAllocatedPartitionID = i;
				return i;
			}
		}
		for (i = 0; i < m_qwLastAllocatedPartitionID; ++i)
		{
			if (IsIDAvailable(i))
			{
				m_qwLastAllocatedPartitionID = i;
				return i;
			}
		}
	}
	else
	{
		for (i = 0; i <= PARTITION_ID_MAX; ++i)
		{
			if (IsIDAvailable(i))
			{
				m_qwLastAllocatedPartitionID = i;
				return i;
			}
		}
	}

	return INVALID_PARTITION_ID;
}

BOOL CVHMPartition::IsIDAvailable(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return FALSE;

	/* ��Ƽ�� ID�� ��ĵ */

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;
	
	for (pList = ListGetHead(m_pListPartition); pList != nullptr; pList = ListGetNext(pList))
	{
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		if (pPartitionDescriptor->PartitionID == qwPartitionID)
			return FALSE;
	}

	return TRUE;
}

PARTITION_DESCRIPTOR *CVHMPartition::GetPartitionDescriptor(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return nullptr;

	/* ��Ƽ�� ID�� ��ĵ */

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	for (pList = ListGetHead(m_pListPartition); pList != nullptr; pList = ListGetNext(pList))
	{
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		if (pPartitionDescriptor->PartitionID == qwPartitionID)
			return pPartitionDescriptor;
	}

	return nullptr;
}

PARTITION_DESCRIPTOR *CVHMPartition::GetPrevPartitionDescriptor(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return nullptr;

	/* ��Ƽ�� ID�� ��ĵ */

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	for (pList = ListGetHead(m_pListPartition); pList != nullptr; pList = ListGetNext(pList))
	{
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		if (pPartitionDescriptor->PartitionID == qwPartitionID)
		{
			pList = ListGetPrev(pList);
			if (!pList) return nullptr;
			return (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		}
	}

	return nullptr;
}

PARTITION_DESCRIPTOR *CVHMPartition::GetNextPartitionDescriptor(QWORD qwPartitionID)
{
	if (!IsOpened() || !m_bPartitionTableLoaded)
		return nullptr;

	/* ��Ƽ�� ID�� ��ĵ */

	LIST_STRUCT pList;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	for (pList = ListGetHead(m_pListPartition); pList != nullptr; pList = ListGetNext(pList))
	{
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		if (pPartitionDescriptor->PartitionID == qwPartitionID)
		{
			pList = ListGetNext(pList);
			if (!pList) return nullptr;
			return (PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pList);
		}
	}

	return nullptr;
}

QWORD CVHMPartition::GetPartitionID(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	if (!pPartitionDescriptor)
		return INVALID_PARTITION_ID;

	return pPartitionDescriptor->PartitionID;
}

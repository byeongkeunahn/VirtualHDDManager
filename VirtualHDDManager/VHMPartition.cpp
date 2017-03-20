
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

	/* 디스크 서명 검사 (0xAA55) */
	if(pMBR->wBootSignature != DISK_SIGNATURE)
	{
		free(pLBA0);
		return FALSE;
	}

	/* 디스크의 Protective MBR 존재 여부 확인 */
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

	/* 디스크의 GPT 헤더 확인 */

	pLBA1 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(qwGPTStartLBA, 1, pLBA1, dwSectorSize);
	pGPTHeaderPrimary = (GPT_HEADER *) pLBA1;

	pLBALast = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(pGPTHeaderPrimary->AlternateLBA, 1, pLBALast, dwSectorSize);
	pGPTHeaderBackup = (GPT_HEADER *) pLBALast;

	/* Primary GPT Header와 Backup GPT Header 중 하나라도 올바르지 않으면 GPT 디스크가 아닌 것으로 간주 */

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

	/* GPT 헤더 서명 확인 */
	if(pGPTHeader->Signature != GPT_HEADER_SIGNATURE)
		return FALSE;
	
	/* GPT 헤더의 MyLBA가 이 헤더의 LBA를 가리키는 지 확인 */
	if(pGPTHeader->MyLBA != qwLBA)
		return FALSE;

	/* GPT 헤더의 CRC32 값을 0으로 놓고 CRC32를 계산하여 원래의 값과 일치하는 지 확인 */
	dwTemp = pGPTHeader->HeaderCRC32;
	pGPTHeader->HeaderCRC32 = 0;

	dwCRC32 = CRC32Calc((BYTE *) pGPTHeader, sizeof(GPT_HEADER), 0);
	pGPTHeader->HeaderCRC32 = dwTemp;

	if(dwTemp != dwCRC32)
		return FALSE;

	/* 섹터 크기, GPT 파티션 엔트리의 크기, GPT 파티션 엔트리의 수를 지역 변수에 설정 및 유효성 검사 (범위) */
	dwSectorSize = pVHMIOWrapper->GetSectorSize();
	dwSizeOfPartitionEntry = pGPTHeader->SizeOfPartitionEntry;
	dwNumberOfPartitionEntries = pGPTHeader->NumberOfPartitionEntries;

	if(!dwSizeOfPartitionEntry || dwSizeOfPartitionEntry < 128 || !IsPowerOfTwo(dwSizeOfPartitionEntry))
		return FALSE;

	dwSectorCountOfPartitionEntries = CVHMPartition::GPTCalculatePartitionEntrySectorCount(dwSizeOfPartitionEntry, dwNumberOfPartitionEntries, dwSectorSize);
	if(!dwSectorCountOfPartitionEntries)
		return FALSE;

	/* GPT 파티션 엔트리의 CRC32를 계산하여 GPT 헤더에 저장된 값과 비교 */
	pGPTPartitionEntries = (BYTE *) malloc(dwSectorCountOfPartitionEntries * dwSectorSize);
	if(bFirst)
		qwPartitionEntryStartLBA = qwLBA + 1;
	else
		qwPartitionEntryStartLBA = qwLBA - dwSectorCountOfPartitionEntries;

	/* GPT 헤더의 파티션 엔트리 시작 LBA 주소와 계산된 LBA 주소가 일치해야 함 */
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

	/* 디스크 정보 가져오기 */
	dwSectorSize = pVHMIOWrapper->GetSectorSize();
	
	/* 메모리 할당 및 MBR 읽기 */
	pLBA0 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadSector(0, 1, pLBA0, dwSectorSize);
	pMBR = (MBR_STRUCTURE *) pLBA0;

	/* 디스크 서명 확인 */
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
	/* pVHMIOWrapper가 유효하고, 이 VHMPartition 객체가 다른 디스크의 파티션을 로드한 상태이면 안 됨 */
	if(!pVHMIOWrapper)
		return VHM_ERROR_INVALID_VHMDISK;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	/* VHMIOWrapper를 로드 (멤버변수에 값 설정) */
	m_pVHMIOWrapper = pVHMIOWrapper;
	m_bOpened = TRUE;

	/* 파티셔닝 타입을 판별하고 LoadPartition 함수를 호출 */
	m_dwPartitioningType = CVHMPartition::DeterminePartitioningType(pVHMIOWrapper);
	if(LoadPartition() != VHM_ERROR_SUCCESS)
		return VHM_ERROR_SUCCESS; /* 인식된 파티셔닝 타입이 없으면 파일시스템 로드가 불가능 */

	/* VHMFilesystem 로드 */
	LoadFilesystem();

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::LoadFilesystem()
{
	/* VHMIOWrapper 객체가 유효해야 함*/
	if(!IsOpened())
		return -1;

	/* 파티션 테이블이 로드되어 있어야 함*/
	if(!m_bPartitionTableLoaded || !m_pListPartition)
		return -1;

	/* 파티션 리스트에서 파티션 정보를 읽어 파일시스템을 로드하고 다음 파티션으로 넘어감 */
	/* 파티션 리스트의 끝에 도달할 때까지 반복 */
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
	/* VHMDisk가 유효하지 않으면 언로드할 수 없음 (파일시스템이 로드되지 않음) */
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	/* 파티션 테이블이 로드되지 않았으면 언로드할 수 없음 (파일시스템이 로드되지 않음) */
	if(!m_bPartitionTableLoaded)
		return VHM_ERROR_SUCCESS;

	/* 파티션 테이블이 비었으면 언로드할 수 없음 (파일시스템이 로드되지 않음) */
	if(!m_pListPartition || ListIsEmpty(m_pListPartition))
		return VHM_ERROR_SUCCESS;

	/* 파티션 리스트에서 파티션 정보를 읽어 파일시스템을 언로드하고 다음 파티션으로 넘어감 */
	/* 파티션 리스트의 끝에 도달할 때까지 반복 */
	/* 오류가 발생하면 오류 여부를 iSuccess에 저장하고 계속 */
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

	/* 파일시스템 언로드 중 오류가 발생했으면 오류 리턴 */
	if(iSuccess & 0x01)
		return VHM_ERROR_INTERNAL;

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::LoadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	/* 매개변수의 유효성 검사*/
	if(!pPartitionDescriptor)
		return VHM_ERROR_INVALID_PARAMETER;

	/* 파일시스템이 이미 로드된 상태이면 로드할 수 없음*/
	if(pPartitionDescriptor->pVHMFilesystem != nullptr)
		return VHM_ERROR_ALREADY_OPENED;

	/* VHMIOWrapper 객체 생성 */
	CVHMIOWrapper *pVHMIOWrapper = new CVHMIOWrapper;
	if(pVHMIOWrapper->Initialize(m_pVHMIOWrapper, pPartitionDescriptor->StartLBA, pPartitionDescriptor->SectorCountLBA) != VHM_ERROR_SUCCESS)
	{
		delete pVHMIOWrapper;
		return -1;
	}

	/* 해당 파티션에 대하여 VHMFilesystem 객체를 생성하는 함수를 호출 */
	/* iSuccess는 오류 코드 -> 오류가 발생했으면 iSuccess를 리턴 */
	CVHMFilesystem *pVHMFilesystem = nullptr;
	int iSuccess = CVHMFilesystem::CreateVHMFilesystemObject(pVHMIOWrapper, &pVHMFilesystem);
	if(pVHMFilesystem == nullptr)
	{
		delete pVHMIOWrapper;
		return iSuccess;
	}

	/* 생성된 객체 지정 */
	pPartitionDescriptor->pVHMIOWrapper = pVHMIOWrapper;
	pPartitionDescriptor->pVHMFilesystem = pVHMFilesystem;
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::UnloadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	/* 매개변수의 유효성 검사 */
	if(!pPartitionDescriptor)
		return VHM_ERROR_INVALID_PARAMETER;

	/* 파일시스템이 로드되지 않은 상태이면 언로드한 이후의 상태와 같으므로 VHM_ERROR_SUCCESS 리턴 */
	if(pPartitionDescriptor->pVHMFilesystem == nullptr)
		return VHM_ERROR_SUCCESS;

	/* VHMFilesystem 객체를 파괴하는 함수를 호출 */
	/* iSuccess는 오류 코드 -> 오류가 발생했으면 iSuccess를 리턴 */
	int iSuccess;
	iSuccess = CVHMFilesystem::DestroyVHMFilesystemObject(pPartitionDescriptor->pVHMFilesystem);
	if(iSuccess != VHM_ERROR_SUCCESS)
		return iSuccess;

	/* VHMIOWrapper 객체 파괴 */
	delete pPartitionDescriptor->pVHMIOWrapper;

	/* 언로드한 객체들의 값을 nullptr로 설정 */
	pPartitionDescriptor->pVHMFilesystem = nullptr;
	pPartitionDescriptor->pVHMIOWrapper = nullptr;

	return VHM_ERROR_SUCCESS;
}

BOOL CVHMPartition::IsVHMFilesystemLoaded()
{
	/* VHMDisk가 유효하지 않은 상태에서는 파일시스템이 로드되어 있을 수 없음 */
	if(!IsOpened())
		return FALSE;
	
	/* 파티션 테이블이 유효하지 않은 상태에서는 파일시스템이 로드되어 있을 수 없음 */
	if(!m_bPartitionTableLoaded)
		return FALSE;

	/* 파티션 테이블이 비었다면 파일시스템이 로드되어 있을 수 없음 */
	if(ListIsEmpty(m_pListPartition))
		return FALSE;

	/* 파티션 테이블을 검색하여 파일시스템이 로드된 상태의 파티션을 1개 이상 발견하면 TRUE 리턴 */
	LIST_STRUCT pListCurrent = ListGetHead(m_pListPartition);
	for(;;)
	{
		if(IsVHMFilesystemLoaded((PARTITION_DESCRIPTOR *)ListGetAt(m_pListPartition, pListCurrent)))
			return TRUE;

		pListCurrent = ListGetNext(pListCurrent);
		if(pListCurrent == nullptr)
			break;
	}

	/* 파티션 테이블이 비어 있지 않으나 파일시스템이 로드된 파티션이 없음 */
	return FALSE;
}

BOOL CVHMPartition::IsVHMFilesystemLoaded(PARTITION_DESCRIPTOR *pPartitionDescriptor)
{
	/* 매개변수의 유효성 검사 */
	if(!pPartitionDescriptor)
		return FALSE;

	/* VHMFilesystem 객체가 nullptr로 설정되어 있으면 파일시스템이 로드되지 않은 상태 */
	if(pPartitionDescriptor->pVHMFilesystem == nullptr)
		return FALSE;

	return TRUE;
}

int CVHMPartition::InitializeDiskAsGPT()
{
	if(!IsOpened())
		return -1;

	/* 파일시스템과 파티션 테이블을 언로드 후 파티셔닝 타입을 '디스크가 초기화되지 않음' 상태로 변경*/
	UnloadFilesystem();

	if(m_bPartitionTableLoaded)
		UnloadPartition();

	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;

	/* 디스크를 GPT로 초기화 */
	DWORD dwSectorSize;
	QWORD qwSectorCount;

	dwSectorSize = m_pVHMIOWrapper->GetSectorSize();
	qwSectorCount = m_pVHMIOWrapper->GetSectorCount();

	m_dwPartitioningType = PART_TYPE_GPT;

	/* GPTLoadPartition() 이 -1을 리턴하는 일은 일어나서는 안 됨 */
	if(GPTLoadPartition() != VHM_ERROR_SUCCESS)
		return -1;

	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::InitializeDiskAsMBR()
{
	if(!IsOpened())
		return -1;
	
	/* 파일시스템과 파티션 테이블을 언로드 후 파티셔닝 타입을 '디스크가 초기화되지 않음' 상태로 변경*/
	UnloadFilesystem();

	if(m_bPartitionTableLoaded)
		UnloadPartition();
	
	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;

	/* 디스크를 MBR로 초기화 */
	DWORD dwSectorSize;
	BYTE *pLBA0;
	MBR_STRUCTURE *pMBR;
	DWORD i;

	/* 디스크의 LBA 0을 0으로 채우고 DISK_SIGNATURE를 지정 */
	dwSectorSize = m_pVHMIOWrapper->GetSectorSize();
	
	pLBA0 = (BYTE *) malloc(dwSectorSize);
	for(i=0; i<dwSectorSize; i++)
		pLBA0[i] = 0;

	pMBR = (MBR_STRUCTURE *) pLBA0;
	pMBR->wBootSignature = DISK_SIGNATURE;

	WriteSector(0, 1, pLBA0);

	free(pLBA0);

	m_dwPartitioningType = PART_TYPE_MBR;
	
	/* MBRLoadPartition() 이 -1을 리턴하는 일은 일어나서는 안 됨 */
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

	/* VHMFilesystem 언로드 */
	UnloadFilesystem();

	/* 파티션 테이블 언로드 */
	UnloadPartition();

	m_dwPartitioningType = PART_TYPE_UNINITIALIZED;

	/* VHMIOWrapper 언로드 */
	
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

	// DeterminePartitioningType에 의해 GPT가 확인되었으므로 확인 작업을 하지 않음
	// Primary GPT 헤더와 파티션 엔트리 읽기

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

	// GPT 헤더 읽기
	dwSectorSize = GetSectorSize();
	pLBA1 = (BYTE *) malloc(dwSectorSize);
	ReadSector(1, 1, pLBA1, dwSectorSize);
	pGPTHeaderPrimary = (GPT_HEADER *) pLBA1;
	pLBALast = (BYTE *) malloc(dwSectorSize);
	ReadSector(pGPTHeaderPrimary->AlternateLBA, 1, pLBALast, dwSectorSize);
	pGPTHeaderBackup = (GPT_HEADER *) pLBALast;

	// GPT 파티션 엔트리 읽기
	dwSectorCountOfPartitionEntries = CVHMPartition::GPTCalculatePartitionEntrySectorCount(pGPTHeaderPrimary->SizeOfPartitionEntry, pGPTHeaderPrimary->NumberOfPartitionEntries, dwSectorSize);
	pSectorGPTPartitionEntries = (BYTE *) malloc(dwSectorCountOfPartitionEntries * dwSectorSize);
	ReadSector(2, dwSectorCountOfPartitionEntries, pSectorGPTPartitionEntries, dwSectorCountOfPartitionEntries * dwSectorSize);
	pGPTPartitionEntries = (GPT_PARTITION_ENTRY *) pSectorGPTPartitionEntries;

	// pGPTInternalData에 대입
	pGPTInternalData = (VHMPARTITION_INTERNAL_GPT *) malloc (sizeof(VHMPARTITION_INTERNAL_GPT));
	pGPTInternalData->dwSectorSize = dwSectorSize;
	pGPTInternalData->dwGPTPartitionEntrySize = pGPTHeaderPrimary->SizeOfPartitionEntry;
	pGPTInternalData->dwGPTPartitionEntryCount = pGPTHeaderPrimary->NumberOfPartitionEntries;
	pGPTInternalData->pGPTHeaderPrimary = pGPTHeaderPrimary;
	pGPTInternalData->pGPTHeaderBackup = pGPTHeaderBackup;
	pGPTInternalData->pGPTPartitionEntries = pGPTPartitionEntries;

	// GPT 파티션 엔트리에서 파티션 정보를 추출하여 리스트 형태로 저장
	for(i=0; i<pGPTInternalData->dwGPTPartitionEntryCount; i++)
	{
		pGPTPartitionEntryTemp = CVHMPartition::GPTGetPartitionEntryPointer(pGPTInternalData, i);

		/* 해당 GPT 파티션 엔트리가 비어 있으면 건너뜀 */
		if(CompareUuid(pGPTPartitionEntryTemp->PartitionTypeGUID, GPT_PARTUUID_FREE) == TRUE)
			continue;

		/* 빈 GPT 파티션 엔트리가 아니라면 리스트에 추가 */
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

	/* GPT 내부 할당 메모리 해제 */
	VHMPARTITION_INTERNAL_GPT *pGPTInternalData = (VHMPARTITION_INTERNAL_GPT *) m_pPartitioningTypeSpecificData;
	free(pGPTInternalData->pGPTHeaderPrimary);
	free(pGPTInternalData->pGPTHeaderBackup);
	free(pGPTInternalData->pGPTPartitionEntries);
	free(pGPTInternalData);

	m_pPartitioningTypeSpecificData = nullptr;
	m_qwPartitioningTypeSpecificDataSize = 0;

	/* 파티션 테이블 언로드 */
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

	// DeterminePartitioningType에 의해 MBR이 확인되었으므로 확인 작업을 하지 않음
	
	DWORD dwSectorSize;
	QWORD qwSectorCount;
	DWORD i;
	MBR_STRUCTURE *pMBR;
	MBR_PARTITION_ENTRY *pMBRPartitionEntry;

	VHMPARTITION_INTERNAL_MBR *pMBRInternalData;
	PARTITION_DESCRIPTOR *pPartitionDescriptor;

	LIST_HANDLE pListHandle;
	ListCreate(&pListHandle);

	// 정보 로드
	dwSectorSize = GetSectorSize();
	qwSectorCount = GetSectorCount();

	// MBR 읽기
	BYTE *pLBA0 = (BYTE *) malloc(dwSectorSize);
	ReadSector(0, 1, pLBA0, dwSectorSize);
	pMBR = (MBR_STRUCTURE *) pLBA0;

	// pMBRInternalData에 대입
	pMBRInternalData = (VHMPARTITION_INTERNAL_MBR *) malloc (sizeof(VHMPARTITION_INTERNAL_MBR));
	pMBRInternalData->dwSectorSize = dwSectorSize;
	pMBRInternalData->pMBR = pMBR;

	// MBR의 주 파티션 엔트리 4개 분석
	// 확장 파티션 컨테이너가 발견되면 따로 처리
	for(i=0; i<4; i++)
	{
		pMBRPartitionEntry = &pMBR->mPartitionEntry[i];

		/* 해당 MBR 파티션 엔트리의 유효성 검사 */
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

	// 마지막 섹터가 디스크 밖에 있거나 32비트 어드레싱 체계를 벗어나면 안 됨
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

	/* MBR 내부 할당 메모리 해제 */
	VHMPARTITION_INTERNAL_MBR *pMBRInternalData = (VHMPARTITION_INTERNAL_MBR *) m_pPartitioningTypeSpecificData;
	free(pMBRInternalData->pMBR);
	free(pMBRInternalData);

	m_pPartitioningTypeSpecificData = nullptr;
	m_qwPartitioningTypeSpecificDataSize = 0;

	/* 파티션 테이블 언로드 */
	UnloadPartitionDescriptor();

	m_bPartitionTableLoaded = FALSE;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMPartition::LoadPartition()
{
	int err_code;

	/* 판단된 파티셔닝 타입에 맞는 파티션 테이블을 로드*/

	switch (m_dwPartitioningType)
	{
	case PART_TYPE_GPT:
		err_code = GPTLoadPartition();
		break;
	case PART_TYPE_MBR:
		err_code = MBRLoadPartition();
		break;
	default:
		// 파티셔닝 타입이 현재 지원되지 않거나 디스크가 초기화되지 않음
		return VHM_ERROR_UNSUPPORTED;
	}

	if (err_code != VHM_ERROR_SUCCESS)
		return err_code;

	/* 파티션에 ID 부여 */
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

	/* 파티션 테이블이 로드되지 않은 상태 */
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
		/* 파티션 리스트가 비었으면 멈춤 */
		if(ListIsEmpty(m_pListPartition))
			break;

		/* 리스트의 맨 앞쪽 데이터를 읽음 */
		pListTemp = ListGetHead(m_pListPartition);
		pPartitionDescriptor = (PARTITION_DESCRIPTOR *) ListGetAt(m_pListPartition, pListTemp);

		/* 해당 CVHMFilesystem 객체는 이미 삭제된 상태이므로 이 파티션 구조만 메모리 할당 해제 */
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
		/* 이전에 할당한 파티션 ID부터 사용 가능 여부를 검사 */
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

	/* 파티션 ID를 스캔 */

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

	/* 파티션 ID를 스캔 */

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

	/* 파티션 ID를 스캔 */

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

	/* 파티션 ID를 스캔 */

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

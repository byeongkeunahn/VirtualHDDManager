
#pragma once

FWDDEF_HANDLE(LIST_HANDLE);

class CVHMDisk;
class CVHMFilesystem;

class CVHMPartition
{
public:
	CVHMPartition();
	virtual ~CVHMPartition();

	// 파티셔닝 타입 판별
public:
	static DWORD DeterminePartitioningType(CVHMIOWrapper *pVHMIOWrapper);
protected:
	static BOOL DPT_GPT(CVHMIOWrapper *pVHMIOWrapper);
	static BOOL DPT_GPTHEADER(CVHMIOWrapper *pVHMIOWrapper, GPT_HEADER *pGPTHeader, QWORD qwLBA, BOOL bFirst);
	static BOOL DPT_MBR(CVHMIOWrapper *pVHMIOWrapper);


public:

	int Create(CVHMIOWrapper *pVHMIOWrapper); // CVHMPartition 초기화

	int LoadFilesystem(); // 파일시스템 로드
	int UnloadFilesystem(); // 파일시스템 언로드
	
	BOOL IsVHMFilesystemLoaded(); // 파일시스템이 로드된 파티션이 1개 이상 있으면 TRUE

	// 디스크 초기화 (파티션 테이블 생성)
	int InitializeDiskAsGPT();
	int InitializeDiskAsMBR();

	BOOL IsOpened(); // CVHMPartition이 초기화되었는지 확인

	DWORD GetPartitioningType(); // 파티셔닝 타입 얻기
	UINT64 GetPartitionCount(); // 파티션 개수 얻기

	QWORD GetFirstPartitionID(); // 처음 파티션 ID 얻기
	QWORD GetPrevPartitionID(QWORD qwPartitionID); // 다음 파티션 ID 얻기
	QWORD GetNextPartitionID(QWORD qwPartitionID); // 이전 파티션 ID 얻기
	QWORD GetLastPartitionID(); // 마지막 파티션 ID 얻기
	
	CVHMFilesystem *GetVHMFilesystem(QWORD qwPartitionID); // 파티션 ID로부터 VHMFilesystem 객체 얻기
	int GetPartitionDescriptor(QWORD qwPartitionID, PARTITION_DESCRIPTOR *pPartitionDescriptor); // 파티션 ID로부터 파티션 서술자 얻기

	int Destroy(); // CVHMPartition 파괴


protected:

	// VHMIOWrapper Wrapper 함수
	int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer);
	int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);
	DWORD GetSectorSize();
	QWORD GetSectorCount();


protected:
	
	// 파일시스템 관리
	BOOL IsVHMFilesystemLoaded(PARTITION_DESCRIPTOR *pPartitionDescriptor); // 특정 파티션의 파일시스템 로드 여부 검사
	int LoadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor); // 특정 파티션의 파일시스템 로드
	int UnloadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor); // 특정 파티션의 파일시스템 언로드


protected:

	// GPT
	int GPTLoadPartition();
	int GPTUnloadPartition();
	static DWORD GPTCalculatePartitionEntrySectorCount(DWORD dwSizeOfPartitionEntry, DWORD dwNumberOfPartitionEntries, DWORD dwSectorSize);
	static GPT_PARTITION_ENTRY *GPTGetPartitionEntryPointer(GPT_HEADER *pGPTHeader, GPT_PARTITION_ENTRY *pGPTPartitionEntryStartAddress, DWORD dwIndex);
	static GPT_PARTITION_ENTRY *GPTGetPartitionEntryPointer(VHMPARTITION_INTERNAL_GPT *pGPTInternalData, DWORD dwIndex);
	
	// MBR
	int MBRLoadPartition();
	int MBRUnloadPartition();
	static BOOL MBRIsPartitionEntryValid(MBR_PARTITION_ENTRY *pPartitionEntry, QWORD qwSectorCount);


	/* 공통 */


	// 파티션 로드 및 언로드

	int LoadPartition();
	int UnloadPartition();
	int UnloadPartitionDescriptor();

	// 파티션 ID 생성

	int InitIDGenerator();
	int DestroyIDGenerator();
	BOOL IsIDGeneratorInitialized();

	int GenerateIDFirst();
	QWORD GenerateID();
	BOOL IsIDAvailable(QWORD qwPartitionID);
	PARTITION_DESCRIPTOR *GetPartitionDescriptor(QWORD qwPartitionID);
	PARTITION_DESCRIPTOR *GetPrevPartitionDescriptor(QWORD qwPartitionID);
	PARTITION_DESCRIPTOR *GetNextPartitionDescriptor(QWORD qwPartitionID);
	QWORD GetPartitionID(PARTITION_DESCRIPTOR *pPartitionDescriptor);


protected:
	CVHMIOWrapper	*m_pVHMIOWrapper;
	LIST_HANDLE		m_pListPartition;
	BOOL			m_bOpened;
	BOOL			m_bPartitionTableLoaded;
	DWORD			m_dwPartitioningType;
	POINTER			m_pPartitioningTypeSpecificData;
	QWORD			m_qwPartitioningTypeSpecificDataSize;
	UINT64			m_uiFirstUsableLBA;
	UINT64			m_uiLastUsableLBA;

	BOOL			m_bIDGeneratorInit;
	QWORD			m_qwLastAllocatedPartitionID;
};


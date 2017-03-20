
#pragma once

FWDDEF_HANDLE(LIST_HANDLE);

class CVHMDisk;
class CVHMFilesystem;

class CVHMPartition
{
public:
	CVHMPartition();
	virtual ~CVHMPartition();

	// ��Ƽ�Ŵ� Ÿ�� �Ǻ�
public:
	static DWORD DeterminePartitioningType(CVHMIOWrapper *pVHMIOWrapper);
protected:
	static BOOL DPT_GPT(CVHMIOWrapper *pVHMIOWrapper);
	static BOOL DPT_GPTHEADER(CVHMIOWrapper *pVHMIOWrapper, GPT_HEADER *pGPTHeader, QWORD qwLBA, BOOL bFirst);
	static BOOL DPT_MBR(CVHMIOWrapper *pVHMIOWrapper);


public:

	int Create(CVHMIOWrapper *pVHMIOWrapper); // CVHMPartition �ʱ�ȭ

	int LoadFilesystem(); // ���Ͻý��� �ε�
	int UnloadFilesystem(); // ���Ͻý��� ��ε�
	
	BOOL IsVHMFilesystemLoaded(); // ���Ͻý����� �ε�� ��Ƽ���� 1�� �̻� ������ TRUE

	// ��ũ �ʱ�ȭ (��Ƽ�� ���̺� ����)
	int InitializeDiskAsGPT();
	int InitializeDiskAsMBR();

	BOOL IsOpened(); // CVHMPartition�� �ʱ�ȭ�Ǿ����� Ȯ��

	DWORD GetPartitioningType(); // ��Ƽ�Ŵ� Ÿ�� ���
	UINT64 GetPartitionCount(); // ��Ƽ�� ���� ���

	QWORD GetFirstPartitionID(); // ó�� ��Ƽ�� ID ���
	QWORD GetPrevPartitionID(QWORD qwPartitionID); // ���� ��Ƽ�� ID ���
	QWORD GetNextPartitionID(QWORD qwPartitionID); // ���� ��Ƽ�� ID ���
	QWORD GetLastPartitionID(); // ������ ��Ƽ�� ID ���
	
	CVHMFilesystem *GetVHMFilesystem(QWORD qwPartitionID); // ��Ƽ�� ID�κ��� VHMFilesystem ��ü ���
	int GetPartitionDescriptor(QWORD qwPartitionID, PARTITION_DESCRIPTOR *pPartitionDescriptor); // ��Ƽ�� ID�κ��� ��Ƽ�� ������ ���

	int Destroy(); // CVHMPartition �ı�


protected:

	// VHMIOWrapper Wrapper �Լ�
	int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer);
	int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);
	DWORD GetSectorSize();
	QWORD GetSectorCount();


protected:
	
	// ���Ͻý��� ����
	BOOL IsVHMFilesystemLoaded(PARTITION_DESCRIPTOR *pPartitionDescriptor); // Ư�� ��Ƽ���� ���Ͻý��� �ε� ���� �˻�
	int LoadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor); // Ư�� ��Ƽ���� ���Ͻý��� �ε�
	int UnloadFilesystem(PARTITION_DESCRIPTOR *pPartitionDescriptor); // Ư�� ��Ƽ���� ���Ͻý��� ��ε�


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


	/* ���� */


	// ��Ƽ�� �ε� �� ��ε�

	int LoadPartition();
	int UnloadPartition();
	int UnloadPartitionDescriptor();

	// ��Ƽ�� ID ����

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


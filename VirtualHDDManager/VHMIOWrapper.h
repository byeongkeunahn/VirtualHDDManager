
#pragma once

class CVHMDisk;
class CVHMPartition;

class CVHMIOWrapper
{
public:
	CVHMIOWrapper();
	virtual ~CVHMIOWrapper();

	BOOL IsInitialized();

	int Initialize(CVHMDisk *pVHMDisk, UINT64 uiStartingLBA, UINT64 uiSectorCount);
	int Initialize(CVHMIOWrapper *pVHMIOWrapper, UINT64 uiStartingLBA, UINT64 uiSectorCount);
	int Initialize(CVHMIOWrapper *pVHMIOWrapper, DWORD dwSectorSize, UINT64 uiStartingLBA, UINT64 uiSectorCount);
	int Uninitialize();

	int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer); // 섹터 1개 읽기
	int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer); // 섹터 1개 쓰기

	int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer); // 섹터 읽기 (LBA - nStartSector, 섹터 수 - nSectorCount)
	int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer); // 섹터 쓰기(LBA - nStartSector, 섹터 수 - nSectorCount)

	DWORD GetSectorSize(); // 섹터 크기 얻기
	QWORD GetSectorCount(); // 섹터 수 얻기

protected:
	BOOL			m_bInitialized; // 이 CVHMIOWrapper 객체가 올바른 정보로 잘 초기화되어 있으면 TRUE
	int				m_iIOObjectType; // I/O 객체의 종류
	POINTER			m_pIOObject; // I/O 객체의 포인터
	UINT64			m_uiStartingLBA; // 파티션의 시작 LBA 주소
	UINT64			m_uiSectorCount; // 파티션의 섹터 갯수
	DWORD			m_dwSectorSize; // 파티션의 섹터 크기
};

BOOL VHMIOWrapperIsObjectValid(CVHMIOWrapper *pVHMIOWrapper); // pVHMIOWrapper가 올바른 객체이면 TRUE


#pragma once

class CVHMIOWrapper;
class CVHMPartition;

class CVHMDisk
{
public:
	CVHMDisk(void);
	virtual ~CVHMDisk(void);

	virtual int OpenDisk(); // 기존 디스크 열기
	virtual int CloseDisk(); // 디스크 닫기

	virtual int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer); // 섹터 1개 읽기
	virtual int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer); // 섹터 1개 쓰기

	virtual int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer); // 섹터 읽기 (LBA - nStartSector, 섹터 수 - nSectorCount)
	virtual int WriteSector(UINT64 uiStartSector, UINT64 uiSectorCount, void *pBuffer); // 섹터 쓰기(LBA - nStartSector, 섹터 수 - nSectorCount)

	virtual int WipeSector(UINT64 uiStartSector, UINT64 uiSectorCount); // 주어진 섹터들을 0으로 채우기 (LBA - nStartSector, 섹터 수 - nSectorCount)
	virtual int WipeDisk(); // 디스크 전체를 0으로 채우기

	virtual BOOL IsOpened(); // 디스크가 열렸으면 TRUE
	virtual DWORD GetSectorSize(); // 디스크의 섹터 크기 구하기
	virtual QWORD GetSectorCount(); // 디스크의 섹터 수 구하기

	virtual DWORD GetPartitioningType(); // 파티셔닝 타입 얻기
	virtual UINT64 GetPartitionCount(); // 파티션 개수 얻기

	virtual QWORD GetFirstPartitionID(); // 처음 파티션 ID 얻기
	virtual QWORD GetPrevPartitionID(QWORD qwPartitionID); // 다음 파티션 ID 얻기
	virtual QWORD GetNextPartitionID(QWORD qwPartitionID); // 이전 파티션 ID 얻기
	virtual QWORD GetLastPartitionID(); // 마지막 파티션 ID 얻기

	virtual DWORD GetDiskType(); // 디스크 타입 얻기

	virtual CVHMIOWrapper *GetVHMIOWrapper(); // VHMIOWrapper 객체 얻기
	virtual CVHMPartition *GetVHMPartition(); // VHMPartition 객체 얻기

protected:
	virtual int InitializeVHMIOWrapper(); // VHMIOWrapper 객체 초기화
	virtual int UninitializeVHMIOWrapper(); // VHMIOWrapper 객체 언로드

	virtual int InitializeVHMPartition(); // VHMPartition 객체 초기화
	virtual int UninitializeVHMPartition(); // VHMPartition 객체 언로드

protected:
	BOOL		m_bOpened;
	DWORD		m_dwSectorSize;
	QWORD		m_qwSectorCount;
	CVHMPartition	*m_pVHMPartition;
	CVHMIOWrapper	*m_pVHMIOWrapper;
};


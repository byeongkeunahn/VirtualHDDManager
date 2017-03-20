
#pragma once

FWDDEF_HANDLE(HHandleID);
struct FAT32_FOBJ_DESC_INTERNAL;

class CVHMFilesystemFAT32 : public CVHMFilesystem
{
public:
	CVHMFilesystemFAT32();
	virtual ~CVHMFilesystemFAT32();
	
	// 파일시스템 생성/열기/닫기/수정
	virtual int CreateFilesystem(FILESYSTEM_INFORMATION *pFilesystemInformation); // 주어진 정보로부터 파일시스템 생성
	virtual int OpenFilesystem(); // 파일시스템 열기
	virtual int CloseFilesystem(); // 파일시스템 닫기
	virtual int CheckFilesystem(); // 파일시스템 확인 (chkdsk, fsck의 역할)

	// 파일시스템 수정
	virtual int ModifyFilesystemVolumeLabel(WCHAR *wszVolumeLabel, UINT64 uiLength); // 볼륨 레이블 수정
	virtual int ModifyFilesystemSize(UINT64 uiNewSectorCount); // 파티션 크기 조정
	virtual int ModifyFilesystemClusterSize(QWORD qwClusterSize); // 클러스터 크기 변경

	// 파일 I/O
	virtual int FileCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle); // pwPath 디렉터리에 pwName의 이름으로 크기 0의 파일 생성
	virtual int FileMove(QWORD qwHandle, const WCHAR *pwNewPath); // 파일을 다른 디렉터리로 옮김
	virtual int FileGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t szBuffer); // 파일 이름 얻기
	virtual int FileSetName(QWORD qwHandle, const WCHAR *pwNewName); // 파일 이름 바꾸기
	virtual QWORD FileGetAttributes(QWORD qwHandle); // 파일 속성 얻기
	virtual QWORD FileSetAttributes(QWORD qwHandle, QWORD qwNewAttributes); // 파일 속성 지정
	virtual QWORD FileGetAccessRights(QWORD qwHandle); // 파일 액세스 권한 얻기
	virtual QWORD FileSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights); // 파일 액세스 권한 지정
	virtual int FileGetSize(QWORD qwHandle, UINT64 *puiSize); // 파일 크기 얻기
	virtual int FileSetSize(QWORD qwHandle, UINT64 uiNewSize); // 파일 크기 설정 (원래 크기보다 작으면 뒤의 내용이 잘림)
	virtual int FileRead(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, UINT64 uiBufferSize, QWORD *pqwRead); // 읽기
	virtual int FileWrite(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, QWORD *pqwWrite); // 쓰기
	virtual int FileSetPointer(QWORD qwHandle, INT64 iDistanceToMove, UINT64 *puiNewFilePointer, QWORD qwMoveMethod); // 파일 포인터 옮기기
	virtual int FileDelete(QWORD qwHandle); // 파일 삭제 및 핸들 파괴
	virtual int FileClose(QWORD qwHandle); // 파일 닫기

	// 디렉터리 I/O
	virtual int DirCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle); // pwPath 디렉터리에 pwName의 이름으로 디렉터리 생성
	virtual int DirMove(QWORD qwHandle,  const WCHAR *pwNewPath); // 디렉터리를 다른 디렉터리로 옮김
	virtual int DirGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t szBuffer); // 폴더 이름 얻기
	virtual int DirSetName(QWORD qwHandle, const WCHAR *pwNewName); // 폴더 이름 바꾸기
	virtual QWORD DirGetAttributes(QWORD qwHandle); // 폴더 속성 얻기
	virtual QWORD DirSetAttributes(QWORD qwHandle, QWORD qwNewAttributes); // 폴더 속성 지정
	virtual QWORD DirGetAccessRights(QWORD qwHandle); // 폴더 액세스 권한 얻기
	virtual QWORD DirSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights); // 폴더 액세스 권한 지정
	virtual int DirGetFileCount(QWORD qwHandle, UINT64 *puiFileCount); // 디렉터리의 파일 갯수 얻기
	virtual int DirGetFolderCount(QWORD qwHandle, UINT64 *puiFileCount); // 디렉터리의 디렉터리 갯수 얻기
	virtual int DirRewind(QWORD qwHandle); // 디렉터리 포인터 리셋
	virtual int DirSeek(QWORD qwHandle, QWORD qwPosition); // 디렉터리 포인터 설정
	virtual QWORD DirTell(QWORD qwHandle, const FOBJECT_DESCRIPTOR *pFObjectDescriptor); // FOBJECT 항목 서술자에 대한 디렉터리 위치값을 얻음
	virtual const FOBJECT_DESCRIPTOR *DirEnumerateNext(QWORD qwHandle); // 디렉터리 항목 나열
	virtual int DirDelete(QWORD qwHandle); // 디렉터리 삭제 및 핸들 파괴
	virtual int DirClose(QWORD qwHandle); // 디렉터리 닫기

	// 특수 I/O
	virtual int WriteBootCode(const void *pBuffer); // 부트 코드 쓰기
	virtual int GetVolumeLabel(WCHAR *pwszBuffer, vhmsize_t ccBuffer); // 볼륨 레이블 얻기

	// VHMFilesystem 객체의 상태 얻기
	virtual QWORD GetFilesystemStatus(); // 파일시스템의 상태 얻기
	virtual int QueryFilesystemInformation(FILESYSTEM_INFORMATION *pFilesystemInformation); // 파일시스템 정보 조회

	// VHMFilesystem 객체의 특성 얻기
	virtual DWORD GetFilesystemType(); // 파일시스템 타입 얻기
	virtual DWORD GetFilesystemCharacteristics(); // 파일시스템 특성 얻기

protected:
	/* 내부 함수 */

	// 볼륨 정보 얻기
	DWORD GetSecPerClus();
	DWORD GetClusterSize();
	DWORD GetClusterCount();

	// 기타 함수
	BOOL IsBasicInformationLoaded();

	// 파일 및 폴더 핸들 관리
	int InitializeHandleManagement();	// 핸들 관리 루틴 초기화
	int UninitializeHandleManagement();	// 핸들 관리 루틴 언로드
	BOOL IsHandleManagementInitialized();	// 핸들 관리 루틴 초기화 여부 확인


	// 클러스터 및 FAT 영역 I/O
	int ReadCluster(DWORD dwStartCluster, DWORD dwClusterCount, BYTE *pBuffer);	// 클러스터 읽기
	int WriteCluster(DWORD dwStartCluster, DWORD dwClusterCount, BYTE *pBuffer); // 클러스터 쓰기
	int FindFreeCluster(DWORD *pdwIndex);	// 사용 가능한 클러스터 찾기 (실패 시 0xFFFFFFFF)
	int AllocateCluster(DWORD dwIndex);	// 클러스터 할당 및 EOF 표시 (사용 가능할 경우에만 가능)
	int FreeCluster(DWORD dwIndex);	// 클러스터 할당 해제 (사용 중일 때에만 가능)
	int LinkCluster(DWORD dwFirst, DWORD dwLast);	// dwFirst->dwLast 클러스터 연결
	int UnlinkCluster(DWORD dwIndex);	// 클러스터 연결 해제 (EOF 표시)
	int MarkAsBadCluster(DWORD dwIndex);	// 배드 클러스터로 표시
	int ReadFAT(DWORD dwIndex, DWORD *pdwValue);	// FAT 데이터 얻기
	int WriteFAT(DWORD dwIndex, DWORD dwValue);	// FAT 데이터 설정 (FAT 미러링/FAT 활성 모드 고려)
	int WriteFAT(DWORD dwFATIndex, DWORD dwIndex, DWORD dwValue); // 지정된 FAT에 데이터 기록
	int ClusterToFATLBA(DWORD dwClusterIndex, QWORD *pqwFATLBA, DWORD *pdwOffsetInSector);	// 클러스터 인덱스를 FAT의 몇 번째 섹터에 저장되어 있는가로 변환
	int ClusterIndexToLBA(DWORD dwClusterIndex, QWORD *pqwLBA);	// 클러스터 인덱스를 LBA로 변환
	int MarkHardwareError(); // 하드웨어 오류 발생을 표시

	// 파일 속성 변환 (VHM 표준 <--> FAT32)
	int AttrConvFAT32ToVhm(BYTE AttrFAT32, QWORD *pAttrVHM);
	int AttrConvVhmToFAT32(QWORD AttrVHM, BYTE *pAttrFAT32);

	// 디렉터리 엔트리 처리
	int GetDirItemInit(void *pDirEntBuf, QWORD qwDirEntBufSize, QWORD *pqwTempVal); // 디렉터리 항목 - 초기화
	int GetNextDirItem(void *pDirEntBuf, QWORD qwDirEntBufSize, FAT32_FOBJ_DESC_INTERNAL **ppFObject, QWORD *pqwTempVal); // 디렉터리 항목 - 다음 항목 얻기 (*ppFObject는 FreeMemory로 해제되어야 함)
	int GetNextValidDirEntry(void *pDirEntBuf, QWORD qwDirEntBufSize, QWORD *pqwCurPos, void **ppDirEnt);
	int NameLen83(void *pDirEnt, QWORD *pqwLen); // 디렉터리 엔트리 항목의 8.3 이름의 길이 계산
	int Name83ToVhm(void *pDirEnt, QWORD qwAttributes, WCHAR *pwOutBuf); // 8.3 이름을 VHM 표준으로 변환
	int NameLenLfn(void *pDirEnt, QWORD *pqwLen); // 디렉터리 엔트리 항목의 LFN 이름의 길이 계산
	int LoadVolumeLabel(); // 볼륨 레이블 로드

	// 파일 I/O
	
	// 디렉터리 I/O

protected:
	/* m_pLBA0, m_pFSInfo는 섹터 전체가 저장되어 있으므로 GetSectorSize()로 얻은 값의 크기의 버퍼임 */
	BYTE				*m_pLBA0;	// LBA 0
	FAT_BPB_BS_COMMON	*m_pFATCommonHeader;	// FAT 공통 헤더
	FAT32_BPB_BS		*m_pFAT32Header;	// FAT32 헤더
	BYTE				*m_pFSInfo;	// FSINFO 섹터

	/* 볼륨 정보 */
	QWORD				m_qwVolumeAttributes;	// 볼륨 정보
	QWORD				m_qwReservedSectorCount;	// 예약된 섹터 수
	QWORD				m_qwFATAreaStartSector;	// FAT 영역 시작 섹터 LBA
	QWORD				m_qwFATAreaSectorCount;	// FAT 영역 섹터 수
	DWORD				m_dwFATCount;	// FAT의 수
	DWORD				m_dwActiveFAT;	// 활성 FAT 번호
	QWORD				m_qwDataAreaStartSector;	// 데이터 영역 시작 섹터 LBA
	QWORD				m_qwDataAreaSectorCount;	// 데이터 영역 섹터 수
	DWORD				m_dwClusterSize;	// 클러스터 크기
	DWORD				m_dwClusterCount;	// 클러스터 수
	WCHAR				m_pwszVolumeLabel[12];	// 볼륨 레이블

	/* 내부 정보 */
	BOOL				m_bBasicInformationLoaded;	// 핸들 관리 루틴 외의 초기화의 완료 여부 (= 기본 정보 로드 여부)
	BOOL				m_bHandleManagementInitialized;	// 핸들 관리 루틴 초기화 여부
	HHandleID			m_hHandleID;	// 핸들 ID
};


#pragma once

class CVHMIOWrapper;

class CVHMFilesystem
{
public:
	CVHMFilesystem(void);
	virtual ~CVHMFilesystem(void);

	// 주어진 파티션의 파일시스템 타입을 판별하여 VHMFilesystem 객체를 생성
	static int CreateVHMFilesystemObject(CVHMIOWrapper *pVHMIOWrapper, CVHMFilesystem **ppVHMFilesystem);
protected:
	inline static int CreateVHMFilesystemFromExistingObject(CVHMFilesystem *pVHMFilesystem, CVHMIOWrapper *pVHMIOWrapper);

public:

	// VHMFilesystem 객체 파괴
	static int DestroyVHMFilesystemObject(CVHMFilesystem *pVHMFilesystem);

	// VHMIOWrapper 객체 지정
	virtual int SetVHMIOWrapper(CVHMIOWrapper *pVHMIOWrapper);

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
	virtual int FileGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer); // 파일 이름 얻기
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
	virtual int DirGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer); // 폴더 이름 얻기
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
	virtual BOOL IsVHMIOWrapperValid(); // 설정된 VHMIOWrapper 객체가 올바르면 TRUE
	virtual BOOL IsMounted(); // 파티션이 마운트되었으면 TRUE
	virtual QWORD GetFilesystemStatus(); // 파일시스템의 상태 얻기
	virtual int QueryFilesystemInformation(FILESYSTEM_INFORMATION *pFilesystemInformation); // 파일시스템 정보 조회

	// VHMFilesystem 객체의 특성 얻기
	virtual DWORD GetFilesystemType(); // 파일시스템 타입 얻기
	virtual DWORD GetFilesystemCharacteristics(); // 파일시스템 특성 얻기

protected:
	virtual int ReadSector(UINT64 nStartSector, UINT32 nSectorCount, void *pBuffer, UINT64 nBufferSize); // 섹터 읽기 (LBA - nStartSector, 섹터 수 - nSectorCount)
	virtual int WriteSector(UINT64 nStartSector, UINT32 nSectorCount, void *pBuffer); // 섹터 쓰기(LBA - nStartSector, 섹터 수 - nSectorCount)
	virtual DWORD GetSectorSize(); // 디스크의 섹터 크기 얻기
	virtual QWORD GetPartitionSectorCount(); // 파티션의 섹터 갯수 얻기

protected:
	BOOL			m_bVHMIOWrapperValid; // VHMIOWrapper 객체가 올바르면 TRUE
	CVHMIOWrapper	*m_pVHMIOWrapper; // VHMIOWrapper 객체
	BOOL			m_bMounted; // 파일시스템이 마운트되었으면 TRUE
};

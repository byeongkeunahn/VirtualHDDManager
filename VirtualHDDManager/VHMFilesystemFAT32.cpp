
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

CVHMFilesystemFAT32::CVHMFilesystemFAT32()
{
	m_bBasicInformationLoaded = FALSE;
	m_bHandleManagementInitialized = FALSE;
	m_hHandleID = nullptr;
}

CVHMFilesystemFAT32::~CVHMFilesystemFAT32()
{
}

int CVHMFilesystemFAT32::CreateFilesystem(FILESYSTEM_INFORMATION *pFilesystemInformation)
{
	/* VHMIOWrapper 객체의 유효성 검사 */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	return -1;
}

int CVHMFilesystemFAT32::OpenFilesystem()
{
	/* VHMIOWrapper 객체의 유효성 검사 */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	/* 파일시스템이 마운트되지 않았음을 확인 */
	if(IsMounted())
		return VHM_ERROR_ALREADY_OPENED;

	/* 파일시스템 마운트 */
	/* 내부 매개변수에 값 설정 */

	// LBA 0 관련 정보 저장
	BYTE *pLBA0 = (BYTE *) malloc(GetSectorSize());
	ReadSector(0, 1, pLBA0, GetSectorSize());

	m_pLBA0 = pLBA0;
	m_pFATCommonHeader = (FAT_BPB_BS_COMMON *) m_pLBA0;
	m_pFAT32Header = (FAT32_BPB_BS *) (m_pLBA0 + sizeof(FAT_BPB_BS_COMMON));
	
	// FSINFO 섹터 관련 정보 저장
	BYTE *pFSINFO = (BYTE *) malloc(GetSectorSize());
	ReadSector(m_pFAT32Header->BPB_FSInfo, 1, pFSINFO, GetSectorSize());
	m_pFSInfo = pFSINFO;

	// 볼륨 속성을 0으로 설정
	m_qwVolumeAttributes = 0;

	// FAT 정보 저장
	m_dwFATCount = m_pFATCommonHeader->BPB_NumFATs;
	m_qwVolumeAttributes &= (m_pFAT32Header->BPB_ExtFlags & FAT32_FAT_MODE_MASK)
		? FAT32_VOL_ATTR_FAT_MODE_ACTIVE : FAT32_VOL_ATTR_FAT_MODE_MIRRORING;
	m_dwActiveFAT = (m_pFAT32Header->BPB_ExtFlags & FAT32_FAT_MODE_MASK)
		? (m_pFAT32Header->BPB_ExtFlags & FAT32_FAT_MASK) : (DWORD)-1;

	// 예약된 섹터 수 저장
	m_qwReservedSectorCount = m_pFATCommonHeader->BPB_RsvdSecCnt;

	// FAT 영역 시작 섹터 LBA 저장
	m_qwFATAreaStartSector = m_pFATCommonHeader->BPB_RsvdSecCnt;

	// FAT 영역 섹터 수 저장
	m_qwFATAreaSectorCount = m_pFAT32Header->BPB_FATSz32 * m_pFATCommonHeader->BPB_NumFATs;

	// 데이터 영역 시작 섹터 LBA 저장
	QWORD qwFATAreaSectors;
	qwFATAreaSectors = m_pFAT32Header->BPB_FATSz32 * m_pFATCommonHeader->BPB_NumFATs;
	m_qwDataAreaStartSector = m_qwFATAreaStartSector + qwFATAreaSectors;

	// 데이터 영역 섹터 수 저장
	m_qwDataAreaSectorCount = m_pFATCommonHeader->BPB_TotSec32 - (m_qwReservedSectorCount + m_qwFATAreaSectorCount);

	// 클러스터 크기 저장
	m_dwClusterSize = GetSectorSize() * m_pFATCommonHeader->BPB_SecPerClus;

	// 클러스터 수 저장
	m_dwClusterCount = (DWORD) (m_qwDataAreaSectorCount / m_pFATCommonHeader->BPB_SecPerClus);

	// 기본 정보가 로드된 것으로 표시
	m_bBasicInformationLoaded = TRUE;

	// 핸들 관리 루틴 초기화
	InitializeHandleManagement();

	/* 파일시스템이 마운트된 것으로 표시 */
	m_bMounted = TRUE;

	// 볼륨 레이블 로드
	LoadVolumeLabel();

/*
	// 테스트 루틴 
	CFile f;
	f.Open(_T("vhmfat32.tst"), CFile::modeCreate | CFile::modeReadWrite);

	DWORD dwIndex = 2;
	DWORD dwClusterSize = GetClusterSize();
	BYTE *pBuffer = (BYTE *)malloc(dwClusterSize);

	for (;;)
	{
		ReadCluster(dwIndex, 1, pBuffer);
		f.Write(pBuffer, dwClusterSize);
		ReadFAT(dwIndex, &dwIndex);

		if ((dwIndex & FAT32_DATA_MASK_DATA) >= 0x0FFFFFF7)
			break;
	}

	free(pBuffer);

	QWORD qwRootSz;
	qwRootSz = f.GetLength();

	pBuffer = new BYTE[qwRootSz];
	f.Seek(0, CFile::begin);
	f.Read(pBuffer, qwRootSz);

	QWORD qwTempVal;
	GetDirItemInit(pBuffer, qwRootSz, &qwTempVal);

	FAT32_FOBJ_DESC_INTERNAL *pFObject;

	TRACE(_T("Start output\r\n"));

	for (;;)
	{
		if (GetNextDirItem(pBuffer, qwRootSz, &pFObject, &qwTempVal) != VHM_ERROR_SUCCESS)
			break;

		TRACE(_T("%s\r\n"), pFObject->wName);
		FreeMemory(pFObject);
	}

	delete[] pBuffer;

	f.Close();	
*/
	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::CloseFilesystem()
{
	/* VHMIOWrapper 객체의 유효성 검사 */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	/* 파일시스템이 마운트되었음을 확인 */
	if(!IsMounted())
		return VHM_ERROR_SUCCESS;

	/* 파일시스템이 언마운트된 것으로 표시 */
	m_bMounted = FALSE;

	/* 파일시스템 언마운트 */
	/* 핸들 관리 루틴 언로드 -> 기본 정보가 로드되지 않은 것으로 표시 -> 기본 정보 언로드 */
	UninitializeHandleManagement();
	m_bBasicInformationLoaded = FALSE;

	free(m_pLBA0);
	free(m_pFSInfo);

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::CheckFilesystem()
{
	/* VHMIOWrapper 객체의 유효성 검사 */
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	/* 파일시스템이 마운트되지 않았음을 확인 */
	if(IsMounted())
		return VHM_ERROR_GENERIC;

	/* VHMFilesystemDetermineFSTypeFATUniversal 함수를 호출하여 기본적인 검사 수행 */

	/* 파일시스템 메타데이터 확인 */

	/* Orphaned LFN Entry 검색 및 제거 */

	/* Orphaned Cluster 검색 및 제거 */

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::ModifyFilesystemVolumeLabel(WCHAR *wszVolumeLabel, UINT64 uiLength)
{
	return -1;
}

int CVHMFilesystemFAT32::ModifyFilesystemSize(UINT64 uiNewSectorCount)
{
	return -1;
}

int CVHMFilesystemFAT32::ModifyFilesystemClusterSize(QWORD qwClusterSize)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CVHMFilesystemFAT32::FileCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle)
{
	/* 요청된 파일이 존재하는 지 확인 */

	/* 요청된 액세스 권한과 공유 모드로 파일을 열 수 있는지 확인 */

	/* 요청된 파일 생성의 성격(종류)로 파일을 열 수 있는지 확인 */

	/* 파일 열기 */

	return -1;
}

int CVHMFilesystemFAT32::FileMove(QWORD qwHandle, const WCHAR *pwNewPath)
{
	return -1;
}

int CVHMFilesystemFAT32::FileGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer)
{
	return -1;
}
int CVHMFilesystemFAT32::FileSetName(QWORD qwHandle, const WCHAR *pwNewName)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::FileGetAttributes(QWORD qwHandle)
{
	return-1;
}

QWORD CVHMFilesystemFAT32::FileSetAttributes(QWORD qwHandle, QWORD qwNewAttributes)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::FileGetAccessRights(QWORD qwHandle)
{
	return -1;
}
QWORD CVHMFilesystemFAT32::FileSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights)
{
	return -1;
}

int CVHMFilesystemFAT32::FileGetSize(QWORD qwHandle, UINT64 *puiSize)
{
	return -1;
}

int CVHMFilesystemFAT32::FileSetSize(QWORD qwHandle, UINT64 uiNewSize)
{
	return -1;
}

int CVHMFilesystemFAT32::FileRead(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, UINT64 uiBufferSize, QWORD *pqwRead)
{
	return -1;
}

int CVHMFilesystemFAT32::FileWrite(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, QWORD *pqwWrite)
{
	return -1;
}

int CVHMFilesystemFAT32::FileSetPointer(QWORD qwHandle, INT64 iDistanceToMove, UINT64 *puiNewFilePointer, QWORD qwMoveMethod)
{
	return -1;
}

int CVHMFilesystemFAT32::FileDelete(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystemFAT32::FileClose(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystemFAT32::DirCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle)
{
	
	return -1;
}

int CVHMFilesystemFAT32::DirMove(QWORD qwHandle, const WCHAR *pwNewPath)
{
	return -1;
}

int CVHMFilesystemFAT32::DirGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer)
{
	return -1;
}

int CVHMFilesystemFAT32::DirSetName(QWORD qwHandle, const WCHAR *pwNewName)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::DirGetAttributes(QWORD qwHandle)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::DirSetAttributes(QWORD qwHandle, QWORD qwNewAttributes)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::DirGetAccessRights(QWORD qwHandle)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::DirSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights)
{
	return -1;
}


int CVHMFilesystemFAT32::DirGetFileCount(QWORD qwHandle, UINT64 *puiFileCount)
{
	return -1;
}

int CVHMFilesystemFAT32::DirGetFolderCount(QWORD qwHandle, UINT64 *puiFileCount)
{
	return -1;
}

int CVHMFilesystemFAT32::DirRewind(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystemFAT32::DirSeek(QWORD qwHandle, QWORD qwPosition)
{
	return -1;
}

QWORD CVHMFilesystemFAT32::DirTell(QWORD qwHandle, const FOBJECT_DESCRIPTOR *pFObjectDescriptor)
{
	return FOBJECT_INVALID_POSITION;
}

const FOBJECT_DESCRIPTOR *CVHMFilesystemFAT32::DirEnumerateNext(QWORD qwHandle)
{
	return nullptr;
}

int CVHMFilesystemFAT32::DirDelete(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystemFAT32::DirClose(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystemFAT32::WriteBootCode(const void *pBuffer)
{
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	DWORD dwSectorSize;
	BYTE *pLBA0;
	const BYTE *pBuffer2;
	const FAT_BPB_BS_COMMON *pFatBpbBsCommon = (const FAT_BPB_BS_COMMON *)pBuffer;

	// 코드 유효성 검사
	if ((pFatBpbBsCommon->BS_jmpBoot[0] != 0xEB || pFatBpbBsCommon->BS_jmpBoot[2] != 0x90)
		&& (pFatBpbBsCommon->BS_jmpBoot[0] != 0xE9))
		return VHM_ERROR_INVALID_PARAMETER;

	dwSectorSize = GetSectorSize();
	pLBA0 = new BYTE[dwSectorSize];

	// LBA 0을 읽고 pBuffer에서 Boot Code 부분만 복사
	pBuffer2 = (const BYTE *)pBuffer;
	ReadSector(0, 1, pLBA0, dwSectorSize);

	// Jump code (Offset 0x0000, End 0x0002, Size 0x0003)
	// Boot code (Offset 0x005A, End 0x01FD, Size 0x01A4)
	// Boot code (Offset 0x0200, End (dwSectorSize - 1))
	CopyMemoryBlock(pLBA0 + 0x0000, pBuffer2 + 0x0000, 0x0003);
	CopyMemoryBlock(pLBA0 + 0x005A, pBuffer2 + 0x005A, 0x01A4);
	if (dwSectorSize > 512)
		CopyMemoryBlock(pLBA0 + 0x0200, pBuffer2 + 0x0200, dwSectorSize - 0x0200);

	// 쓰기 및 메모리 정리
	WriteSector(0, 1, pLBA0);
	delete[] pLBA0;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystemFAT32::GetVolumeLabel(WCHAR *pwszBuffer, vhmsize_t ccBuffer)
{
	if (!IsMounted())
		return VHM_ERROR_GENERIC;

	vhmsize_t ccBuffer_Internal;	
	ccBuffer_Internal = _tcslen(m_pwszVolumeLabel) + 1;

	if (ccBuffer < ccBuffer_Internal)
		return VHM_ERROR_BUFFER_TOO_SMALL;

	_tcscpy_s(pwszBuffer, ccBuffer, m_pwszVolumeLabel);

	return VHM_ERROR_SUCCESS;
}

QWORD CVHMFilesystemFAT32::GetFilesystemStatus()
{
	return FILESYSTEM_STATUS_INVALID;
}

DWORD CVHMFilesystemFAT32::GetFilesystemCharacteristics()
{
	return FILESYSTEM_CHARACTERISTICS_NONE;
}

int CVHMFilesystemFAT32::QueryFilesystemInformation(FILESYSTEM_INFORMATION *pFilesystemInformation)
{
	return -1;
}

DWORD CVHMFilesystemFAT32::GetFilesystemType()
{
	return FILESYSTEM_FAT32;
}

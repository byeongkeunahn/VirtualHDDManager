
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMIOWrapper.h"
#include "VHMFilesystemFATStructure.h"
#include "VHMFilesystemFATCommon.h"

int VHMFilesystemDetermineFSTypeFATUniversal(CVHMIOWrapper *pVHMIOWrapper)
{
	if(!VHMIOWrapperIsObjectValid(pVHMIOWrapper))
		return FILESYSTEM_INVALID;

	int iType = FILESYSTEM_INVALID;

	/* 지역변수로 값 옮기기 */
	const DWORD dwSectorSize = pVHMIOWrapper->GetSectorSize();
	const UINT64 uiPartitionSectorCount = pVHMIOWrapper->GetSectorCount();

	/* 파티션의 LBA 0 읽기 */
	BYTE *pLBA0 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadOneSector(0, pLBA0, dwSectorSize);
	FAT_BPB_BS_COMMON *pFATLBA0 = (FAT_BPB_BS_COMMON *) pLBA0;
	FAT12_FAT16_BPB_BS *pFAT1216BPBBS = (FAT12_FAT16_BPB_BS *) (pLBA0 + 36);
	FAT32_BPB_BS *pFAT32BPBBS = (FAT32_BPB_BS *) (pLBA0 + 36);

	/* LBA 0 (파일시스템 헤더) 정보 확인*/
	if(((pFATLBA0->BS_jmpBoot[0] != 0xEB || pFATLBA0->BS_jmpBoot[2] != 0x90) && (pFATLBA0->BS_jmpBoot[0] != 0xE9)) /* BS_jmpBoot 확인 */ ||
		pFATLBA0->BPB_BytsPerSec != dwSectorSize /* 섹터 크기가 올바른 지 확인 */ ||
		!IsPowerOfTwo(pFATLBA0->BPB_SecPerClus) /* 값이 올바른 지 확인 */ || 
		(pFATLBA0->BPB_BytsPerSec * pFATLBA0->BPB_SecPerClus) > FAT_MAX_CLUSTER_SIZE /* 클러스터 크기가 올바른 지 확인 */ || 
		pFATLBA0->BPB_RsvdSecCnt <= 0 /* 예약된 섹터는 LBA 0을 포함하므로 최소 1개임 */||
		pLBA0[510] != 0x55 || pLBA0[511] != 0xAA /* 55 AA 시그니처 확인 */)
	{
		AfxMessageBox(_T("Not FAT-series file system"));
		free(pLBA0);
		return FILESYSTEM_INVALID;
	}

	/* FAT 타입 판별 */

	/* 파일시스템 정보 조사 */
	DWORD dwRootDirSectors; // 루트 디렉터리 영역의 섹터 수
	DWORD dwFATSz; // FAT 1개의 섹터 수
	DWORD dwTotSec; // 파티션 전체의 섹터 수
	DWORD dwDataSec; // 데이터 영역의 섹터 수
	DWORD dwCountOfClusters; // 클러스터 수

	dwRootDirSectors = ((pFATLBA0->BPB_RootEntCnt * 32) + (pFATLBA0->BPB_BytsPerSec - 1)) / pFATLBA0->BPB_BytsPerSec;

	if(pFATLBA0->BPB_FATSz16 != 0)
		dwFATSz = pFATLBA0->BPB_FATSz16;
	else
		dwFATSz = pFAT32BPBBS->BPB_FATSz32;

	if(pFATLBA0->BPB_TotSec16 != 0)
		dwTotSec = pFATLBA0->BPB_TotSec16;
	else
		dwTotSec = pFATLBA0->BPB_TotSec32;

//	if (!dwTotSec)
	//	return FILESYSTEM_INVALID;

	dwDataSec = dwTotSec - (pFATLBA0->BPB_RsvdSecCnt + (pFATLBA0->BPB_NumFATs * dwFATSz) + dwRootDirSectors);

	dwCountOfClusters = dwDataSec / pFATLBA0->BPB_SecPerClus;
	if(dwCountOfClusters <= FAT12_MAXIMUM_CLUSTER_COUNT)
		iType = FILESYSTEM_FAT12;
	else if(dwCountOfClusters <= FAT16_MAXIMUM_CLUSTER_COUNT)
		iType = FILESYSTEM_FAT16;
	else if(dwCountOfClusters <= FAT32_MAXIMUM_CLUSTER_COUNT)
		iType = FILESYSTEM_FAT32;

	AfxMessageBox(_T("Filesystem recognized as FAT-series filesystem!"));

	free(pLBA0);
	return iType;
}

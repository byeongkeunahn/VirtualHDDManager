
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

	/* ���������� �� �ű�� */
	const DWORD dwSectorSize = pVHMIOWrapper->GetSectorSize();
	const UINT64 uiPartitionSectorCount = pVHMIOWrapper->GetSectorCount();

	/* ��Ƽ���� LBA 0 �б� */
	BYTE *pLBA0 = (BYTE *) malloc(dwSectorSize);
	pVHMIOWrapper->ReadOneSector(0, pLBA0, dwSectorSize);
	FAT_BPB_BS_COMMON *pFATLBA0 = (FAT_BPB_BS_COMMON *) pLBA0;
	FAT12_FAT16_BPB_BS *pFAT1216BPBBS = (FAT12_FAT16_BPB_BS *) (pLBA0 + 36);
	FAT32_BPB_BS *pFAT32BPBBS = (FAT32_BPB_BS *) (pLBA0 + 36);

	/* LBA 0 (���Ͻý��� ���) ���� Ȯ��*/
	if(((pFATLBA0->BS_jmpBoot[0] != 0xEB || pFATLBA0->BS_jmpBoot[2] != 0x90) && (pFATLBA0->BS_jmpBoot[0] != 0xE9)) /* BS_jmpBoot Ȯ�� */ ||
		pFATLBA0->BPB_BytsPerSec != dwSectorSize /* ���� ũ�Ⱑ �ùٸ� �� Ȯ�� */ ||
		!IsPowerOfTwo(pFATLBA0->BPB_SecPerClus) /* ���� �ùٸ� �� Ȯ�� */ || 
		(pFATLBA0->BPB_BytsPerSec * pFATLBA0->BPB_SecPerClus) > FAT_MAX_CLUSTER_SIZE /* Ŭ������ ũ�Ⱑ �ùٸ� �� Ȯ�� */ || 
		pFATLBA0->BPB_RsvdSecCnt <= 0 /* ����� ���ʹ� LBA 0�� �����ϹǷ� �ּ� 1���� */||
		pLBA0[510] != 0x55 || pLBA0[511] != 0xAA /* 55 AA �ñ״�ó Ȯ�� */)
	{
		AfxMessageBox(_T("Not FAT-series file system"));
		free(pLBA0);
		return FILESYSTEM_INVALID;
	}

	/* FAT Ÿ�� �Ǻ� */

	/* ���Ͻý��� ���� ���� */
	DWORD dwRootDirSectors; // ��Ʈ ���͸� ������ ���� ��
	DWORD dwFATSz; // FAT 1���� ���� ��
	DWORD dwTotSec; // ��Ƽ�� ��ü�� ���� ��
	DWORD dwDataSec; // ������ ������ ���� ��
	DWORD dwCountOfClusters; // Ŭ������ ��

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

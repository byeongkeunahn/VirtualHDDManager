
#pragma once

#pragma pack(push, 1)

// FAT12/FAT16/FAT32 공통 파일시스템 정보
typedef struct _fat_bpb_bs_common
{
	BYTE	BS_jmpBoot[3];
	BYTE	BS_OEMName[8];
	WORD	BPB_BytsPerSec;
	BYTE	BPB_SecPerClus;
	WORD	BPB_RsvdSecCnt;
	BYTE	BPB_NumFATs;
	WORD	BPB_RootEntCnt;
	WORD	BPB_TotSec16;
	BYTE	BPB_Media;
	WORD	BPB_FATSz16;
	WORD	BPB_SecPerTrk;
	WORD	BPB_NumHeads;
	DWORD	BPB_HiddSec;
	DWORD	BPB_TotSec32;

}FAT_BPB_BS_COMMON;

#define FAT_MAX_CLUSTER_SIZE			32768 // FAT 파일시스템의 최대 클러스터 크기 = 32KiB

#define FAT12_MAXIMUM_CLUSTER_COUNT		4084
#define FAT16_MAXIMUM_CLUSTER_COUNT		65524
#define FAT32_MAXIMUM_CLUSTER_COUNT		0x0FFFFFF4

// FAT12/FAT16 파일시스템 정보
typedef struct _fat12_fat16_bpb_bs
{
	BYTE	BS_DrvNum;
	BYTE	BS_Reserved1;
	BYTE	BS_BootSig;
	DWORD	BS_VolID;
	BYTE	BS_VolLab[11];
	BYTE	BS_FilSysType[8];

}FAT12_FAT16_BPB_BS;

// FAT32 파일시스템 정보
typedef struct _fat32_bpb_bs
{
	DWORD	BPB_FATSz32;
	WORD	BPB_ExtFlags;
	WORD	BPB_FSVer;
	DWORD	BPB_RootClus;
	WORD	BPB_FSInfo;
	WORD	BPB_BkBootSec;
	BYTE	BPB_Reserved[12];
	BYTE	BS_DrvNum;
	BYTE	BS_Reserved1;
	BYTE	BS_BootSig;
	DWORD	BS_VolID;
	BYTE	BS_VolLab[11];
	BYTE	BS_FilSysType[8];

}FAT32_BPB_BS;

#define FAT32_FAT_MASK				0x7 // FAT 마스크
#define FAT32_FAT_MODE_MASK			0x80 // FAT 모드 마스크
#define FAT32_FAT_MODE_MIRRORING	0x0 // FAT 미러링

// FAT32 파일시스템 정보 섹터
typedef struct _fat32_fsinfo_sector
{
	DWORD	FSI_LeadSig;
	BYTE	FSI_Reserved1[480];
	DWORD	FSI_StrucSig;
	DWORD	FSI_Free_Count;
	DWORD	FSI_Nxt_Free;
	BYTE	FSI_Reserved2[12];
	DWORD	FSI_TrailSig;

}FAT32_FSINFO_SECTOR;

#define FAT32_FSINFOSEC_LEADSIG		0x41615252
#define	FAT32_FSINFOSEC_STRUCSIG	0x61417272
#define FAT32_FSINFOSEC_TRAILSIG	0xAA550000

// FAT16/FAT32 오류 마스크 (AND연산 결과가 0이면 오류)
#define FAT16_CLEAN_SHUTDOWN_MASK	0x8000
#define FAT16_HW_ERROR_MASK			0x4000
#define FAT32_CLEAN_SHUTDOWN_MASK	0x08000000
#define FAT32_HW_ERROR_MASK			0x04000000

// FAT12/FAT16/FAT32 디렉터리 엔트리
typedef struct _fat_directory_entry
{
	BYTE	DIR_Name[11];
	BYTE	DIR_Attr;
	BYTE	DIR_NTRes;
	BYTE	DIR_CrtTimeTenth;
	WORD	DIR_CrtTime;
	WORD	DIR_CrtDate;
	WORD	DIR_LstAccDate;
	WORD	DIR_FstClusHI;
	WORD	DIR_WrtTime;
	WORD	DIR_WrtDate;
	WORD	DIR_FstClusLO;
	DWORD	DIR_FileSize;

}FAT_DIR_ENTRY;

#define FAT_DIR_ENTRY_SIZE			32 // 디렉터리 엔트리 1개의 크기
#define FAT_MAX_FILE_COUNT_DIR		65536 // 한 디렉터리에 포함될 수 있는 최대 파일 수

// FAT_DIR_ENTRY - DIR_Attr
#define FAT_ATTR_READ_ONLY		0x01
#define FAT_ATTR_HIDDEN			0x02
#define FAT_ATTR_SYSTEM			0x04
#define FAT_ATTR_VOLUME_ID		0x08
#define FAT_ATTR_DIRECTORY		0x10
#define FAT_ATTR_ARCHIVE		0x20
#define FAT_ATTR_LONG_NAME		(FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)
#define FAT_ATTR_LONG_NAME_MASK		(FAT_ATTR_LONG_NAME | FAT_ATTR_DIRECTORY | FAT_ATTR_ARCHIVE)
#define FAT_ATTR_LAST_LONG_ENTRY	0x40

// FAT32 긴 파일 이름 (Long File Name) 엔트리
typedef struct _fat_lfn_entry
{
	BYTE	LDIR_Ord;
	WCHAR	LDIR_Name1[5];
	BYTE	LDIR_Attr;
	BYTE	LDIR_Type;
	BYTE	LDIR_Chksum;
	WCHAR	LDIR_Name2[6];
	WORD	LDIR_FstClusLO;
	WCHAR	LDIR_Name3[2];

}FAT_LFN_ENTRY;

#pragma pack(pop)

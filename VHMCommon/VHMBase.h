
/* VHMBase.h */

#pragma once

/*

Virtual HDD Manager Library

<클래스 소개>

CVHMRoot - VHM 루트 클래스
	* VHM의 최상위 오브젝트
	* 하위 VHM 오브젝트를 관리하고, I/O 요청을 해석하여 VHM 오브젝트로 포워드

CVHMDisk - 디스크 I/O 입출력 클래스
	CVHMDiskFile - 가상 디스크 파일에 대한 I/O (VHM, RAW)
	CVHMDiskVHD - 가상 디스크 파일에 대한 I/O (VHD)
	CVHMDiskVHDX - 가상 디스크 파일에 대한 I/O (VHDX)
	CVHMDiskVMDK - 가상 디스크 파일에 대한 I/O (VMDK)
	CVHMDiskVDI - 가상 디스크 파일에 대한 I/O (VDI)
	CVHMDiskPhysical - 물리 디스크에 대한 I/O
	CVHMDiskRam - RAM에 생성된 가상 디스크에 대한 I/O
	* 각 포맷은 섹터 단위 저장을 통해 다른 포맷으로 변환 가능

CVHMPartition - 파티션 관리 클래스 (MBR, GPT)

CVHMFilesystem - 파일시스템 처리 클래스
	CVHMFilesystemFAT12 - FAT12 파티션 처리
	CVHMFilesystemFAT16 - FAT16 파티션 처리
	CVHMFilesystemFAT32 - FAT32 파티션 처리
	CVHMFilesystemexFAT - exFAT 파티션 처리
	CVHMFilesystemNTFS - NTFS 파티션 처리
	CVHMFilesystemext4 - ext4 파티션 처리
	CVHMFilesystemHFS+ - HFS+ 파티션 처리

CVHMScript - VHM 스크립트 처리 클래스
	* VHMScript 형식으로 작성된 스크립트를 해석하여 VHMRoot에 해당 동작을 요청


<VHM 모델 - Disk, Partition, Filesystem, IOWrapper>

CVHMDisk <--> 섹터 단위 I/O
CVHMDisk <--> CVHMPartition <--> 파티션 내의 섹터 I/O
CVHMDisk <--> CVHMPartition <--> CVHMFilesystem <--> 파일 I/O

CVHMDisk <--> CVHMPartition : CVHMIOWrapper는 CVHMDisk 객체별로 1개씩 존재하여 CVHMPartition에 할당된다.

CVHMPartition <--> CVHMIOWrapper : CVHMIOWrapper는 각 파티션별로 1개씩 존재하여 CVHMFilesystem의 I/O 요청을 CVHMPartition으로 전달한다.

CVHMIOWrapper <--> CVHMFilesystem : CVHMIOWrapper에 구현된 I/O Wrapping 함수를 호출하여 I/O를 수행한다.

*/

#pragma pack(push, 1)

// Data types

#ifndef _WINDNS_INCLUDED_
typedef UINT64 QWORD;
#endif

typedef INT32 SDWORD;
typedef INT64 SQWORD;

typedef UINT64 VPARAM;
typedef void* POINTER;


typedef struct _OWORD
{
	QWORD LowPart;
	QWORD HighPart;

}OWORD;

typedef struct _vhmuuid
{
	DWORD	Data1;
	WORD	Data2;
	WORD	Data3;
	BYTE	Data4[8];

}VHMUUID;

typedef const WCHAR *PCWCHAR;

// Pointer size type
#if defined(__x86_64__) || defined(_M_X64)
typedef UINT64 vhmsize_t;
#elif defined(__i386) || defined(_M_IX86)
typedef UINT32 vhmsize_t;
#else
#error VHMBase: Unsupported target platform
#endif

// TRUE, FALSE
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Invalid value
#define VHM_INVALID_VALUE         (-1)

// Eightcc
#define MAKEEIGHTCC(ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7)		\
	((QWORD)(BYTE)(ch0) | ((QWORD)(BYTE)(ch1) << 8) |			\
	((QWORD)(BYTE)(ch2) << 16) | ((QWORD)(BYTE)(ch3) << 24) |	\
	((QWORD)(BYTE)(ch4) << 32) | ((QWORD)(BYTE)(ch5) << 40) |	\
	((QWORD)(BYTE)(ch6) << 48) | ((QWORD)(BYTE)(ch7) << 56))

// Handle
#define DEFINE_HANDLE(name) typedef struct _define_handle_dummy_##name { BYTE dummy[sizeof(POINTER)]; } N_##name, *name;
#define FWDDEF_HANDLE(name) struct _define_handle_dummy_##name; typedef _define_handle_dummy_##name N_##name, *name;

// Determines if n is power of 2 or not
#define IsPowerOfTwo(n) ((((n) & (n-1)) == 0) ? TRUE : FALSE)

const DWORD SECTORSIZE = 512;
const QWORD TwoExp32 = 0x100000000UL;

// Callback return values
#define CALLBACK_CONTINUE						0
#define CALLBACK_STOP							1

// Error Codes
#define VHM_ERROR_GENERIC						-1 // 일반적 오류
#define VHM_ERROR_SUCCESS						0 // 성공
#define VHM_ERROR_NOT_INITIALIZED				1 // 초기화되지 않음
#define VHM_ERROR_INVALID_VHMDISK				2 // 올바르지 않은 VHMDisk 객체
#define VHM_ERROR_ALREADY_OPENED				3 // 이미 열려 있음
#define VHM_ERROR_MEMORY_ALLOCATION_FAILED		4 // 메모리 할당 실패
#define VHM_ERROR_NOT_OPENED					5 // 열려있지 않음
#define VHM_ERROR_INVALID_PARAMETER				6 // 올바르지 않은 매개변수
#define VHM_ERROR_INVALID						7 // 올바르지 않음
#define VHM_ERROR_INTERNAL						8 // 내부 오류
#define VHM_ERROR_DISK_IO						9 // 디스크 I/O 오류
#define VHM_ERROR_DRIVER_NOT_FOUND				10 // (해당 항목에 대한) 드라이버를 찾을 수 없음 (파일시스템 드라이버 등)
#define VHM_ERROR_UNSUPPORTED					11 // 지원되지 않는 기능
#define VHM_ERROR_BUFFER_TOO_SMALL				12 // 버퍼 크기가 충분하지 않음
#define VHM_ERROR_PERMISSION_DENIED				13 // 권한 거부
#define VHM_ERROR_NOT_FOUND						14 // 찾을 수 없음
#define VHM_ERROR_NOT_AVAILABLE					15 // 사용 불가능
#define VHM_ERROR_TOO_BIG						16 // 너무 큼
#define VHM_ERROR_END							17 // 끝 (파일, 버퍼, 스택, ...)

// Error
#define VHM_ECHECK(code) { if (code != VHM_ERROR_SUCCESS) { err_code = code; goto cleanup; } }
#define VHM_ERROR(code) { err_code = code; goto cleanup; }

// VHMDisk

#define VIRTUAL_DISK_FORMAT_RAW					0
#define VIRTUAL_DISK_FORMAT_VHM					1
#define VIRTUAL_DISK_FORMAT_RAM					2
#define VIRTUAL_DISK_FORMAT_VHD					3
#define VIRTUAL_DISK_FORMAT_VHDX				4
#define VIRTUAL_DISK_FORMAT_VMDK				5
#define VIRTUAL_DISK_FORMAT_VDI					6
#define VIRTUAL_DISK_FORMAT_PHYSICALDEVICE		0x80000000
#define VIRTUAL_DISK_FORMAT_INVALID				0xFFFFFFFF

// VHMDiskFile
typedef struct _vhmheader
{
	DWORD		Magic;				// Magic
	DWORD		SectorSize;			// Sector Size(512B, 4K, etc) - 2^n
	QWORD		SectorCount;		// Sector Count - ex) SectorSize = 4K, SectorCount = 2^30 -> BYTE = 4K * 2^30 = 4T
	BYTE		Reserved[65520];

} VHMHEADER;

const DWORD VHMHEADER_MAGIC = 0x314D4856;

// VHMPartition, VHMFilesystem - Internal Filesystem IDs
#define FILESYSTEM_INVALID			-1
#define FILESYSTEM_FAT12			1
#define FILESYSTEM_FAT16			2
#define FILESYSTEM_FAT32			3

// VHMPartition, VHMFilesystem - MBR Filesystem IDs
#define MBR_PARID_FREE		0x00
#define MBR_PARID_FAT12		0x01
#define MBR_PARID_FAT16		0x04
#define MBR_PARID_HPFS		0x07
#define MBR_PARID_NTFS		0x07
#define MBR_PARID_exFAT		0x07
#define MBR_PARID_HFS		0xAF
#define MBR_PARID_HFSPlus	0xAF
#define MBR_PARID_GPTPROTECTIVE	0xEE

// VHMPartition, VHMFilesystem - GPT Filesystem IDs
const VHMUUID GPT_PARTUUID_FREE =
{0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const VHMUUID GPT_PARTUUID_EFISYS =
{0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B};

const VHMUUID GPT_PARTUUID_BIOSBOOT = 
{0x21686148, 0x6449, 0x6E6F, 0x74, 0x4E, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49};

const VHMUUID GPT_PARTUUID_MICROSOFTRESERVED = 
{0xE3C9E316, 0x0B5C, 0x4DB8, 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE};

const VHMUUID GPT_PARTUUID_MICROSOFTDATA = 
{0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7};

const VHMUUID GPT_PARTUUID_LINUXDATA =
{0x0FC63DAF, 0x8483, 0x4772, 0x8E, 0x79, 0x3D, 0x69, 0xD8, 0x47, 0x7D, 0xE4};

const VHMUUID GPT_PARTUUID_APPLEHFSPLUS =
{0x48465300, 0x0000, 0x11AA, 0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC};

// VHMPartition

#define PARTITION_ID_MAX				((QWORD)-2)
#define INVALID_PARTITION_ID			((QWORD)-1)

#define PART_TYPE_UNINITIALIZED			0	// 초기화되지 않은 디스크
#define PART_TYPE_UNPARTITIONED_DISK	1	// 파티셔닝되지 않은 (LBA 0이 파티션의 첫 섹터인) 디스크
#define PART_TYPE_MBR					2
#define PART_TYPE_GPT					3
#define PART_TYPE_INVALID_VHMDISK		((DWORD) -1)

#define DISK_SIGNATURE					((WORD) 0xAA55)

#define MBR_PART_ATTR_BIOSBOOTABLE		0x80

#define GPT_PART_ATTR_USED_BY_EFI			0x0000000000000001
#define GPT_PART_ATTR_REQUIRED_TO_FUNCTION	0x0000000000000002
#define GPT_PART_ATTR_USED_BY_OS			0x0000000000000004
#define GPT_PART_ATTR_REQUIRED_BY_OS		0x0000000000000008
#define GPT_PART_ATTR_BACKUP_REQUIRED		0x0000000000000010
#define GPT_PART_ATTR_USER_DATA				0x0000000000000020
#define GPT_PART_ATTR_CRITICAL_USER_DATA	0x0000000000000040
#define GPT_PART_ATTR_REDUNDANT_PARTITION	0x0000000000000080
#define GPT_PART_ATTR_SYSTEM				0x0000000000000001
#define GPT_PART_ATTR_BIOSBOOTABLE			0x0000000000000004
#define GPT_PART_ATTR_READONLY				0x1000000000000000
#define GPT_PART_ATTR_HIDDEN				0x4000000000000000
#define GPT_PART_ATTR_DONOTAUTOMOUNT		0x8000000000000000

#define GPT_HEADER_SIGNATURE			0x5452415020494645 /* EFI 파티션 서명 (ASCII: "EFI PART") */
#define GPT_HEADER_REVISION_MAJOR		0x1000
#define GPT_HEADER_REVISION_MINOR		0x0000

class CVHMIOWrapper;
class CVHMFilesystem;

typedef struct _partition_descriptor
{
	QWORD			PartitionID;
	UINT64			StartLBA;
	UINT64			SectorCountLBA;
	QWORD			AttributeGPT;
	BYTE			AttributeMBR;
	CVHMIOWrapper	*pVHMIOWrapper; // VHMIOWrapper가 로드되지 않았으면 nullptr
	CVHMFilesystem	*pVHMFilesystem; // VHMFilesystem이 로드되지 않았으면 nullptr

}PARTITION_DESCRIPTOR;

// CHS addressing is not implemented
typedef struct _chs_address
{
	unsigned char	head:8;
	unsigned short	sector:6;
	unsigned short	ylinder:10;

}CHS_ADDRESS;

typedef struct _mbr_partition_entry
{
	BYTE			bBootable;
	CHS_ADDRESS		chsFirstSector;
	BYTE			bPartitionType;
	CHS_ADDRESS		chsLastSector;
	DWORD			dwFirstSectorLBA;
	DWORD			dwSectorCountLBA;

}MBR_PARTITION_ENTRY;

typedef struct _mbr_structure
{
	BYTE			bBootCode[440];
	DWORD			dwDiskSignature;
	WORD			wReserved;
	MBR_PARTITION_ENTRY mPartitionEntry[4];
	WORD			wBootSignature;

}MBR_STRUCTURE;

typedef struct _gpt_header
{
	QWORD	Signature;
	WORD	RevisionMinor;
	WORD	RevisionMajor;
	DWORD	HeaderSize;
	DWORD	HeaderCRC32;
	BYTE	Reserved[4];
	QWORD	MyLBA;
	QWORD	AlternateLBA;
	QWORD	FirstUsableLBA;
	QWORD	LastUsableLBA;
	VHMUUID	DiskGUID;
	QWORD	PartitionEntryLBA;
	DWORD	NumberOfPartitionEntries;
	DWORD	SizeOfPartitionEntry;
	DWORD	PartitionEntryArrayCRC32;

}GPT_HEADER;

typedef struct _gpt_partition_entry
{
	VHMUUID	PartitionTypeGUID;
	VHMUUID	UniquePartitionGUID;
	QWORD	StartingLBA;
	QWORD	EndingLBA;
	QWORD	Attributes;
	WCHAR	PartitionName[36];

}GPT_PARTITION_ENTRY;

typedef struct _vhmpartition_internal_gpt
{
	DWORD	dwSectorSize; // GPT 헤더 메모리 할당 크기와 동일
	DWORD	dwGPTPartitionEntrySize; // GPT 파티션 엔트리 크기
	DWORD	dwGPTPartitionEntryCount; // GPT 파티션 엔트리 수
	GPT_HEADER	*pGPTHeaderPrimary;
	GPT_HEADER	*pGPTHeaderBackup;
	GPT_PARTITION_ENTRY	*pGPTPartitionEntries;

}VHMPARTITION_INTERNAL_GPT;

typedef struct _vhmpartition_internal_mbr
{
	DWORD dwSectorSize; // 섹터 크기: MBR 메모리 할당 크기외 동일
	MBR_STRUCTURE	*pMBR;

}VHMPARTITION_INTERNAL_MBR;

// VHMFilesystem

/* Filesystem type check function callback */
typedef int (*VHMFilesystemDetermineFSType)(CVHMIOWrapper *pVHMIOWrapper);

// Filesystem Information
typedef struct _filesystem_information
{
	QWORD	Size;
	WCHAR	VolumeLabel[256];
	UINT64	StartLBA;
	UINT64	SectorCountLBA;
	QWORD	ClusterSize;

}FILESYSTEM_INFORMATION;

// VHMFilesystem - GetFilesystemStatus()
#define FILESYSTEM_STATUS_INVALID		0xFFFFFFFFFFFFFFFF // 파일시스템 상태가 올바르지 않음
#define FILESYSTEM_STATUS_MOUNTED		0x0000000000000001 // 파일시스템이 마운트됨
#define FILESYSTEM_STATUS_VOLUME_OK		0x0000000000000002 // 파일시스템에 오류가 없음
#define FILESYSTEM_STATUS_READABLE		0x0000000000000004 // 파일시스템을 읽을 수 있음
#define FILESYSTEM_STATUS_WRITABLE		0x0000000000000008 // 파일시스템에 쓸 수 있음

// VHMFilesystem - GetFilesystemCharacteristics()
#define FILESYSTEM_CHARACTERISTICS_NONE				0x0000000000000000 // 특성 없음
#define FILESYSTEM_CHARACTERISTICS_JOURNALING		0x0000000000000001 // 저널링 지원
#define FILESYSTEM_CHARACTERISTICS_UNIX_PERMISSION	0x0000000000000002 // 유닉스 권한 지원
#define FILESYSTEM_CHARACTERISTICS_ACL				0x0000000000000004 // ACL (Access Control List) 지원
#define FILESYSTEM_CHARACTERISTICS_HOT_RESIZE		0x0000000000000008 // 파일시스템이 마운트되었을 때 리사이즈 가능

/* VHMFilesystem - FObject (File/Folder) Flags */

// 요청되는 액세스 권한 / 공유 모드
#define ACCESS_RIGHT_DENY			0x00000000
#define ACCESS_RIGHT_READ			0x00000001
#define ACCESS_RIGHT_WRITE			0x00000002
#define ACCESS_RIGHT_RENAME			0x00000004
#define ACCESS_RIGHT_DELETE			0x00000008

// 요청되는 파일 생성의 성격(종류)
#define CREATE_NEW					1
#define CREATE_ALWAYS				2
#define OPEN_EXISTING				3
#define OPEN_ALWAYS					4
#define TRUNCATE_EXISTING			5

// 파일에 지정된 액세스 권한 - 전통적인 리눅스 모델을 따름
#define FOBJ_ACCESS_RIGHT_OTHERS_READ		0x0000000000000004 // 다른 사용자 - 읽기
#define FOBJ_ACCESS_RIGHT_OTHERS_WRITE		0x0000000000000002 // 다른 사용자 - 쓰기
#define FOBJ_ACCESS_RIGHT_OTHERS_EXECUTE	0x0000000000000001 // 다른 사용자 - 실행
#define FOBJ_ACCESS_RIGHT_GROUP_READ		0x0000000000000020 // 그룹에 속한 사용자 - 읽기
#define FOBJ_ACCESS_RIGHT_GROUP_WRITE		0x0000000000000010 // 그룹에 속한 사용자 - 쓰기
#define FOBJ_ACCESS_RIGHT_GROUP_EXECUTE		0x0000000000000008 // 그룹에 속한 사용자 - 실행
#define FOBJ_ACCESS_RIGHT_OWNER_READ		0x0000000000000100 // 소유자 - 읽기
#define FOBJ_ACCESS_RIGHT_OWNER_WRITE		0x0000000000000080 // 소유자 - 쓰기
#define FOBJ_ACCESS_RIGHT_OWNER_EXECUTE		0x0000000000000040 // 소유자 - 실행
#define FOBJ_ACCESS_RIGHT_STICKY			0x0000000000000200 // 다른 사용자 - 삭제 금지
#define FOBJ_ACCESS_RIGHT_SETGID			0x0000000000000400 // 실행하는 동안 GID를 파일 GID로 설정 (Effective GID)
#define FOBJ_ACCESS_RIGHT_SETUID			0x0000000000000800 // 실행하는 동안 UID를 파일 UID로 설정 (Effective UID)
#define FOBJ_ACCESS_RIGHT_UNSUPPORTED		0xFFFFFFFFFFFFFFFF // 권한 지원 X -> 모두에게 액세스 허용

// 파일 속성
#define FOBJ_ATTR_DIRECTORY				0x8000000000000000 // 디렉터리
#define FOBJ_ATTR_NORMAL				0x0000000000000000 // 일반 파일 속성
#define FOBJ_ATTR_READ_ONLY				0x0000000000000001 // 읽기 전용
#define FOBJ_ATTR_HIDDEN				0x0000000000000002 // 숨김
#define FOBJ_ATTR_SYSTEM				0x0000000000000004 // 시스템
#define FOBJ_ATTR_ARCHIVE				0x0000000000000008 // 백업되지 않음
#define FOBJ_ATTR_VOLUME_ID				0x0000000000000010 // 볼륨 레이블
#define FOBJ_ATTR_TEMPORARY				0x0000000000000020 // 임시 파일
#define FOBJ_ATTR_ENCRYPTED				0x0000000000000040 // 암호화된 파일
#define FOBJ_ATTR_EXECUTABLE			0x0000000000000080 // 실행 가능
#define FOBJ_ATTR_SYMLINK				0x0000000000000100 // 심볼릭 링크
#define FOBJ_ATTR_HARDLINK				0x0000000000000200 // 하드 링크

/* Directory Enumeration */

// FObject descriptor
typedef struct _fobj_descriptor
{
	QWORD	qwPointer; // 파일 포인터 (SEEK 위치)
	QWORD	qwFileSize; // 파일 크기
	QWORD	qwFileAttributes; // 파일 속성 (하위 32비트-공통, 상위 32비트-파일시스템 특정)
	QWORD	qwFileAccessRights; // 파일 액세스 권한
	WCHAR	wName[1]; // FObject 이름 (경로 미포함)

}FOBJECT_DESCRIPTOR;

#define FOBJECT_INVALID_POSITION		((QWORD)-1)


// Text Encoding

typedef UINT ENCODING;

#define ENC_ANSI		0x1 // ANSI
#define ENC_UTF8		0x2 // UTF-8
#define ENC_U16B		0x3 // UTF-16, Big Endian
#define ENC_U16L		0x4 // UTF-16, Little Endian
#define ENC_U32B		0x5 // UTF-32, Big Endian
#define ENC_U32L		0x6 // UTF-32, Little Endian

#pragma pack(pop)


/* VHMBase.h */

#pragma once

/*

Virtual HDD Manager Library

<Ŭ���� �Ұ�>

CVHMRoot - VHM ��Ʈ Ŭ����
	* VHM�� �ֻ��� ������Ʈ
	* ���� VHM ������Ʈ�� �����ϰ�, I/O ��û�� �ؼ��Ͽ� VHM ������Ʈ�� ������

CVHMDisk - ��ũ I/O ����� Ŭ����
	CVHMDiskFile - ���� ��ũ ���Ͽ� ���� I/O (VHM, RAW)
	CVHMDiskVHD - ���� ��ũ ���Ͽ� ���� I/O (VHD)
	CVHMDiskVHDX - ���� ��ũ ���Ͽ� ���� I/O (VHDX)
	CVHMDiskVMDK - ���� ��ũ ���Ͽ� ���� I/O (VMDK)
	CVHMDiskVDI - ���� ��ũ ���Ͽ� ���� I/O (VDI)
	CVHMDiskPhysical - ���� ��ũ�� ���� I/O
	CVHMDiskRam - RAM�� ������ ���� ��ũ�� ���� I/O
	* �� ������ ���� ���� ������ ���� �ٸ� �������� ��ȯ ����

CVHMPartition - ��Ƽ�� ���� Ŭ���� (MBR, GPT)

CVHMFilesystem - ���Ͻý��� ó�� Ŭ����
	CVHMFilesystemFAT12 - FAT12 ��Ƽ�� ó��
	CVHMFilesystemFAT16 - FAT16 ��Ƽ�� ó��
	CVHMFilesystemFAT32 - FAT32 ��Ƽ�� ó��
	CVHMFilesystemexFAT - exFAT ��Ƽ�� ó��
	CVHMFilesystemNTFS - NTFS ��Ƽ�� ó��
	CVHMFilesystemext4 - ext4 ��Ƽ�� ó��
	CVHMFilesystemHFS+ - HFS+ ��Ƽ�� ó��

CVHMScript - VHM ��ũ��Ʈ ó�� Ŭ����
	* VHMScript �������� �ۼ��� ��ũ��Ʈ�� �ؼ��Ͽ� VHMRoot�� �ش� ������ ��û


<VHM �� - Disk, Partition, Filesystem, IOWrapper>

CVHMDisk <--> ���� ���� I/O
CVHMDisk <--> CVHMPartition <--> ��Ƽ�� ���� ���� I/O
CVHMDisk <--> CVHMPartition <--> CVHMFilesystem <--> ���� I/O

CVHMDisk <--> CVHMPartition : CVHMIOWrapper�� CVHMDisk ��ü���� 1���� �����Ͽ� CVHMPartition�� �Ҵ�ȴ�.

CVHMPartition <--> CVHMIOWrapper : CVHMIOWrapper�� �� ��Ƽ�Ǻ��� 1���� �����Ͽ� CVHMFilesystem�� I/O ��û�� CVHMPartition���� �����Ѵ�.

CVHMIOWrapper <--> CVHMFilesystem : CVHMIOWrapper�� ������ I/O Wrapping �Լ��� ȣ���Ͽ� I/O�� �����Ѵ�.

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
#define VHM_ERROR_GENERIC						-1 // �Ϲ��� ����
#define VHM_ERROR_SUCCESS						0 // ����
#define VHM_ERROR_NOT_INITIALIZED				1 // �ʱ�ȭ���� ����
#define VHM_ERROR_INVALID_VHMDISK				2 // �ùٸ��� ���� VHMDisk ��ü
#define VHM_ERROR_ALREADY_OPENED				3 // �̹� ���� ����
#define VHM_ERROR_MEMORY_ALLOCATION_FAILED		4 // �޸� �Ҵ� ����
#define VHM_ERROR_NOT_OPENED					5 // �������� ����
#define VHM_ERROR_INVALID_PARAMETER				6 // �ùٸ��� ���� �Ű�����
#define VHM_ERROR_INVALID						7 // �ùٸ��� ����
#define VHM_ERROR_INTERNAL						8 // ���� ����
#define VHM_ERROR_DISK_IO						9 // ��ũ I/O ����
#define VHM_ERROR_DRIVER_NOT_FOUND				10 // (�ش� �׸� ����) ����̹��� ã�� �� ���� (���Ͻý��� ����̹� ��)
#define VHM_ERROR_UNSUPPORTED					11 // �������� �ʴ� ���
#define VHM_ERROR_BUFFER_TOO_SMALL				12 // ���� ũ�Ⱑ ������� ����
#define VHM_ERROR_PERMISSION_DENIED				13 // ���� �ź�
#define VHM_ERROR_NOT_FOUND						14 // ã�� �� ����
#define VHM_ERROR_NOT_AVAILABLE					15 // ��� �Ұ���
#define VHM_ERROR_TOO_BIG						16 // �ʹ� ŭ
#define VHM_ERROR_END							17 // �� (����, ����, ����, ...)

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

#define PART_TYPE_UNINITIALIZED			0	// �ʱ�ȭ���� ���� ��ũ
#define PART_TYPE_UNPARTITIONED_DISK	1	// ��Ƽ�Ŵ׵��� ���� (LBA 0�� ��Ƽ���� ù ������) ��ũ
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

#define GPT_HEADER_SIGNATURE			0x5452415020494645 /* EFI ��Ƽ�� ���� (ASCII: "EFI PART") */
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
	CVHMIOWrapper	*pVHMIOWrapper; // VHMIOWrapper�� �ε���� �ʾ����� nullptr
	CVHMFilesystem	*pVHMFilesystem; // VHMFilesystem�� �ε���� �ʾ����� nullptr

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
	DWORD	dwSectorSize; // GPT ��� �޸� �Ҵ� ũ��� ����
	DWORD	dwGPTPartitionEntrySize; // GPT ��Ƽ�� ��Ʈ�� ũ��
	DWORD	dwGPTPartitionEntryCount; // GPT ��Ƽ�� ��Ʈ�� ��
	GPT_HEADER	*pGPTHeaderPrimary;
	GPT_HEADER	*pGPTHeaderBackup;
	GPT_PARTITION_ENTRY	*pGPTPartitionEntries;

}VHMPARTITION_INTERNAL_GPT;

typedef struct _vhmpartition_internal_mbr
{
	DWORD dwSectorSize; // ���� ũ��: MBR �޸� �Ҵ� ũ��� ����
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
#define FILESYSTEM_STATUS_INVALID		0xFFFFFFFFFFFFFFFF // ���Ͻý��� ���°� �ùٸ��� ����
#define FILESYSTEM_STATUS_MOUNTED		0x0000000000000001 // ���Ͻý����� ����Ʈ��
#define FILESYSTEM_STATUS_VOLUME_OK		0x0000000000000002 // ���Ͻý��ۿ� ������ ����
#define FILESYSTEM_STATUS_READABLE		0x0000000000000004 // ���Ͻý����� ���� �� ����
#define FILESYSTEM_STATUS_WRITABLE		0x0000000000000008 // ���Ͻý��ۿ� �� �� ����

// VHMFilesystem - GetFilesystemCharacteristics()
#define FILESYSTEM_CHARACTERISTICS_NONE				0x0000000000000000 // Ư�� ����
#define FILESYSTEM_CHARACTERISTICS_JOURNALING		0x0000000000000001 // ���θ� ����
#define FILESYSTEM_CHARACTERISTICS_UNIX_PERMISSION	0x0000000000000002 // ���н� ���� ����
#define FILESYSTEM_CHARACTERISTICS_ACL				0x0000000000000004 // ACL (Access Control List) ����
#define FILESYSTEM_CHARACTERISTICS_HOT_RESIZE		0x0000000000000008 // ���Ͻý����� ����Ʈ�Ǿ��� �� �������� ����

/* VHMFilesystem - FObject (File/Folder) Flags */

// ��û�Ǵ� �׼��� ���� / ���� ���
#define ACCESS_RIGHT_DENY			0x00000000
#define ACCESS_RIGHT_READ			0x00000001
#define ACCESS_RIGHT_WRITE			0x00000002
#define ACCESS_RIGHT_RENAME			0x00000004
#define ACCESS_RIGHT_DELETE			0x00000008

// ��û�Ǵ� ���� ������ ����(����)
#define CREATE_NEW					1
#define CREATE_ALWAYS				2
#define OPEN_EXISTING				3
#define OPEN_ALWAYS					4
#define TRUNCATE_EXISTING			5

// ���Ͽ� ������ �׼��� ���� - �������� ������ ���� ����
#define FOBJ_ACCESS_RIGHT_OTHERS_READ		0x0000000000000004 // �ٸ� ����� - �б�
#define FOBJ_ACCESS_RIGHT_OTHERS_WRITE		0x0000000000000002 // �ٸ� ����� - ����
#define FOBJ_ACCESS_RIGHT_OTHERS_EXECUTE	0x0000000000000001 // �ٸ� ����� - ����
#define FOBJ_ACCESS_RIGHT_GROUP_READ		0x0000000000000020 // �׷쿡 ���� ����� - �б�
#define FOBJ_ACCESS_RIGHT_GROUP_WRITE		0x0000000000000010 // �׷쿡 ���� ����� - ����
#define FOBJ_ACCESS_RIGHT_GROUP_EXECUTE		0x0000000000000008 // �׷쿡 ���� ����� - ����
#define FOBJ_ACCESS_RIGHT_OWNER_READ		0x0000000000000100 // ������ - �б�
#define FOBJ_ACCESS_RIGHT_OWNER_WRITE		0x0000000000000080 // ������ - ����
#define FOBJ_ACCESS_RIGHT_OWNER_EXECUTE		0x0000000000000040 // ������ - ����
#define FOBJ_ACCESS_RIGHT_STICKY			0x0000000000000200 // �ٸ� ����� - ���� ����
#define FOBJ_ACCESS_RIGHT_SETGID			0x0000000000000400 // �����ϴ� ���� GID�� ���� GID�� ���� (Effective GID)
#define FOBJ_ACCESS_RIGHT_SETUID			0x0000000000000800 // �����ϴ� ���� UID�� ���� UID�� ���� (Effective UID)
#define FOBJ_ACCESS_RIGHT_UNSUPPORTED		0xFFFFFFFFFFFFFFFF // ���� ���� X -> ��ο��� �׼��� ���

// ���� �Ӽ�
#define FOBJ_ATTR_DIRECTORY				0x8000000000000000 // ���͸�
#define FOBJ_ATTR_NORMAL				0x0000000000000000 // �Ϲ� ���� �Ӽ�
#define FOBJ_ATTR_READ_ONLY				0x0000000000000001 // �б� ����
#define FOBJ_ATTR_HIDDEN				0x0000000000000002 // ����
#define FOBJ_ATTR_SYSTEM				0x0000000000000004 // �ý���
#define FOBJ_ATTR_ARCHIVE				0x0000000000000008 // ������� ����
#define FOBJ_ATTR_VOLUME_ID				0x0000000000000010 // ���� ���̺�
#define FOBJ_ATTR_TEMPORARY				0x0000000000000020 // �ӽ� ����
#define FOBJ_ATTR_ENCRYPTED				0x0000000000000040 // ��ȣȭ�� ����
#define FOBJ_ATTR_EXECUTABLE			0x0000000000000080 // ���� ����
#define FOBJ_ATTR_SYMLINK				0x0000000000000100 // �ɺ��� ��ũ
#define FOBJ_ATTR_HARDLINK				0x0000000000000200 // �ϵ� ��ũ

/* Directory Enumeration */

// FObject descriptor
typedef struct _fobj_descriptor
{
	QWORD	qwPointer; // ���� ������ (SEEK ��ġ)
	QWORD	qwFileSize; // ���� ũ��
	QWORD	qwFileAttributes; // ���� �Ӽ� (���� 32��Ʈ-����, ���� 32��Ʈ-���Ͻý��� Ư��)
	QWORD	qwFileAccessRights; // ���� �׼��� ����
	WCHAR	wName[1]; // FObject �̸� (��� ������)

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

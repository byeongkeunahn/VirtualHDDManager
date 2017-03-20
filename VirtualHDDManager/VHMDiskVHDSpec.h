
#pragma once

#pragma pack(push, 1)

// 섹터 크기 (고정)
#define VHD_SECTOR_SZ			512

// 버전
#define VHD_MAJOR_VER			0x0001
#define VHD_MINOR_VER			0x0000

// VHD Footer

#define VHD_FOOTER_MAJOR_VER	0x0001
#define VHD_FOOTER_MINOR_VER	0x0000

#define VHM_CREATE_OS_WIN		0x

#define VHD_CHS_MAKEDWORD(c,h,s)	((DWORD)(WORD)(c) | ((DWORD)(BYTE)(h) << 16) | ((DWORD)(BYTE)(s) << 24))
#define VHD_CHS_GETC(dw)		((WORD)(dw))
#define VHD_CHS_GETH(dw)		((BYTE)((DWORD)(dw) >> 16))
#define VHD_CHS_GETS(dw)		((BYTE)((DWORD)(dw) >> 24))

#define VHD_TYPE_NONE			0
#define VHD_TYPE_FIXED			2
#define VHD_TYPE_DYNAMIC		3
#define VHD_TYPE_DIFFERENCING	4

typedef struct VHD_FOOTER
{
	BYTE		cookie[8];
	UINT32		feature;
	UINT16		major_ver;
	UINT16		minor_ver;
	UINT64		data_off;
	DWORD		timestamp;
	DWORD		creator_app;
	DWORD		creator_ver;
	DWORD		creator_os;
	UINT64		original_sz;
	UINT64		current_sz;
	DWORD		disk_chs;
	DWORD		disk_type;
	DWORD		checksum;
	VHMUUID		uuid;
	BYTE		saved_state;
	BYTE		reserved[427];

}VHD_FOOTER;

// VHD dynamic disk header

#define VHD_DYNHDR_MAJOR_VER	0x1000
#define VHD_DYNHDR_MINOR_VER	0x0000

typedef struct VHD_PARENT_LOCATOR
{
	DWORD		platform_code;
	DWORD		platform_data_space;
	DWORD		platform_data_length;
	BYTE		reserved[4];
	UINT64		platform_data_off;

}VHD_PARENT_LOCATOR;

typedef struct VHD_DYN_DSK_HDR
{
	BYTE				cookie[8];
	UINT64				data_off;
	UINT64				table_off; // BAT offset (absolute)
	UINT16				major_ver;
	UINT16				minor_ver;
	UINT32				max_table_entry_count; // maximum BAT entry count
	DWORD				checksum;
	VHMUUID				parent_uuid;
	DWORD				parent_timestamp;
	BYTE				reserved1[4];
	WCHAR				parent_unicode_name[16];
	VHD_PARENT_LOCATOR	parent_locator_entry[8];
	BYTE				reserved2[256];

}VHD_DYN_DSK_HDR;

#pragma pack(pop)

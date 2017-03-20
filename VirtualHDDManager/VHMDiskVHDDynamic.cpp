
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMDiskVHDSpec.h"
#include "VHMDiskVHD.h"

const BYTE g_vhd_dyn_dsk_hdr_cookie[8] = { 'c', 'x', 's', 'p', 'a', 'r', 's', 'e' };

typedef struct
{
	VHD_FOOTER		*vhd_footer;
	VHD_DYN_DSK_HDR	*vhd_dyn_dsk_hdr;
	UINT64			bat_off; // BAT absolute offset
	UINT64			bat_max_entry; // BAT max entry count
	UINT64			data_block_off; // data block start absolute offset
	UINT64			data_block_count; // current data block count

} VHD_DYN_INTERNAL;

int CVHMDiskVHD::DynamicCreateDisk(CFile *pFile, DWORD dwSectorSize, QWORD qwSectorCount)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::DynamicOpenDisk(CFile *pFile, VHD_FOOTER *vhd_footer)
{
	return VHM_ERROR_INTERNAL;

	if (!pFile || !vhd_footer)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_DYN_INTERNAL *vhd_dyn_internal;
	VHD_DYN_DSK_HDR *vhd_dyn_dsk_hdr;
	UINT64 uiDynDskHdrOff;
	DWORD dwChecksum;

	vhd_dyn_dsk_hdr = nullptr;

	if (vhd_footer->major_ver > VHD_MAJOR_VER)
		return VHM_ERROR_UNSUPPORTED;

	// Dynamic disk header 읽기
	uiDynDskHdrOff = vhd_footer->data_off;
	vhd_dyn_dsk_hdr = (VHD_DYN_DSK_HDR *)malloc(sizeof(VHD_DYN_DSK_HDR));
	memset(vhd_dyn_dsk_hdr, 0, sizeof(VHD_DYN_DSK_HDR));
	pFile->Seek(uiDynDskHdrOff, CFile::begin);
	pFile->Read(vhd_dyn_dsk_hdr, sizeof(VHD_DYN_DSK_HDR));
	SToN_DynDskHdr(vhd_dyn_dsk_hdr, vhd_dyn_dsk_hdr);
	
	// Dynamic disk header 확인
	ChecksumDynDskHdr(vhd_dyn_dsk_hdr, &dwChecksum);
	if (dwChecksum != vhd_dyn_dsk_hdr->checksum)
		goto error;

	if (memcmp(g_vhd_dyn_dsk_hdr_cookie, vhd_dyn_dsk_hdr->cookie, 8) != 0)
		goto error;

	if (vhd_dyn_dsk_hdr->major_ver > VHD_MAJOR_VER)
		goto error;

	// 내부 헤더 할당
	vhd_dyn_internal = (VHD_DYN_INTERNAL *)malloc(sizeof(VHD_DYN_INTERNAL));
	vhd_dyn_internal->vhd_footer = (VHD_FOOTER *)malloc(sizeof(VHD_FOOTER));
	CopyMemoryBlock(vhd_dyn_internal->vhd_footer, vhd_footer, sizeof(VHD_FOOTER));
	vhd_dyn_internal->vhd_dyn_dsk_hdr = vhd_dyn_dsk_hdr;
	vhd_dyn_internal->bat_off = vhd_dyn_dsk_hdr->table_off;
	vhd_dyn_internal->bat_max_entry = vhd_dyn_dsk_hdr->max_table_entry_count;
	

	m_pData = vhd_dyn_internal;
	


	m_dwDiskType = VHD_TYPE_DYNAMIC;
	m_pData = malloc(sizeof(VHD_FOOTER));
	CopyMemoryBlock(m_pData, vhd_footer, sizeof(VHD_FOOTER));

	return VHM_ERROR_SUCCESS;

error:
	if (vhd_dyn_dsk_hdr)
		free(vhd_dyn_dsk_hdr);

	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::DynamicCloseDisk()
{
	// 메모리 해제
	VHD_DYN_INTERNAL *vhd_dyn_internal = (VHD_DYN_INTERNAL *)m_pData;
	free(vhd_dyn_internal->vhd_footer);
	free(vhd_dyn_internal->vhd_dyn_dsk_hdr);
	free(m_pData);
	m_pData = nullptr;

	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::DynamicReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::DynamicWriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::DynamicReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::DynamicWriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	return VHM_ERROR_UNSUPPORTED;
}
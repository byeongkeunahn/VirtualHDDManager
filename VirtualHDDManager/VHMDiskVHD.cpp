
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMDiskVHDSpec.h"
#include "VHMDiskVHD.h"

CVHMDiskVHD::CVHMDiskVHD(void)
{

}

CVHMDiskVHD::~CVHMDiskVHD(void)
{

}

int CVHMDiskVHD::CreateDisk(const wchar_t *wszFilePath, DWORD dwSectorSize, QWORD qwSectorCount, QWORD qwFlags)
{
	return VHM_ERROR_UNSUPPORTED;

	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	if(dwSectorSize < 512 || !IsPowerOfTwo(dwSectorSize))
		return VHM_ERROR_INVALID_PARAMETER;

	// 섹터 수는 0이 아니어야 한다.
	if(qwSectorCount == 0)
		return VHM_ERROR_INVALID_PARAMETER;

	// wszFilePath에 주어진 매개변수로 VHD 파일을 생성한다.
	if(m_fileVHD.Open(wszFilePath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
	{
		return VHM_ERROR_DISK_IO;
	}

	// 멤버 변수 설정
	m_bOpened = TRUE;
	m_dwSectorSize = dwSectorSize;
	m_qwSectorCount = qwSectorCount;

	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::OpenDisk(const wchar_t *wszFilePath)
{
	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	// wszFilePath에 주어진 매개변수로 VHD 파일을 연다.
	if(m_fileVHD.Open(wszFilePath, CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
		return VHM_ERROR_DISK_IO;

	UINT64 uiFileSize;
	UINT64 uiFooterOff;
	UINT64 uiFooterSz;
	VHD_FOOTER vhdFooter;
	DWORD dwChecksum;

	// 파일 크기 = 0 또는 511 (mod 512)
	uiFileSize = m_fileVHD.GetLength();
	if (uiFileSize % VHD_SECTOR_SZ != 0 && uiFileSize % VHD_SECTOR_SZ != (VHD_SECTOR_SZ - 1))
		goto error;

	// 헤더 읽기 (파일 끝에 존재)
	uiFooterOff = uiFileSize % VHD_SECTOR_SZ == 0 ? (uiFileSize - VHD_SECTOR_SZ) : (uiFileSize - (VHD_SECTOR_SZ - 1));
	uiFooterSz = uiFileSize - uiFooterOff;
	memset(&vhdFooter, 0, sizeof(VHD_FOOTER));
	m_fileVHD.Seek(uiFooterOff, CFile::begin);
	m_fileVHD.Read(&vhdFooter, (UINT)uiFooterSz);
	SToN_Footer(&vhdFooter, &vhdFooter);

	// 헤더 Checksum 계산 및 확인
	ChecksumFooter(&vhdFooter, &dwChecksum);
	if (dwChecksum != vhdFooter.checksum)
		goto error;
	
	m_fileVHD.SeekToBegin();

	switch (vhdFooter.disk_type)
	{
	case VHD_TYPE_FIXED:
		if (FixedOpenDisk(&m_fileVHD, &vhdFooter) != VHM_ERROR_SUCCESS) goto error;
		break;
	case VHD_TYPE_DYNAMIC:
		if (DynamicOpenDisk(&m_fileVHD, &vhdFooter) != VHM_ERROR_SUCCESS) goto error;
		break;
	case VHD_TYPE_DIFFERENCING:
	default:
		m_fileVHD.Close();
		return VHM_ERROR_UNSUPPORTED;
	}	

	// 멤버 변수 설정
	m_bOpened = TRUE;
	
	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;

error:
	m_fileVHD.Close();
	return VHM_ERROR_INVALID;
}

int CVHMDiskVHD::CloseDisk()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	int err_code;

	UninitializeVHMPartition();
	UninitializeVHMIOWrapper();

	switch (m_dwDiskType)
	{
	case VHD_TYPE_FIXED:
		if ((err_code = FixedCloseDisk()) != VHM_ERROR_SUCCESS) return err_code;
		break;
	case VHD_TYPE_DYNAMIC:
		if ((err_code = DynamicCloseDisk()) != VHM_ERROR_SUCCESS) return err_code;
		break;
	default:
		return VHM_ERROR_INTERNAL;
	}

	m_fileVHD.Close();
	m_bOpened = FALSE;

	m_dwSectorSize = 0;
	m_qwSectorCount = 0;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	switch (m_dwDiskType)
	{
	case VHD_TYPE_FIXED:
		return FixedReadOneSector(uiSectorLBA, pBuffer, szBuffer);
	case VHD_TYPE_DYNAMIC:
		return DynamicReadOneSector(uiSectorLBA, pBuffer, szBuffer);
	}

	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	switch (m_dwDiskType)
	{
	case VHD_TYPE_FIXED:
		return FixedWriteOneSector(uiSectorLBA, pBuffer);
	case VHD_TYPE_DYNAMIC:
		return DynamicWriteOneSector(uiSectorLBA, pBuffer);
	}

	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	switch (m_dwDiskType)
	{
	case VHD_TYPE_FIXED:
		return FixedReadSector(uiStartingLBA, uiSectorCount, pBuffer, szBuffer);
	case VHD_TYPE_DYNAMIC:
		return DynamicReadSector(uiStartingLBA, uiSectorCount, pBuffer, szBuffer);
	}

	return VHM_ERROR_UNSUPPORTED;
}

int CVHMDiskVHD::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if (!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	switch (m_dwDiskType)
	{
	case VHD_TYPE_FIXED:
		return FixedWriteSector(uiStartingLBA, uiSectorCount, pBuffer);
	case VHD_TYPE_DYNAMIC:
		return DynamicWriteSector(uiStartingLBA, uiSectorCount, pBuffer);
	}

	return VHM_ERROR_UNSUPPORTED;
}

DWORD CVHMDiskVHD::GetDiskType()
{
	if(!IsOpened())
		return VIRTUAL_DISK_FORMAT_INVALID;

	return VIRTUAL_DISK_FORMAT_VHD;
}

int CVHMDiskVHD::ChecksumFooter(VHD_FOOTER *vhd_footer, DWORD *pdwChecksum)
{
	DWORD dwChecksum_prev;
	DWORD dwChecksum;
	BYTE *pbuf_header;
	int i;

	dwChecksum_prev = vhd_footer->checksum;
	vhd_footer->checksum = 0;
	pbuf_header = (BYTE *)vhd_footer;

	for (i = 0, dwChecksum = 0; i < sizeof(VHD_FOOTER); ++i)
		dwChecksum += pbuf_header[i];

	vhd_footer->checksum = dwChecksum_prev;

	dwChecksum = ~dwChecksum;
	*pdwChecksum = dwChecksum;

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::ChecksumDynDskHdr(VHD_DYN_DSK_HDR *vhd_dyn_dsk_hdr, DWORD *pdwChecksum)
{
	DWORD dwChecksum_prev;
	DWORD dwChecksum;
	BYTE *pbuf_header;
	int i;

	dwChecksum_prev = vhd_dyn_dsk_hdr->checksum;
	vhd_dyn_dsk_hdr->checksum = 0;
	pbuf_header = (BYTE *)vhd_dyn_dsk_hdr;

	for (i = 0, dwChecksum = 0; i < sizeof(VHD_DYN_DSK_HDR); ++i)
		dwChecksum += pbuf_header[i];

	vhd_dyn_dsk_hdr->checksum = dwChecksum_prev;

	dwChecksum = ~dwChecksum;
	*pdwChecksum = dwChecksum;

	return VHM_ERROR_SUCCESS;
}

#define VHD_STON(a) out->a = BToN(in->a);
#define VHD_NTOS(a) out->a = NToB(in->a);

int CVHMDiskVHD::SToN_Footer(VHD_FOOTER *in, VHD_FOOTER *out)
{
	if (!in || !out)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_STON(feature);
	VHD_STON(major_ver);
	VHD_STON(minor_ver);
	VHD_STON(data_off);
	VHD_STON(timestamp);
	VHD_STON(creator_app);
	VHD_STON(creator_ver);
	VHD_STON(creator_os);
	VHD_STON(original_sz);
	VHD_STON(current_sz);
	VHD_STON(disk_chs);
	VHD_STON(disk_type);
	VHD_STON(checksum);
	VHD_STON(uuid.Data1);
	VHD_STON(uuid.Data2);
	VHD_STON(uuid.Data3);
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::NToS_Footer(VHD_FOOTER *in, VHD_FOOTER *out)
{
	if (!in || !out)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_NTOS(feature);
	VHD_NTOS(major_ver);
	VHD_NTOS(minor_ver);
	VHD_NTOS(data_off);
	VHD_NTOS(timestamp);
	VHD_NTOS(creator_app);
	VHD_NTOS(creator_ver);
	VHD_NTOS(creator_os);
	VHD_NTOS(original_sz);
	VHD_NTOS(current_sz);
	VHD_NTOS(disk_chs);
	VHD_NTOS(disk_type);
	VHD_NTOS(checksum);
	VHD_NTOS(uuid.Data1);
	VHD_NTOS(uuid.Data2);
	VHD_NTOS(uuid.Data3);

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::SToN_ParentLocator(VHD_PARENT_LOCATOR *in, VHD_PARENT_LOCATOR *out)
{
	if (!in || !out)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_STON(platform_code);
	VHD_STON(platform_data_space);
	VHD_STON(platform_data_off);

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::NToS_ParentLocator(VHD_PARENT_LOCATOR *in, VHD_PARENT_LOCATOR *out)
{
	if (!in || !out)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_NTOS(platform_code);
	VHD_NTOS(platform_data_space);
	VHD_NTOS(platform_data_off);

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::SToN_DynDskHdr(VHD_DYN_DSK_HDR *in, VHD_DYN_DSK_HDR *out)
{
	if (!in || !out)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_STON(data_off);
	VHD_STON(table_off);
	VHD_STON(major_ver);
	VHD_STON(minor_ver);
	VHD_STON(max_table_entry_count);
	VHD_STON(checksum);
	VHD_STON(parent_uuid.Data1);
	VHD_STON(parent_uuid.Data2);
	VHD_STON(parent_uuid.Data3);
	VHD_STON(parent_timestamp);

	for (int i = 0; i < 16; ++i)
		out->parent_unicode_name[i] = (WCHAR)BToN((UINT16)(in->parent_unicode_name[i]));
	for (int i = 0; i < 8; ++i)
		SToN_ParentLocator(&in->parent_locator_entry[i], &out->parent_locator_entry[i]);

	return VHM_ERROR_SUCCESS;
}

int CVHMDiskVHD::NToS_DynDskHdr(VHD_DYN_DSK_HDR *in, VHD_DYN_DSK_HDR *out)
{
	if (!in || !out)
		return VHM_ERROR_INVALID_PARAMETER;

	VHD_NTOS(data_off);
	VHD_NTOS(table_off);
	VHD_NTOS(major_ver);
	VHD_NTOS(minor_ver);
	VHD_NTOS(max_table_entry_count);
	VHD_NTOS(checksum);
	VHD_NTOS(parent_uuid.Data1);
	VHD_NTOS(parent_uuid.Data2);
	VHD_NTOS(parent_uuid.Data3);
	VHD_NTOS(parent_timestamp);

	for (int i = 0; i < 16; ++i)
		out->parent_unicode_name[i] = (WCHAR)NToB((UINT16)(in->parent_unicode_name[i]));
	for (int i = 0; i < 8; ++i)
		NToS_ParentLocator(&in->parent_locator_entry[i], &out->parent_locator_entry[i]);

	return VHM_ERROR_SUCCESS;
}

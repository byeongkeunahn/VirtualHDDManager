
#pragma once

struct VHD_FOOTER;
struct VHD_PARENT_LOCATOR;
struct VHD_DYN_DSK_HDR;

class CVHMDiskVHD : public CVHMDisk
{
public:
	CVHMDiskVHD(void);
	virtual ~CVHMDiskVHD(void);

	virtual int CreateDisk(const wchar_t *wszFilePath, DWORD dwSectorSize, QWORD qwSectorCount, QWORD qwFlags);
	virtual int OpenDisk(const wchar_t *wszFilePath);
	virtual int CloseDisk();
	
	virtual int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	virtual int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer);

	virtual int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	virtual int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);

	virtual DWORD GetDiskType();

protected:

	/* 내부 함수 */


	// Fixed VHD

	int FixedCreateDisk(CFile *pFile, DWORD dwSectorSize, QWORD qwSectorCount);
	int FixedOpenDisk(CFile *pFile, VHD_FOOTER *vhd_footer);
	int FixedCloseDisk();

	int FixedReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	int FixedWriteOneSector(UINT64 uiSectorLBA, void *pBuffer);

	int FixedReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	int FixedWriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);


	// Dynamic VHD

	int DynamicCreateDisk(CFile *pFile, DWORD dwSectorSize, QWORD qwSectorCount);
	int DynamicOpenDisk(CFile *pFile, VHD_FOOTER *vhd_footer);
	int DynamicCloseDisk();

	int DynamicReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	int DynamicWriteOneSector(UINT64 uiSectorLBA, void *pBuffer);

	int DynamicReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	int DynamicWriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);


	// Checksum calculation

	int ChecksumFooter(VHD_FOOTER *vhd_footer, DWORD *pdwChecksum);
	int ChecksumDynDskHdr(VHD_DYN_DSK_HDR *vhd_dyn_dsk_hdr, DWORD *pdwChecksum);

	
	// Endian conversion (S: Specification, N: Native)
	// 'in': input, 'out': output
	// 'in' is converted and stored in 'out'

	int SToN_Footer(VHD_FOOTER *in, VHD_FOOTER *out);
	int NToS_Footer(VHD_FOOTER *in, VHD_FOOTER *out);

	int SToN_ParentLocator(VHD_PARENT_LOCATOR *in, VHD_PARENT_LOCATOR *out);
	int NToS_ParentLocator(VHD_PARENT_LOCATOR *in, VHD_PARENT_LOCATOR *out);

	int SToN_DynDskHdr(VHD_DYN_DSK_HDR *in, VHD_DYN_DSK_HDR *out);
	int NToS_DynDskHdr(VHD_DYN_DSK_HDR *in, VHD_DYN_DSK_HDR *out);

protected:

	/* 내부 변수 */

	CFile		m_fileVHD;
	DWORD		m_dwDiskType;
	void		*m_pData;	
};


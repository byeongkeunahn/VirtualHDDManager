
#pragma once

class CVHMDiskRam : public CVHMDisk
{
public:
	CVHMDiskRam(void);
	virtual ~CVHMDiskRam(void);

	virtual int CreateDisk(DWORD dwSectorSize, QWORD qwSectorCount);
	virtual int CloseDisk();

	virtual int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	virtual int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer);

	virtual int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	virtual int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);

	virtual DWORD GetDiskType();

protected:
	BYTE		*m_pDiskData;
};


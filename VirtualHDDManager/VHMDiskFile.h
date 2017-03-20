
#pragma once

class CVHMDiskFile : public CVHMDisk
{
public:
	CVHMDiskFile(void);
	virtual ~CVHMDiskFile(void);

	virtual int CreateDisk(const wchar_t *wszFilePath, DWORD dwSectorSize, QWORD qwSectorCount);
	virtual int OpenDisk(const wchar_t *wszFilePath);
	virtual int OpenDiskRaw(const wchar_t *wszFilePath);
	virtual int CloseDisk();
	
	virtual int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer);
	virtual int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer);

	virtual int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer);
	virtual int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer);

	virtual DWORD GetDiskType();

protected:
	CFile		m_fileVHM;
	UINT64		m_uiHeaderLength;
};


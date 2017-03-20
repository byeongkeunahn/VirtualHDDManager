
/* StreamFile.h */

#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMCOMMON_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


class VHMDLL CStreamFile : public CStream
{
public:
	CStreamFile();
	virtual ~CStreamFile();

	virtual int Create(LPCTSTR lpszFilePath);
	virtual int Close();

	virtual int Read(void *pBuffer, vhmsize_t szBufRead);
	virtual int Write(const void *pBuffer, vhmsize_t szBufWrite);
	virtual int Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer = nullptr);
	virtual UINT64 Tell();
	virtual UINT64 GetSize();
};


#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMCOMMON_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


/* CStreamBlock */

class CStream;
class CStreamMemDyn;

class VHMDLL CStreamBlock
{
public:
	CStreamBlock();
	virtual ~CStreamBlock();

	virtual int Create();
	virtual int Close();
	virtual BOOL IsOpened();
protected:
	virtual int SetOpenState(BOOL bOpened);

public:
	virtual int Alloc(vhmsize_t size, vhmsize_t *poff);
	virtual int Free(vhmsize_t off, vhmsize_t size);

	virtual vhmsize_t GetStreamSize();
	virtual vhmsize_t GetUsedSize();


protected:
	BOOL m_bOpened;

	CStream *m_pStream;
	CStreamMemDyn *m_pStreamMemDyn;
};

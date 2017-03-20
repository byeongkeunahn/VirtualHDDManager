
/* StreamMemDynamic.h */

#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMCOMMON_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


#define STREAM_MEM_DYN_DEF_GRANULARITY    (65536)

/* CStreamMemDyn */

class VHMDLL CStreamMemDyn : public CStream
{
public:
	CStreamMemDyn();
	virtual ~CStreamMemDyn();

	virtual int Create(vhmsize_t szMax = 0, vhmsize_t szGranularity = STREAM_MEM_DYN_DEF_GRANULARITY); // szStreamMax = 0 for unlimited writing
	virtual int Close();

	virtual int Read(void *pBuffer, vhmsize_t szBufRead);
	virtual int Write(const void *pBuffer, vhmsize_t szBufWrite);
	virtual int Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer = nullptr);
	virtual UINT64 Tell();
	virtual UINT64 GetSize();

protected:
	int AdjustBufferSize(vhmsize_t szNew);

protected:
	vhmsize_t m_uiPointer;
	BYTE *m_pBuffer;
	vhmsize_t m_szBuffer;
	vhmsize_t m_szBufferMax;
	vhmsize_t m_szGranularity;
};

/* CStreamMemDynTmpl */

template <typename T> class CStreamMemDynTmpl : public CStreamTmpl<T>
{
public:
	CStreamMemDynTmpl();
	virtual ~CStreamMemDynTmpl();

	virtual int Create(vhmsize_t ccMax = 0, vhmsize_t ccGranularity = STREAM_MEM_DYN_DEF_GRANULARITY); // szStreamMax = 0 for unlimited writing
};

template <typename T> CStreamMemDynTmpl<T>::CStreamMemDynTmpl()
{
	m_pStream = new CStreamMemDyn;
}

template <typename T> CStreamMemDynTmpl<T>::~CStreamMemDynTmpl()
{
	delete m_pStream;
}

template <typename T> int CStreamMemDynTmpl<T>::Create(vhmsize_t ccMax, vhmsize_t ccGranularity)
{
	return ((CStreamMemDyn *)m_pStream)->Create(ccMax * sizeof(T), ccGranularity * sizeof(T));
}

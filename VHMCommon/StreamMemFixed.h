
/* StreamMemFixed.h */

#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMCOMMON_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


/* CStreamMemFixed */

class VHMDLL CStreamMemFixed : public CStream
{
public:
	CStreamMemFixed();
	virtual ~CStreamMemFixed();

	virtual int Create(vhmsize_t szStream);
	virtual int Close();

	virtual int Read(void *pBuffer, vhmsize_t szBufRead);
	virtual int Write(const void *pBuffer, vhmsize_t szBufWrite);
	virtual int Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer = nullptr);
	virtual UINT64 Tell();
	virtual UINT64 GetSize();

protected:
	vhmsize_t m_uiPointer;
	BYTE *m_pBuffer;
	vhmsize_t m_szBuffer;
};

/* CStreamMemFixedTmpl */

template <typename T> class CStreamMemFixedTmpl : public CStreamTmpl<T>
{
public:
	CStreamMemFixedTmpl();
	virtual ~CStreamMemFixedTmpl();

	virtual int Create(vhmsize_t ccStream);
};

template <typename T> CStreamMemFixedTmpl<T>::CStreamMemFixedTmpl()
{
	m_pStream = new CStreamMemFixed;
}

template <typename T> CStreamMemFixedTmpl<T>::~CStreamMemFixedTmpl()
{
	delete m_pStream;
}

template <typename T> int CStreamMemFixedTmpl<T>::Create(vhmsize_t ccStream)
{
	return ((CStreamMemFixed *)m_pStream)->Create(ccStream * sizeof(T));
}

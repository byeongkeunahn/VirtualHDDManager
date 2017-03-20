
/* Stream.h */

#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMCOMMON_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


#define SSEEK_BEGIN    0
#define SSEEK_CUR      1
#define SSEEK_END      2

/* CStream */

class VHMDLL CStream
{
public:
	CStream();
	virtual ~CStream();

	// NOTE: there isn't a Create() function (not necessary in abstract class)
	virtual int Close();
	virtual BOOL IsOpened();
protected:
	virtual int SetOpenState(BOOL bOpened);

public:
	virtual int Read(void *pBuffer, vhmsize_t szBufRead) = 0;
	virtual int Write(const void *pBuffer, vhmsize_t szBufWrite) = 0;
	virtual int Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer = nullptr) = 0;
	virtual UINT64 Tell() = 0;
	virtual UINT64 GetSize() = 0;

	virtual int GetFlags(UINT32 *puiFlags);
	virtual int SetFlags(UINT32 uiSFlags);
	virtual int GetSFlags(UINT32 *puiFlags);
	virtual int SetSFlags(UINT32 uiSFlags);

protected:
	BOOL m_bOpened;
	UINT32 m_uiFlags;
	UINT32 m_uiSFlags;
};

/* CStreamTmpl */

template <typename T> class CStreamTmpl
{
public:
	CStreamTmpl();
	virtual ~CStreamTmpl();

	virtual int Close();
	virtual BOOL IsOpened();

	virtual int Read(T *pBuffer, vhmsize_t ccBufRead);
	virtual int Write(const T *pBuffer, vhmsize_t ccBufWrite);
	virtual int Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer);
	virtual UINT64 Tell();
	virtual UINT64 GetCount();

protected:
	CStream *m_pStream;
};

template <typename T> CStreamTmpl<T>::CStreamTmpl()
{

}

template <typename T> CStreamTmpl<T>::~CStreamTmpl()
{

}

template <typename T> int CStreamTmpl<T>::Close()
{
	return m_pStream->Close();
}

template <typename T> BOOL CStreamTmpl<T>::IsOpened()
{
	return m_pStream->IsOpened();
}

template <typename T> int CStreamTmpl<T>::Read(T *pBuffer, vhmsize_t ccBufRead)
{
	return m_pStream->Read(pBuffer, ccBufRead * sizeof(T));
}

template <typename T> int CStreamTmpl<T>::Write(const T *pBuffer, vhmsize_t ccBufWrite)
{
	return m_pStream->Write(pBuffer, ccBufWrite * sizeof(T));
}

template <typename T> int CStreamTmpl<T>::Seek(INT64 iOff, UINT nMoveMethod, UINT64 *pNewPointer)
{
	return m_pStream->Seek(iOff * sizeof(T), nMoveMethod, pNewPointer);
}

template <typename T> UINT64 CStreamTmpl<T>::Tell()
{
	return m_pStream->Tell() / sizeof(T);
}

template <typename T> UINT64 CStreamTmpl<T>::GetCount()
{
	return m_pStream->GetSize() / sizeof(T);
}

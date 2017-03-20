
#include "stdafx.h"
#include "VHMBase.h"
#include "Stream.h"
#include "StreamMemDyn.h"
#include "StreamBlock.h"

CStreamBlock::CStreamBlock()
{
	SetOpenState(FALSE);
}

CStreamBlock::~CStreamBlock()
{

}

int CStreamBlock::Create()
{

}

int CStreamBlock::Close()
{

}

BOOL CStreamBlock::IsOpened()
{

}

int CStreamBlock::SetOpenState(BOOL bOpened)
{
	m_bOpened = bOpened;
	return VHM_ERROR_SUCCESS;
}

int CStreamBlock::Alloc(vhmsize_t size, vhmsize_t *poff)
{

}

int CStreamBlock::Free(vhmsize_t off, vhmsize_t size)
{

}

vhmsize_t CStreamBlock::GetStreamSize()
{
	if (!IsOpened())
		return VHM_INVALID_VALUE;

	return m_pStream->Tell();
}

vhmsize_t CStreamBlock::GetUsedSize()
{

}

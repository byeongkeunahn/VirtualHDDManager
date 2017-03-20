
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMIOWrapper.h"

#define VHMIOWRAPPER_TYPE_INVALID		-1
#define VHMIOWRAPPER_TYPE_DISK			1
#define VHMIOWRAPPER_TYPE_IOWRAPPER		2

CVHMIOWrapper::CVHMIOWrapper()
{
	m_bInitialized = FALSE;
	m_iIOObjectType = VHMIOWRAPPER_TYPE_INVALID;
	m_pIOObject = nullptr;
	m_uiStartingLBA = 0;
	m_uiSectorCount = 0;
	m_dwSectorSize = 0;
}

CVHMIOWrapper::~CVHMIOWrapper()
{
	Uninitialize();
}

BOOL CVHMIOWrapper::IsInitialized()
{
	return m_bInitialized;
}

int CVHMIOWrapper::Initialize(CVHMDisk *pVHMDisk, UINT64 uiStartingLBA, UINT64 uiSectorCount)
{
	if(IsInitialized())
		return VHM_ERROR_ALREADY_OPENED;

	if(!pVHMDisk /* VHMDisk 객체가 올바르지 않음 */ ||
		!uiSectorCount /* 섹터 갯수가 올바르지 않음 */||
		(uiStartingLBA + uiSectorCount - 1) >= pVHMDisk->GetSectorCount() /* 파티션의 마지막 섹터가 디스크에 존재하지 않음 */)
		return VHM_ERROR_INVALID_PARAMETER;

	/* 매개변수의 유효성을 검사하였으므로 값 대입 */
	m_iIOObjectType = VHMIOWRAPPER_TYPE_DISK;
	m_pIOObject = pVHMDisk;
	m_uiStartingLBA = uiStartingLBA;
	m_uiSectorCount = uiSectorCount;
	m_dwSectorSize = pVHMDisk->GetSectorSize();

	m_bInitialized = TRUE;
	return VHM_ERROR_SUCCESS;
}

int CVHMIOWrapper::Initialize(CVHMIOWrapper *pVHMIOWrapper, UINT64 uiStartingLBA, UINT64 uiSectorCount)
{
	if(IsInitialized())
		return VHM_ERROR_ALREADY_OPENED;

	if(!pVHMIOWrapper /* VHMIOWrapper 객체가 올바르지 않음 */ ||
		!uiSectorCount /* 섹터 갯수가 올바르지 않음 */||
		(uiStartingLBA + uiSectorCount - 1) >= pVHMIOWrapper->GetSectorCount() /* 파티션의 마지막 섹터가 디스크에 존재하지 않음 */)
		return VHM_ERROR_INVALID_PARAMETER;

	/* 매개변수의 유효성을 검사하였으므로 값 대입 */
	m_iIOObjectType = VHMIOWRAPPER_TYPE_IOWRAPPER;
	m_pIOObject = pVHMIOWrapper;
	m_uiStartingLBA = uiStartingLBA;
	m_uiSectorCount = uiSectorCount;
	m_dwSectorSize = pVHMIOWrapper->GetSectorSize();

	m_bInitialized = TRUE;
	return VHM_ERROR_SUCCESS;
}

int CVHMIOWrapper::Initialize(CVHMIOWrapper *pVHMIOWrapper, DWORD dwSectorSize, UINT64 uiStartingLBA, UINT64 uiSectorCount)
{
	if (IsInitialized())
		return VHM_ERROR_ALREADY_OPENED;

	return VHM_ERROR_UNSUPPORTED;
}

int CVHMIOWrapper::Uninitialize()
{
	if(!IsInitialized())
		return VHM_ERROR_SUCCESS;

	/* 매개변수 초기화 */
	m_bInitialized = FALSE;
	m_iIOObjectType = VHMIOWRAPPER_TYPE_INVALID;
	m_pIOObject = nullptr;
	m_uiStartingLBA = 0;
	m_uiSectorCount = 0;
	m_dwSectorSize = 0;

	return VHM_ERROR_SUCCESS;
}

int CVHMIOWrapper::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	if (!IsInitialized())
		return VHM_ERROR_NOT_OPENED;

	if (uiSectorLBA >= GetSectorCount())
		return VHM_ERROR_INVALID_PARAMETER;

	UINT64 uiRealSectorLBA = uiSectorLBA + m_uiStartingLBA;

	switch (m_iIOObjectType)
	{
	case VHMIOWRAPPER_TYPE_DISK:
		return ((CVHMDisk *)m_pIOObject)->ReadOneSector(uiRealSectorLBA, pBuffer, szBuffer);
	case VHMIOWRAPPER_TYPE_IOWRAPPER:
		return ((CVHMIOWrapper *)m_pIOObject)->ReadOneSector(uiRealSectorLBA, pBuffer, szBuffer);
	}

	return VHM_ERROR_INTERNAL;
}

int CVHMIOWrapper::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	if (!IsInitialized())
		return VHM_ERROR_NOT_OPENED;

	if (uiSectorLBA >= GetSectorCount())
		return VHM_ERROR_INVALID_PARAMETER;

	UINT64 uiRealSectorLBA = uiSectorLBA + m_uiStartingLBA;

	switch (m_iIOObjectType)
	{
	case VHMIOWRAPPER_TYPE_DISK:
		return ((CVHMDisk *)m_pIOObject)->WriteOneSector(uiRealSectorLBA, pBuffer);
	case VHMIOWRAPPER_TYPE_IOWRAPPER:
		return ((CVHMIOWrapper *)m_pIOObject)->WriteOneSector(uiRealSectorLBA, pBuffer);
	}

	return VHM_ERROR_INTERNAL;
}

int CVHMIOWrapper::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsInitialized())
		return VHM_ERROR_INTERNAL;

	UINT64 uiLastSector;
	uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if(uiLastSector >= GetSectorCount())
		return VHM_ERROR_INVALID_PARAMETER;

	UINT64 uiRealStartingLBA = uiStartingLBA + m_uiStartingLBA;

	switch(m_iIOObjectType)
	{
	case VHMIOWRAPPER_TYPE_DISK:
		return ((CVHMDisk *)m_pIOObject)->ReadSector(uiRealStartingLBA, uiSectorCount, pBuffer, szBuffer);
	case VHMIOWRAPPER_TYPE_IOWRAPPER:
		return ((CVHMIOWrapper *)m_pIOObject)->ReadSector(uiRealStartingLBA, uiSectorCount, pBuffer, szBuffer);
	}

	return VHM_ERROR_INTERNAL;
}

int CVHMIOWrapper::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if(!IsInitialized())
		return VHM_ERROR_INTERNAL;

	UINT64 uiLastSector;
	uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if(uiLastSector >= GetSectorCount())
		return VHM_ERROR_INVALID_PARAMETER;

	UINT64 uiRealStartingLBA = uiStartingLBA + m_uiStartingLBA;

	switch(m_iIOObjectType)
	{
	case VHMIOWRAPPER_TYPE_DISK:
		return ((CVHMDisk *)m_pIOObject)->WriteSector(uiRealStartingLBA, uiSectorCount, pBuffer);
	case VHMIOWRAPPER_TYPE_IOWRAPPER:
		return ((CVHMIOWrapper *)m_pIOObject)->WriteSector(uiRealStartingLBA, uiSectorCount, pBuffer);
	}
	
	return VHM_ERROR_INTERNAL;
}

DWORD CVHMIOWrapper::GetSectorSize()
{
	if(!IsInitialized())
		return 0; /* ERROR */

	return m_dwSectorSize;
}

QWORD CVHMIOWrapper::GetSectorCount()
{
	if(!IsInitialized())
		return 0; /* ERROR */

	return m_uiSectorCount;
}

BOOL VHMIOWrapperIsObjectValid(CVHMIOWrapper *pVHMIOWrapper)
{
	if(!pVHMIOWrapper)
		return FALSE;

	return pVHMIOWrapper->IsInitialized();
}


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

	if(!pVHMDisk /* VHMDisk ��ü�� �ùٸ��� ���� */ ||
		!uiSectorCount /* ���� ������ �ùٸ��� ���� */||
		(uiStartingLBA + uiSectorCount - 1) >= pVHMDisk->GetSectorCount() /* ��Ƽ���� ������ ���Ͱ� ��ũ�� �������� ���� */)
		return VHM_ERROR_INVALID_PARAMETER;

	/* �Ű������� ��ȿ���� �˻��Ͽ����Ƿ� �� ���� */
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

	if(!pVHMIOWrapper /* VHMIOWrapper ��ü�� �ùٸ��� ���� */ ||
		!uiSectorCount /* ���� ������ �ùٸ��� ���� */||
		(uiStartingLBA + uiSectorCount - 1) >= pVHMIOWrapper->GetSectorCount() /* ��Ƽ���� ������ ���Ͱ� ��ũ�� �������� ���� */)
		return VHM_ERROR_INVALID_PARAMETER;

	/* �Ű������� ��ȿ���� �˻��Ͽ����Ƿ� �� ���� */
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

	/* �Ű����� �ʱ�ȭ */
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


#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMIOWrapper.h"
#include "VHMDisk.h"
#include "VHMPartition.h"
#include "VHMFilesystem.h"
#include "VHMFilesystemFATStructure.h"
#include "VHMFilesystemFATCommon.h"
#include "VHMFilesystemFAT32.h"

CVHMFilesystem::CVHMFilesystem(void)
{
	m_bVHMIOWrapperValid = FALSE;
	m_pVHMIOWrapper = nullptr;
	m_bMounted = FALSE;
}

CVHMFilesystem::~CVHMFilesystem(void)
{
	CloseFilesystem();
}

int CVHMFilesystem::CreateVHMFilesystemObject(CVHMIOWrapper *pVHMIOWrapper, CVHMFilesystem **ppVHMFilesystem)
{
	/* VHMIOWrapper 유효성 검사 */
	if(VHMIOWrapperIsObjectValid(pVHMIOWrapper) == FALSE)
		return VHM_ERROR_INVALID_PARAMETER;

	CVHMFilesystem *pVHMFilesystem = nullptr;
	int iType;

	/* FAT 파일시스템 검사 -> FAT 타입에 따라 */
	iType = VHMFilesystemDetermineFSTypeFATUniversal(pVHMIOWrapper);
	switch(iType)
	{
	case FILESYSTEM_FAT12:
		//pVHMFilesystem = new CVHMFilesystemFAT12;
		break;
	case FILESYSTEM_FAT16:
		//pVHMFilesystem = new CVHMFilesystemFAT16;
		break;
	case FILESYSTEM_FAT32:
		pVHMFilesystem = new CVHMFilesystemFAT32;
		break;
	}

	/* pVHMFilesystem에 해당하는 파일시스템 마운트 */
	if(pVHMFilesystem == nullptr)
		return VHM_ERROR_DRIVER_NOT_FOUND;

	if (CVHMFilesystem::CreateVHMFilesystemFromExistingObject(pVHMFilesystem, pVHMIOWrapper) != VHM_ERROR_SUCCESS)
	{
		delete pVHMFilesystem;
		return VHM_ERROR_INTERNAL;
	}

	*ppVHMFilesystem = pVHMFilesystem;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystem::CreateVHMFilesystemFromExistingObject(CVHMFilesystem *pVHMFilesystem, CVHMIOWrapper *pVHMIOWrapper)
{
	/* VHMIOWrapper 객체를 설정하고 파일시스템을 마운트 */
	int iSuccess;

	iSuccess = pVHMFilesystem->SetVHMIOWrapper(pVHMIOWrapper);
	if(iSuccess != VHM_ERROR_SUCCESS) goto fail;

	iSuccess = pVHMFilesystem->OpenFilesystem();
	if(iSuccess != VHM_ERROR_SUCCESS) goto fail;

	return VHM_ERROR_SUCCESS;

fail:
	delete pVHMFilesystem;
	return iSuccess;
}

int CVHMFilesystem::DestroyVHMFilesystemObject(CVHMFilesystem *pVHMFilesystem)
{
	if(!pVHMFilesystem)
		return VHM_ERROR_INVALID_PARAMETER;

	pVHMFilesystem->CloseFilesystem();
	delete pVHMFilesystem;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystem::SetVHMIOWrapper(CVHMIOWrapper *pVHMIOWrapper)
{
	if(IsMounted())
		return VHM_ERROR_ALREADY_OPENED;

	/* VHMIOWrapper 객체 유효성 검사 */
	if(!VHMIOWrapperIsObjectValid(pVHMIOWrapper))
		return VHM_ERROR_INVALID_PARAMETER;

	/* 값 대입 */
	m_pVHMIOWrapper = pVHMIOWrapper;
	m_bVHMIOWrapperValid = TRUE;

	return VHM_ERROR_SUCCESS;
}

int CVHMFilesystem::CreateFilesystem(FILESYSTEM_INFORMATION *pFilesystemInformation)
{
	return -1;
}

int CVHMFilesystem::OpenFilesystem()
{
	return -1;
}

int CVHMFilesystem::CloseFilesystem()
{
	return -1;
}

int CVHMFilesystem::CheckFilesystem()
{
	return -1;
}

int CVHMFilesystem::ModifyFilesystemVolumeLabel(WCHAR *wszVolumeLabel, UINT64 uiLength)
{
	return -1;
}

int CVHMFilesystem::ModifyFilesystemSize(UINT64 uiNewSectorCount)
{
	return -1;
}

int CVHMFilesystem::ModifyFilesystemClusterSize(QWORD qwClusterSize)
{
	return -1;
}

int CVHMFilesystem::FileCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle)
{
	return -1;
}

int CVHMFilesystem::FileMove(QWORD qwHandle, const WCHAR *pwNewPath)
{
	return -1;
}

int CVHMFilesystem::FileGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer)
{
	return -1;
}
int CVHMFilesystem::FileSetName(QWORD qwHandle, const WCHAR *pwNewName)
{
	return -1;
}

QWORD CVHMFilesystem::FileGetAttributes(QWORD qwHandle)
{
	return-1;
}

QWORD CVHMFilesystem::FileSetAttributes(QWORD qwHandle, QWORD qwNewAttributes)
{
	return -1;
}

QWORD CVHMFilesystem::FileGetAccessRights(QWORD qwHandle)
{
	return -1;
}
QWORD CVHMFilesystem::FileSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights)
{
	return -1;
}

int CVHMFilesystem::FileGetSize(QWORD qwHandle, UINT64 *puiSize)
{
	return -1;
}

int CVHMFilesystem::FileSetSize(QWORD qwHandle, UINT64 uiNewSize)
{
	return -1;
}

int CVHMFilesystem::FileRead(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, UINT64 uiBufferSize, QWORD *pqwRead)
{
	return -1;
}

int CVHMFilesystem::FileWrite(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, QWORD *pqwWrite)
{
	return -1;
}

int CVHMFilesystem::FileSetPointer(QWORD qwHandle, INT64 iDistanceToMove, UINT64 *puiNewFilePointer, QWORD qwMoveMethod)
{
	return -1;
}

int CVHMFilesystem::FileDelete(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystem::FileClose(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystem::DirCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle)
{
	return -1;
}

int CVHMFilesystem::DirMove(QWORD qwHandle, const WCHAR *pwNewPath)
{
	return -1;
}

int CVHMFilesystem::DirGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer)
{
	return -1;
}

int CVHMFilesystem::DirSetName(QWORD qwHandle, const WCHAR *pwNewName)
{
	return -1;
}

QWORD CVHMFilesystem::DirGetAttributes(QWORD qwHandle)
{
	return -1;
}

QWORD CVHMFilesystem::DirSetAttributes(QWORD qwHandle, QWORD qwNewAttributes)
{
	return -1;
}

QWORD CVHMFilesystem::DirGetAccessRights(QWORD qwHandle)
{
	return -1;
}

QWORD CVHMFilesystem::DirSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights)
{
	return -1;
}


int CVHMFilesystem::DirGetFileCount(QWORD qwHandle, UINT64 *puiFileCount)
{
	return -1;
}

int CVHMFilesystem::DirGetFolderCount(QWORD qwHandle, UINT64 *puiFileCount)
{
	return -1;
}

int CVHMFilesystem::DirRewind(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystem::DirSeek(QWORD qwHandle, QWORD qwPosition)
{
	return -1;
}

QWORD CVHMFilesystem::DirTell(QWORD qwHandle, const FOBJECT_DESCRIPTOR *pFObjectDescriptor)
{
	return FOBJECT_INVALID_POSITION;
}

const FOBJECT_DESCRIPTOR *CVHMFilesystem::DirEnumerateNext(QWORD qwHandle)
{
	return nullptr;
}

int CVHMFilesystem::DirDelete(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystem::DirClose(QWORD qwHandle)
{
	return -1;
}

int CVHMFilesystem::WriteBootCode(const void *pBuffer)
{
	return -1;
}

int CVHMFilesystem::GetVolumeLabel(WCHAR *pwszBuffer, vhmsize_t ccBuffer)
{
	return -1;
}

BOOL CVHMFilesystem::IsVHMIOWrapperValid()
{
	return m_bVHMIOWrapperValid;
}

BOOL CVHMFilesystem::IsMounted()
{
	return m_bMounted;
}

QWORD CVHMFilesystem::GetFilesystemStatus()
{
	return FILESYSTEM_STATUS_INVALID;
}

DWORD CVHMFilesystem::GetFilesystemCharacteristics()
{
	return FILESYSTEM_CHARACTERISTICS_NONE;
}

int CVHMFilesystem::QueryFilesystemInformation(FILESYSTEM_INFORMATION *pFilesystemInformation)
{
	return -1;
}

DWORD CVHMFilesystem::GetFilesystemType()
{
	return FILESYSTEM_INVALID;
}

int CVHMFilesystem::ReadSector(UINT64 nStartSector, UINT32 nSectorCount, void *pBuffer, UINT64 nBufferSize)
{
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	return m_pVHMIOWrapper->ReadSector(nStartSector, nSectorCount, pBuffer, nBufferSize);
}

int CVHMFilesystem::WriteSector(UINT64 nStartSector, UINT32 nSectorCount, void *pBuffer)
{
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	return m_pVHMIOWrapper->WriteSector(nStartSector, nSectorCount, pBuffer);
}

DWORD CVHMFilesystem::GetSectorSize()
{
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	return m_pVHMIOWrapper->GetSectorSize();
}

QWORD CVHMFilesystem::GetPartitionSectorCount()
{
	if(!IsVHMIOWrapperValid())
		return VHM_ERROR_NOT_OPENED;

	return m_pVHMIOWrapper->GetSectorCount();
}
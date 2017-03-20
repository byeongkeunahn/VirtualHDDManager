
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMDisk.h"
#include "VHMDiskFile.h"

CVHMDiskFile::CVHMDiskFile(void)
{

}

CVHMDiskFile::~CVHMDiskFile(void)
{

}

int CVHMDiskFile::CreateDisk(const wchar_t *wszFilePath, DWORD dwSectorSize, QWORD qwSectorCount)
{
	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	if(dwSectorSize < 512 || !IsPowerOfTwo(dwSectorSize))
		return VHM_ERROR_INVALID_PARAMETER;

	// 섹터 수는 0이 아니어야 한다.
	if(qwSectorCount == 0)
		return VHM_ERROR_INVALID_PARAMETER;

	// wszFilePath에 주어진 매개변수로 VHM 파일을 생성한다.
	if(m_fileVHM.Open(wszFilePath, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
	{
		return VHM_ERROR_DISK_IO;
	}

	// 바이트로 나타낸 디스크 크기
	QWORD qwDiskSizeInBytes = dwSectorSize * qwSectorCount;

	// 파일 크기 설정
	m_fileVHM.SetLength(sizeof(VHMHEADER) + qwDiskSizeInBytes);
	if(m_fileVHM.GetLength() != (sizeof(VHMHEADER) + qwDiskSizeInBytes))
	{
		m_fileVHM.Close();
		DeleteFile(wszFilePath);
		return VHM_ERROR_DISK_IO;
	}

	// 파일에 헤더 쓰기
	// 섹터 초기화는 하지 않음
	VHMHEADER vhmheader;
	vhmheader.Magic = VHMHEADER_MAGIC;
	vhmheader.SectorSize = dwSectorSize;
	vhmheader.SectorCount = qwSectorCount;
	memset(vhmheader.Reserved, 0, sizeof(vhmheader.Reserved));
	m_fileVHM.Seek(0, CFile::begin);
	m_fileVHM.Write(&vhmheader, sizeof(vhmheader));

	// 멤버 변수 설정
	m_bOpened = TRUE;
	m_dwSectorSize = dwSectorSize;
	m_qwSectorCount = qwSectorCount;
	m_uiHeaderLength = sizeof(vhmheader);

	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskFile::OpenDisk(const wchar_t *wszFilePath)
{
	if(IsOpened())
		return VHM_ERROR_ALREADY_OPENED;

	// wszFilePath에 주어진 매개변수로 VHM 파일을 연다.
	if (m_fileVHM.Open(wszFilePath, CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
		return VHM_ERROR_DISK_IO;

	// 헤더를 확인한다.
	VHMHEADER vhmheader;
	m_fileVHM.Seek(0, CFile::begin);
	m_fileVHM.Read(&vhmheader, sizeof(vhmheader));

	if(vhmheader.Magic != VHMHEADER_MAGIC || !IsPowerOfTwo(vhmheader.SectorSize) || vhmheader.SectorCount == 0 ||
		m_fileVHM.GetLength() != (sizeof(vhmheader) + vhmheader.SectorSize * vhmheader.SectorCount) ||
		vhmheader.SectorSize < 512)
	{
		m_fileVHM.Close();
		return VHM_ERROR_DISK_IO;
	}

	// 멤버 변수 설정
	m_bOpened = TRUE;
	m_dwSectorSize = vhmheader.SectorSize;
	m_qwSectorCount = vhmheader.SectorCount;
	m_uiHeaderLength = sizeof(vhmheader);
	
	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskFile::OpenDiskRaw(const wchar_t *wszFilePath)
{
	if(IsOpened())
		return -1;

	// wszFilePath에 주어진 매개변수로 VHM 파일을 연다.
	if(m_fileVHM.Open(wszFilePath, CFile::modeReadWrite | CFile::shareExclusive) == FALSE)
		return -1;

	QWORD qwFileSize;
	qwFileSize = m_fileVHM.GetLength();

	if(qwFileSize < 512)
	{
		m_fileVHM.Close();
		return -1;
	}

	// 디스크 정보 설정
	m_bOpened = TRUE;
	m_dwSectorSize = 512;
	m_qwSectorCount = qwFileSize / m_dwSectorSize;
	m_uiHeaderLength = 0;
	
	InitializeVHMIOWrapper();
	InitializeVHMPartition();
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskFile::CloseDisk()
{
	if(!IsOpened())
		return VHM_ERROR_SUCCESS;

	UninitializeVHMPartition();
	UninitializeVHMIOWrapper();

	m_fileVHM.Close();
	m_bOpened = FALSE;

	m_dwSectorSize = 0;
	m_qwSectorCount = 0;
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskFile::ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer)
{
	return ReadSector(uiSectorLBA, 1, pBuffer, szBuffer);
}

int CVHMDiskFile::WriteOneSector(UINT64 uiSectorLBA, void *pBuffer)
{
	return WriteSector(uiSectorLBA, 1, pBuffer);
}

int CVHMDiskFile::ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	UINT64 u64BytesToRead = GetSectorSize() * uiSectorCount;
	if(u64BytesToRead >= TwoExp32)
		return -1;

	UINT32 uBytesToRead = (UINT32) u64BytesToRead;

	if(szBuffer < uBytesToRead)
		return -1;

	UINT64 uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if(uiLastSector >= GetSectorCount())
		return -1;

	m_fileVHM.Seek(m_uiHeaderLength + uiStartingLBA * GetSectorSize(), CFile::begin);
	if(m_fileVHM.Read(pBuffer, uBytesToRead) != uBytesToRead)
	{
		AfxMessageBox(_T("CFile::Read() returned -1"));
		return -1;
	}
	
	return VHM_ERROR_SUCCESS;
}

int CVHMDiskFile::WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer)
{
	if(!IsOpened())
		return VHM_ERROR_NOT_OPENED;

	UINT64 u64BytesToWrite = GetSectorSize() * uiSectorCount;
	if(u64BytesToWrite >= TwoExp32)
		return -1;

	UINT64 uiLastSector = uiStartingLBA + uiSectorCount - 1;
	if(uiLastSector >= GetSectorSize())
		return -1;

	UINT32 uBytesToWrite = (UINT32) u64BytesToWrite;

	m_fileVHM.Seek(m_uiHeaderLength + uiStartingLBA * GetSectorSize(), CFile::begin);
	m_fileVHM.Write(pBuffer, uBytesToWrite);
	
	return VHM_ERROR_SUCCESS;
}

DWORD CVHMDiskFile::GetDiskType()
{
	if(!IsOpened())
		return VIRTUAL_DISK_FORMAT_INVALID;

	return (m_uiHeaderLength == 0) ? VIRTUAL_DISK_FORMAT_RAW : VIRTUAL_DISK_FORMAT_VHM;
}
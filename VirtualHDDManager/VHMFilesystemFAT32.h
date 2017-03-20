
#pragma once

FWDDEF_HANDLE(HHandleID);
struct FAT32_FOBJ_DESC_INTERNAL;

class CVHMFilesystemFAT32 : public CVHMFilesystem
{
public:
	CVHMFilesystemFAT32();
	virtual ~CVHMFilesystemFAT32();
	
	// ���Ͻý��� ����/����/�ݱ�/����
	virtual int CreateFilesystem(FILESYSTEM_INFORMATION *pFilesystemInformation); // �־��� �����κ��� ���Ͻý��� ����
	virtual int OpenFilesystem(); // ���Ͻý��� ����
	virtual int CloseFilesystem(); // ���Ͻý��� �ݱ�
	virtual int CheckFilesystem(); // ���Ͻý��� Ȯ�� (chkdsk, fsck�� ����)

	// ���Ͻý��� ����
	virtual int ModifyFilesystemVolumeLabel(WCHAR *wszVolumeLabel, UINT64 uiLength); // ���� ���̺� ����
	virtual int ModifyFilesystemSize(UINT64 uiNewSectorCount); // ��Ƽ�� ũ�� ����
	virtual int ModifyFilesystemClusterSize(QWORD qwClusterSize); // Ŭ������ ũ�� ����

	// ���� I/O
	virtual int FileCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle); // pwPath ���͸��� pwName�� �̸����� ũ�� 0�� ���� ����
	virtual int FileMove(QWORD qwHandle, const WCHAR *pwNewPath); // ������ �ٸ� ���͸��� �ű�
	virtual int FileGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t szBuffer); // ���� �̸� ���
	virtual int FileSetName(QWORD qwHandle, const WCHAR *pwNewName); // ���� �̸� �ٲٱ�
	virtual QWORD FileGetAttributes(QWORD qwHandle); // ���� �Ӽ� ���
	virtual QWORD FileSetAttributes(QWORD qwHandle, QWORD qwNewAttributes); // ���� �Ӽ� ����
	virtual QWORD FileGetAccessRights(QWORD qwHandle); // ���� �׼��� ���� ���
	virtual QWORD FileSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights); // ���� �׼��� ���� ����
	virtual int FileGetSize(QWORD qwHandle, UINT64 *puiSize); // ���� ũ�� ���
	virtual int FileSetSize(QWORD qwHandle, UINT64 uiNewSize); // ���� ũ�� ���� (���� ũ�⺸�� ������ ���� ������ �߸�)
	virtual int FileRead(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, UINT64 uiBufferSize, QWORD *pqwRead); // �б�
	virtual int FileWrite(QWORD qwHandle, UINT64 uiSize, BYTE *pBuffer, QWORD *pqwWrite); // ����
	virtual int FileSetPointer(QWORD qwHandle, INT64 iDistanceToMove, UINT64 *puiNewFilePointer, QWORD qwMoveMethod); // ���� ������ �ű��
	virtual int FileDelete(QWORD qwHandle); // ���� ���� �� �ڵ� �ı�
	virtual int FileClose(QWORD qwHandle); // ���� �ݱ�

	// ���͸� I/O
	virtual int DirCreate(const WCHAR *pwName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, QWORD *pqwHandle); // pwPath ���͸��� pwName�� �̸����� ���͸� ����
	virtual int DirMove(QWORD qwHandle,  const WCHAR *pwNewPath); // ���͸��� �ٸ� ���͸��� �ű�
	virtual int DirGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t szBuffer); // ���� �̸� ���
	virtual int DirSetName(QWORD qwHandle, const WCHAR *pwNewName); // ���� �̸� �ٲٱ�
	virtual QWORD DirGetAttributes(QWORD qwHandle); // ���� �Ӽ� ���
	virtual QWORD DirSetAttributes(QWORD qwHandle, QWORD qwNewAttributes); // ���� �Ӽ� ����
	virtual QWORD DirGetAccessRights(QWORD qwHandle); // ���� �׼��� ���� ���
	virtual QWORD DirSetAccessRights(QWORD qwHandle, QWORD qwNewAccessRights); // ���� �׼��� ���� ����
	virtual int DirGetFileCount(QWORD qwHandle, UINT64 *puiFileCount); // ���͸��� ���� ���� ���
	virtual int DirGetFolderCount(QWORD qwHandle, UINT64 *puiFileCount); // ���͸��� ���͸� ���� ���
	virtual int DirRewind(QWORD qwHandle); // ���͸� ������ ����
	virtual int DirSeek(QWORD qwHandle, QWORD qwPosition); // ���͸� ������ ����
	virtual QWORD DirTell(QWORD qwHandle, const FOBJECT_DESCRIPTOR *pFObjectDescriptor); // FOBJECT �׸� �����ڿ� ���� ���͸� ��ġ���� ����
	virtual const FOBJECT_DESCRIPTOR *DirEnumerateNext(QWORD qwHandle); // ���͸� �׸� ����
	virtual int DirDelete(QWORD qwHandle); // ���͸� ���� �� �ڵ� �ı�
	virtual int DirClose(QWORD qwHandle); // ���͸� �ݱ�

	// Ư�� I/O
	virtual int WriteBootCode(const void *pBuffer); // ��Ʈ �ڵ� ����
	virtual int GetVolumeLabel(WCHAR *pwszBuffer, vhmsize_t ccBuffer); // ���� ���̺� ���

	// VHMFilesystem ��ü�� ���� ���
	virtual QWORD GetFilesystemStatus(); // ���Ͻý����� ���� ���
	virtual int QueryFilesystemInformation(FILESYSTEM_INFORMATION *pFilesystemInformation); // ���Ͻý��� ���� ��ȸ

	// VHMFilesystem ��ü�� Ư�� ���
	virtual DWORD GetFilesystemType(); // ���Ͻý��� Ÿ�� ���
	virtual DWORD GetFilesystemCharacteristics(); // ���Ͻý��� Ư�� ���

protected:
	/* ���� �Լ� */

	// ���� ���� ���
	DWORD GetSecPerClus();
	DWORD GetClusterSize();
	DWORD GetClusterCount();

	// ��Ÿ �Լ�
	BOOL IsBasicInformationLoaded();

	// ���� �� ���� �ڵ� ����
	int InitializeHandleManagement();	// �ڵ� ���� ��ƾ �ʱ�ȭ
	int UninitializeHandleManagement();	// �ڵ� ���� ��ƾ ��ε�
	BOOL IsHandleManagementInitialized();	// �ڵ� ���� ��ƾ �ʱ�ȭ ���� Ȯ��


	// Ŭ������ �� FAT ���� I/O
	int ReadCluster(DWORD dwStartCluster, DWORD dwClusterCount, BYTE *pBuffer);	// Ŭ������ �б�
	int WriteCluster(DWORD dwStartCluster, DWORD dwClusterCount, BYTE *pBuffer); // Ŭ������ ����
	int FindFreeCluster(DWORD *pdwIndex);	// ��� ������ Ŭ������ ã�� (���� �� 0xFFFFFFFF)
	int AllocateCluster(DWORD dwIndex);	// Ŭ������ �Ҵ� �� EOF ǥ�� (��� ������ ��쿡�� ����)
	int FreeCluster(DWORD dwIndex);	// Ŭ������ �Ҵ� ���� (��� ���� ������ ����)
	int LinkCluster(DWORD dwFirst, DWORD dwLast);	// dwFirst->dwLast Ŭ������ ����
	int UnlinkCluster(DWORD dwIndex);	// Ŭ������ ���� ���� (EOF ǥ��)
	int MarkAsBadCluster(DWORD dwIndex);	// ��� Ŭ�����ͷ� ǥ��
	int ReadFAT(DWORD dwIndex, DWORD *pdwValue);	// FAT ������ ���
	int WriteFAT(DWORD dwIndex, DWORD dwValue);	// FAT ������ ���� (FAT �̷���/FAT Ȱ�� ��� ���)
	int WriteFAT(DWORD dwFATIndex, DWORD dwIndex, DWORD dwValue); // ������ FAT�� ������ ���
	int ClusterToFATLBA(DWORD dwClusterIndex, QWORD *pqwFATLBA, DWORD *pdwOffsetInSector);	// Ŭ������ �ε����� FAT�� �� ��° ���Ϳ� ����Ǿ� �ִ°��� ��ȯ
	int ClusterIndexToLBA(DWORD dwClusterIndex, QWORD *pqwLBA);	// Ŭ������ �ε����� LBA�� ��ȯ
	int MarkHardwareError(); // �ϵ���� ���� �߻��� ǥ��

	// ���� �Ӽ� ��ȯ (VHM ǥ�� <--> FAT32)
	int AttrConvFAT32ToVhm(BYTE AttrFAT32, QWORD *pAttrVHM);
	int AttrConvVhmToFAT32(QWORD AttrVHM, BYTE *pAttrFAT32);

	// ���͸� ��Ʈ�� ó��
	int GetDirItemInit(void *pDirEntBuf, QWORD qwDirEntBufSize, QWORD *pqwTempVal); // ���͸� �׸� - �ʱ�ȭ
	int GetNextDirItem(void *pDirEntBuf, QWORD qwDirEntBufSize, FAT32_FOBJ_DESC_INTERNAL **ppFObject, QWORD *pqwTempVal); // ���͸� �׸� - ���� �׸� ��� (*ppFObject�� FreeMemory�� �����Ǿ�� ��)
	int GetNextValidDirEntry(void *pDirEntBuf, QWORD qwDirEntBufSize, QWORD *pqwCurPos, void **ppDirEnt);
	int NameLen83(void *pDirEnt, QWORD *pqwLen); // ���͸� ��Ʈ�� �׸��� 8.3 �̸��� ���� ���
	int Name83ToVhm(void *pDirEnt, QWORD qwAttributes, WCHAR *pwOutBuf); // 8.3 �̸��� VHM ǥ������ ��ȯ
	int NameLenLfn(void *pDirEnt, QWORD *pqwLen); // ���͸� ��Ʈ�� �׸��� LFN �̸��� ���� ���
	int LoadVolumeLabel(); // ���� ���̺� �ε�

	// ���� I/O
	
	// ���͸� I/O

protected:
	/* m_pLBA0, m_pFSInfo�� ���� ��ü�� ����Ǿ� �����Ƿ� GetSectorSize()�� ���� ���� ũ���� ������ */
	BYTE				*m_pLBA0;	// LBA 0
	FAT_BPB_BS_COMMON	*m_pFATCommonHeader;	// FAT ���� ���
	FAT32_BPB_BS		*m_pFAT32Header;	// FAT32 ���
	BYTE				*m_pFSInfo;	// FSINFO ����

	/* ���� ���� */
	QWORD				m_qwVolumeAttributes;	// ���� ����
	QWORD				m_qwReservedSectorCount;	// ����� ���� ��
	QWORD				m_qwFATAreaStartSector;	// FAT ���� ���� ���� LBA
	QWORD				m_qwFATAreaSectorCount;	// FAT ���� ���� ��
	DWORD				m_dwFATCount;	// FAT�� ��
	DWORD				m_dwActiveFAT;	// Ȱ�� FAT ��ȣ
	QWORD				m_qwDataAreaStartSector;	// ������ ���� ���� ���� LBA
	QWORD				m_qwDataAreaSectorCount;	// ������ ���� ���� ��
	DWORD				m_dwClusterSize;	// Ŭ������ ũ��
	DWORD				m_dwClusterCount;	// Ŭ������ ��
	WCHAR				m_pwszVolumeLabel[12];	// ���� ���̺�

	/* ���� ���� */
	BOOL				m_bBasicInformationLoaded;	// �ڵ� ���� ��ƾ ���� �ʱ�ȭ�� �Ϸ� ���� (= �⺻ ���� �ε� ����)
	BOOL				m_bHandleManagementInitialized;	// �ڵ� ���� ��ƾ �ʱ�ȭ ����
	HHandleID			m_hHandleID;	// �ڵ� ID
};

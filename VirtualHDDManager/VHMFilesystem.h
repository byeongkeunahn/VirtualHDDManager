
#pragma once

class CVHMIOWrapper;

class CVHMFilesystem
{
public:
	CVHMFilesystem(void);
	virtual ~CVHMFilesystem(void);

	// �־��� ��Ƽ���� ���Ͻý��� Ÿ���� �Ǻ��Ͽ� VHMFilesystem ��ü�� ����
	static int CreateVHMFilesystemObject(CVHMIOWrapper *pVHMIOWrapper, CVHMFilesystem **ppVHMFilesystem);
protected:
	inline static int CreateVHMFilesystemFromExistingObject(CVHMFilesystem *pVHMFilesystem, CVHMIOWrapper *pVHMIOWrapper);

public:

	// VHMFilesystem ��ü �ı�
	static int DestroyVHMFilesystemObject(CVHMFilesystem *pVHMFilesystem);

	// VHMIOWrapper ��ü ����
	virtual int SetVHMIOWrapper(CVHMIOWrapper *pVHMIOWrapper);

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
	virtual int FileGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer); // ���� �̸� ���
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
	virtual int DirGetName(QWORD qwHandle, WCHAR *pwBuffer, vhmsize_t ccBuffer); // ���� �̸� ���
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
	virtual BOOL IsVHMIOWrapperValid(); // ������ VHMIOWrapper ��ü�� �ùٸ��� TRUE
	virtual BOOL IsMounted(); // ��Ƽ���� ����Ʈ�Ǿ����� TRUE
	virtual QWORD GetFilesystemStatus(); // ���Ͻý����� ���� ���
	virtual int QueryFilesystemInformation(FILESYSTEM_INFORMATION *pFilesystemInformation); // ���Ͻý��� ���� ��ȸ

	// VHMFilesystem ��ü�� Ư�� ���
	virtual DWORD GetFilesystemType(); // ���Ͻý��� Ÿ�� ���
	virtual DWORD GetFilesystemCharacteristics(); // ���Ͻý��� Ư�� ���

protected:
	virtual int ReadSector(UINT64 nStartSector, UINT32 nSectorCount, void *pBuffer, UINT64 nBufferSize); // ���� �б� (LBA - nStartSector, ���� �� - nSectorCount)
	virtual int WriteSector(UINT64 nStartSector, UINT32 nSectorCount, void *pBuffer); // ���� ����(LBA - nStartSector, ���� �� - nSectorCount)
	virtual DWORD GetSectorSize(); // ��ũ�� ���� ũ�� ���
	virtual QWORD GetPartitionSectorCount(); // ��Ƽ���� ���� ���� ���

protected:
	BOOL			m_bVHMIOWrapperValid; // VHMIOWrapper ��ü�� �ùٸ��� TRUE
	CVHMIOWrapper	*m_pVHMIOWrapper; // VHMIOWrapper ��ü
	BOOL			m_bMounted; // ���Ͻý����� ����Ʈ�Ǿ����� TRUE
};

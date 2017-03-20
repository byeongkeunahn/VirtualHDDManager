
#pragma once

class CVHMIOWrapper;
class CVHMPartition;

class CVHMDisk
{
public:
	CVHMDisk(void);
	virtual ~CVHMDisk(void);

	virtual int OpenDisk(); // ���� ��ũ ����
	virtual int CloseDisk(); // ��ũ �ݱ�

	virtual int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer); // ���� 1�� �б�
	virtual int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer); // ���� 1�� ����

	virtual int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer); // ���� �б� (LBA - nStartSector, ���� �� - nSectorCount)
	virtual int WriteSector(UINT64 uiStartSector, UINT64 uiSectorCount, void *pBuffer); // ���� ����(LBA - nStartSector, ���� �� - nSectorCount)

	virtual int WipeSector(UINT64 uiStartSector, UINT64 uiSectorCount); // �־��� ���͵��� 0���� ä��� (LBA - nStartSector, ���� �� - nSectorCount)
	virtual int WipeDisk(); // ��ũ ��ü�� 0���� ä���

	virtual BOOL IsOpened(); // ��ũ�� �������� TRUE
	virtual DWORD GetSectorSize(); // ��ũ�� ���� ũ�� ���ϱ�
	virtual QWORD GetSectorCount(); // ��ũ�� ���� �� ���ϱ�

	virtual DWORD GetPartitioningType(); // ��Ƽ�Ŵ� Ÿ�� ���
	virtual UINT64 GetPartitionCount(); // ��Ƽ�� ���� ���

	virtual QWORD GetFirstPartitionID(); // ó�� ��Ƽ�� ID ���
	virtual QWORD GetPrevPartitionID(QWORD qwPartitionID); // ���� ��Ƽ�� ID ���
	virtual QWORD GetNextPartitionID(QWORD qwPartitionID); // ���� ��Ƽ�� ID ���
	virtual QWORD GetLastPartitionID(); // ������ ��Ƽ�� ID ���

	virtual DWORD GetDiskType(); // ��ũ Ÿ�� ���

	virtual CVHMIOWrapper *GetVHMIOWrapper(); // VHMIOWrapper ��ü ���
	virtual CVHMPartition *GetVHMPartition(); // VHMPartition ��ü ���

protected:
	virtual int InitializeVHMIOWrapper(); // VHMIOWrapper ��ü �ʱ�ȭ
	virtual int UninitializeVHMIOWrapper(); // VHMIOWrapper ��ü ��ε�

	virtual int InitializeVHMPartition(); // VHMPartition ��ü �ʱ�ȭ
	virtual int UninitializeVHMPartition(); // VHMPartition ��ü ��ε�

protected:
	BOOL		m_bOpened;
	DWORD		m_dwSectorSize;
	QWORD		m_qwSectorCount;
	CVHMPartition	*m_pVHMPartition;
	CVHMIOWrapper	*m_pVHMIOWrapper;
};


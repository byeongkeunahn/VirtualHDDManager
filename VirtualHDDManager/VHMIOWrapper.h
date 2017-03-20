
#pragma once

class CVHMDisk;
class CVHMPartition;

class CVHMIOWrapper
{
public:
	CVHMIOWrapper();
	virtual ~CVHMIOWrapper();

	BOOL IsInitialized();

	int Initialize(CVHMDisk *pVHMDisk, UINT64 uiStartingLBA, UINT64 uiSectorCount);
	int Initialize(CVHMIOWrapper *pVHMIOWrapper, UINT64 uiStartingLBA, UINT64 uiSectorCount);
	int Initialize(CVHMIOWrapper *pVHMIOWrapper, DWORD dwSectorSize, UINT64 uiStartingLBA, UINT64 uiSectorCount);
	int Uninitialize();

	int ReadOneSector(UINT64 uiSectorLBA, void *pBuffer, UINT64 szBuffer); // ���� 1�� �б�
	int WriteOneSector(UINT64 uiSectorLBA, void *pBuffer); // ���� 1�� ����

	int ReadSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer, UINT64 szBuffer); // ���� �б� (LBA - nStartSector, ���� �� - nSectorCount)
	int WriteSector(UINT64 uiStartingLBA, UINT64 uiSectorCount, void *pBuffer); // ���� ����(LBA - nStartSector, ���� �� - nSectorCount)

	DWORD GetSectorSize(); // ���� ũ�� ���
	QWORD GetSectorCount(); // ���� �� ���

protected:
	BOOL			m_bInitialized; // �� CVHMIOWrapper ��ü�� �ùٸ� ������ �� �ʱ�ȭ�Ǿ� ������ TRUE
	int				m_iIOObjectType; // I/O ��ü�� ����
	POINTER			m_pIOObject; // I/O ��ü�� ������
	UINT64			m_uiStartingLBA; // ��Ƽ���� ���� LBA �ּ�
	UINT64			m_uiSectorCount; // ��Ƽ���� ���� ����
	DWORD			m_dwSectorSize; // ��Ƽ���� ���� ũ��
};

BOOL VHMIOWrapperIsObjectValid(CVHMIOWrapper *pVHMIOWrapper); // pVHMIOWrapper�� �ùٸ� ��ü�̸� TRUE

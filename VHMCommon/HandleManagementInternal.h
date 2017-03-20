
#pragma once

/*

�ڵ� ���� - ���� ���� �� �Լ�

*/

// �ڵ� ũ�� -> 8����Ʈ
#define HANDLE_SIZE 8

typedef struct _handle_management
{
	vhmsize_t	HandleBlockSize; // �ڵ� ��� ũ��
	QWORD	HandleCount; // �ڵ� ����
	QWORD	AllocatedBlockCount; // �޸𸮿� ������ �Ҵ�� �ڵ� ��� ���� (�ӵ� ����� ���� �����)
	QWORD	BlockAllocationUnit; // �ڵ� ������� �� �޸𸮸� �Ҵ��� �� ���� ����
	QWORD	LastAllocatedHandleValue; // ���� ���������� �Ҵ�� �ڵ� ��
	BYTE	*HandlePointer; // HandlePointer�� �޸� ũ��� HandleCount * (HandleBlockSize + HANDLE_SIZE)

}HANDLE_ID;

POINTER HandleMemAlloc(vhmsize_t szMem); // �޸� �Ҵ�
POINTER HandleMemRealloc(POINTER ptrMemory, vhmsize_t szMem); // �޸� ���Ҵ�
POINTER HandleMemCopy(POINTER ptrDest, POINTER ptrSrc, vhmsize_t szCopy); // �޸� ���� (ptrSrc -> ptrDest)
void HandleMemZero(POINTER ptrMemory, QWORD szZero); // �޸𸮸� 0���� ä��
void HandleMemFree(POINTER ptrMemory); // �޸� �Ҵ� ����

QWORD HandleValueToIndex(HHandleID hHandleID, QWORD qwHandle); // qwHandle�� �ε��� ��� (���� �߻� �� -1 ����)
BYTE *HandleIndexToPointer(HHandleID hHandleID, QWORD qwIndex); // qwIndex�� ������ ��� (���� �߻� �� nullptr ����)
BYTE *HandleGetPointerFromHandle(HHandleID hHandleID, QWORD qwHandle); // qwHandle�� �ش��ϴ� ����� ���� ��ġ ������ ���

int HandleChangeAllocatedBlockCount(HHandleID hHandleID, QWORD qwNewBlockCount); // �̸� �Ҵ�� ����� ���� qwNewBlockCount�� ���� (��, qwNewBlockCount�� HandleCount���� ���� �� ����)
int HandleAllocateOneBlockUnit(HHandleID hHandleID); // BlockAllocationUnit ������ �ø��Ͽ� �޸� �Ҵ�
int HandleFreeOneBlockUnit(HHandleID hHandleID); // BlockAllocationUnit ������ �����Ͽ� �޸� �Ҵ� ����
QWORD HandleGetUnusedBlockCount(HHandleID hHandleID); // �̸� �Ҵ�� ����� �� �������� �ڵ� �뵵�� �Ҵ�� ����� ������ �� ���� ����� ������ ����

QWORD HandleGenerateValue(HHandleID hHandleID); // hHandleID�� ���Ե��� ���� �ڵ��� �����Ͽ� ����
QWORD HandleGetValue(HHandleID hHandleID, QWORD qwIndex); // HandlePointer�� qwIndex��° ����� �ڵ� ���� ����
int HandleSetValue(HHandleID hHandleID, QWORD qwIndex, QWORD qwHandleValue); // HandlePointer�� qwIndex��° ����� �ڵ� ���� qwHandle�� ����

QWORD HandleGetLastAllocatedHandleValue(HHandleID hHandleID); // ���� ���������� �Ҵ�� �ڵ� ���� ����
int HandleSetLastAllocatedHandleValue(HHandleID hHandleID, QWORD qwValue); // ���� ���������� �Ҵ�� �ڵ� ���� ����

BOOL HandleIsHandleIDValid(HHandleID hHandleID); // hHandleID�� ��ȿ�� �ڵ� ID���� �˻�
BOOL HandleIsHandleValid(HHandleID hHandleID, QWORD qwHandle); // qwHandle�� hHandleID�� ��ȿ�� �ڵ� ID���� �˻�

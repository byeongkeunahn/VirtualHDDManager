
#include "stdafx.h"
#include "VHMBase.h"
#include "VHMUtility.h"
#include "HandleManagement.h"
#include "HandleManagementInternal.h"

POINTER HandleMemAlloc(vhmsize_t szMem)
{
	return (POINTER)malloc(szMem);
}

POINTER HandleMemRealloc(POINTER ptrMemory, vhmsize_t szMem)
{
	return (POINTER)realloc(ptrMemory, szMem);
}

POINTER HandleMemCopy(POINTER ptrDest, POINTER ptrSrc, vhmsize_t szCopy)
{
	CopyMemoryBlock(ptrDest, ptrSrc, szCopy);
	return ptrDest;
}

void HandleMemZero(POINTER ptrMemory, QWORD szZero)
{
	QWORD qwTemp = szZero / sizeof(QWORD);
	QWORD i;

	QWORD *pqwMemory = (QWORD *)ptrMemory;

	for (i = qwTemp; i != 0; i--)
		*pqwMemory++ = 0;

	BYTE *pMemory = (BYTE *)pqwMemory;
	qwTemp = szZero % sizeof(QWORD);
	for (i = 0; i < qwTemp; i++)
		*pMemory++ = 0;
}

void HandleMemFree(POINTER ptrMemory)
{
	free(ptrMemory);
}

QWORD HandleValueToIndex(HHandleID hHandleID, QWORD qwHandle)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	QWORD i;

	/* �ڵ� ������ŭ ������ ���鼭 �ش� �ڵ� ���� ã�� */
	for (i = 0; i < HandleGetHandleCount(hHandleID); i++)
	{
		if (qwHandle == HandleGetValue(hHandleID, i))
			return i;
	}

	return (QWORD)-1;
}

BYTE *HandleIndexToPointer(HHandleID hHandleID, QWORD qwIndex)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return nullptr;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* qwIndex�� �ڵ� ������ ������ �� �� */
	QWORD qwHandleCount = HandleGetHandleCount(hHandleID);
	if (qwIndex >= qwHandleCount)
		return nullptr;

	/* �ڵ� 1���� ��ü ũ�� = (�ڵ� ��� ũ��) + (�ڵ� ���� ũ��) */
	vhmsize_t szHandleBlockTotalSize = pHandleID->HandleBlockSize + HANDLE_SIZE;

	/* (qwIndex��°�� ������) =  (�ڵ� ��ü ũ��) * (qwIndex) + (�ڵ� ���� ũ��) */
	BYTE *pPointer;
	pPointer = pHandleID->HandlePointer;
	pPointer += szHandleBlockTotalSize * qwIndex;
	pPointer += HANDLE_SIZE;

	return pPointer;
}

BYTE *HandleGetPointerFromHandle(HHandleID hHandleID, QWORD qwHandle)
{
	if (!HandleIsHandleValid(hHandleID, qwHandle))
		return nullptr;

	/* �ڵ� ���� �ε����� ��ȯ�ϰ�, �� �ε����� �����ͷ� ��ȯ */
	QWORD qwIndex = HandleValueToIndex(hHandleID, qwHandle);
	if (qwIndex == (QWORD)-1)
		return nullptr;

	return HandleIndexToPointer(hHandleID, qwIndex);
}

int HandleChangeAllocatedBlockCount(HHandleID hHandleID, QWORD qwNewBlockCount)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* ��û�� ���ο� ��� ���� ���� �Ҵ�� �ڵ� ������ �۾Ƽ��� �� �� */
	if (pHandleID->HandleCount > qwNewBlockCount)
		return -1;

	/* ��� ��Ͽ� ���� �Ҵ� ������ ��û�Ǿ����� HandleMemFree ȣ�� */
	if (qwNewBlockCount == 0)
	{
		HandleMemFree(pHandleID->HandlePointer);
		pHandleID->HandlePointer = nullptr;
		pHandleID->AllocatedBlockCount = 0;
		return 0;
	}

	/* HandleMemRealloc �Լ��� �̿��Ͽ� �޸� ���Ҵ� */
	QWORD szNewMem = (pHandleID->HandleBlockSize + HANDLE_SIZE) * qwNewBlockCount;
	POINTER ptrNew = (POINTER)HandleMemRealloc(pHandleID->HandlePointer, szNewMem);
	if (!ptrNew)
		return -1; /* HandleMemRealloc ���� */

	pHandleID->HandlePointer = (BYTE *)ptrNew;
	pHandleID->AllocatedBlockCount = qwNewBlockCount;
	return 0;
}

int HandleAllocateOneBlockUnit(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* ���� �Ҵ�� ����� �����Ͽ� �ּ� 1������ �ִ� BlockAllocationUnit����ŭ �Ҵ��Ͽ� BlockAllocationUnit�� ���� */
	QWORD qwNewBlockCount;
	qwNewBlockCount = pHandleID->AllocatedBlockCount;
	qwNewBlockCount = RoundDown(qwNewBlockCount, pHandleID->BlockAllocationUnit);
	qwNewBlockCount += pHandleID->BlockAllocationUnit;

	return HandleChangeAllocatedBlockCount(hHandleID, qwNewBlockCount);
}

int HandleFreeOneBlockUnit(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	if (pHandleID->AllocatedBlockCount == 0)
		return -1;

	/* ���� �Ҵ�� ����� �����Ͽ� �ּ� 1������ �ִ� BlockAllocationUnit����ŭ �Ҵ� �����Ͽ� BlockAllocationUnit�� ���� */
	QWORD qwNewBlockCount;
	qwNewBlockCount = pHandleID->AllocatedBlockCount;
	qwNewBlockCount = RoundUp(qwNewBlockCount, pHandleID->BlockAllocationUnit);
	qwNewBlockCount -= pHandleID->BlockAllocationUnit;

	return HandleChangeAllocatedBlockCount(hHandleID, qwNewBlockCount);
}

QWORD HandleGetUnusedBlockCount(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* ���� ��� ���� = ��ü �Ҵ�� ����� ���� - �ڵ�� �Ҵ�� ��� ���� */
	QWORD qwUnusedBlockCount;
	qwUnusedBlockCount = pHandleID->AllocatedBlockCount;
	qwUnusedBlockCount -= pHandleID->HandleCount;

	return qwUnusedBlockCount;
}

QWORD HandleGenerateValue(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	BOOL bAvailableHandleFound = FALSE;
	QWORD qwValue = 0;
	QWORD i;

	/* qwValue�� 1���� 1�� ������Ű�鼭 ������ ���� �ڵ� ���� ã�� */
	for (i = 0; i < 0x8000000000000000UL; i++)
	{
		qwValue++;
		if (HandleValueToIndex(hHandleID, qwValue) == ((QWORD)-1))
		{
			bAvailableHandleFound = TRUE;
			break;
		}
	}

	if (bAvailableHandleFound == FALSE)
		return (QWORD)-1;

	return qwValue;
}

QWORD HandleGetValue(HHandleID hHandleID, QWORD qwIndex)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	if (qwIndex >= pHandleID->HandleCount)
		return (QWORD)-1;

	/* qwIndex��°�� �ڵ� �� ��ġ�� �� �ڵ� ���� ��ȯ */
	QWORD szOffset;
	szOffset = HandleGetBlockSize(hHandleID) + HANDLE_SIZE;
	szOffset *= qwIndex;
	BYTE *pPointer = (BYTE *)pHandleID->HandlePointer;
	pPointer += szOffset;

	QWORD *pqwPointer = (QWORD *)pPointer;

	return (*pqwPointer);
}

int HandleSetValue(HHandleID hHandleID, QWORD qwIndex, QWORD qwHandleValue)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	if (qwIndex >= pHandleID->AllocatedBlockCount)
		return -1;

	/* qwIndex��°�� �ڵ� �� ��ġ�� �� qwHandleValue�� ���� */
	QWORD szOffset;
	szOffset = HandleGetBlockSize(hHandleID) + HANDLE_SIZE;
	szOffset *= qwIndex;
	BYTE *pPointer = (BYTE *)pHandleID->HandlePointer;
	pPointer += szOffset;

	QWORD *pqwPointer = (QWORD *)pPointer;
	*pqwPointer = qwHandleValue;

	return 0;
}

QWORD HandleGetLastAllocatedHandleValue(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* �ڵ� ����ü���� �ش� ���� ����*/
	return pHandleID->LastAllocatedHandleValue;
}

int HandleSetLastAllocatedHandleValue(HHandleID hHandleID, QWORD qwValue)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* �ڵ� ����ü�� �ش� ���� ���� */
	pHandleID->LastAllocatedHandleValue = qwValue;
	return 0;
}

BOOL HandleIsHandleIDValid(HHandleID hHandleID)
{
	if (!hHandleID)
		return FALSE;

	return TRUE;
}

BOOL HandleIsHandleValid(HHandleID hHandleID, QWORD qwHandle)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return FALSE;

	/* �ùٸ� �ڵ��̶�� qwHandle�� �����ϴ� �ùٸ� �ڵ� �ε����� �����ؾ� �� */
	if (HandleValueToIndex(hHandleID, qwHandle) == ((QWORD)-1))
		return FALSE;

	return TRUE;
}

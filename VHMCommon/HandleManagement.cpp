
#include "stdafx.h"
#include "VHMBase.h"
#include "HandleManagement.h"
#include "HandleManagementInternal.h"

HHandleID HandleIDCreate(vhmsize_t szBlock, QWORD qwAllocationUnit)
{
	if (szBlock <= 0 || qwAllocationUnit <= 0)
		return nullptr;

	/* �ڵ� ID ���� ���� ���� �Ҵ� */
	HANDLE_ID *pHandleID = (HANDLE_ID *)HandleMemAlloc(sizeof(HANDLE_ID));
	if (pHandleID == nullptr)
		return nullptr;

	/* �ڵ� ID�� �⺻���� ���� ���� */
	pHandleID->HandleBlockSize = szBlock;
	pHandleID->HandleCount = 0;
	pHandleID->AllocatedBlockCount = 0;
	pHandleID->BlockAllocationUnit = qwAllocationUnit;
	pHandleID->LastAllocatedHandleValue = (QWORD)-1;
	pHandleID->HandlePointer = nullptr;

	return (HHandleID)pHandleID;
}

int HandleIDDestroy(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	/* ��� �ڵ��� �ı� */
	if (HandleRemoveAll(hHandleID) != 0)
		return -1;

	/* �ڵ� ID ���� ���� ���� �޸� ���� */
	HandleMemFree(hHandleID);

	return 0;
}

QWORD HandleAdd(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* �ڵ�� �Ҵ���� ���� ��� ���� �����ϰ�, ������ �Ҵ��� ��û  */
	if (HandleGetUnusedBlockCount(hHandleID) == 0)
	{
		if (HandleAllocateOneBlockUnit(hHandleID) == -1)
			return (QWORD)-1;
	}

	/* HandleGetUnusedBlockCount �Լ��� �Ǵٽ� 0�� ��ȯ�ϸ� ���� */
	if (HandleGetUnusedBlockCount(hHandleID) == 0)
		return (QWORD)-1;

	/* �ڵ� ���� ���� �� �ڵ� 1���� �Ҵ���� ���� ��Ͽ��� �Ҵ� */
	QWORD qwHandle = HandleGenerateValue(hHandleID);
	if (qwHandle == (QWORD)-1)
		return -1;

	/* ���ο� �ڵ��� ���� �����ϰ� �ڵ� ī��Ʈ�� 1��ŭ ������Ŵ */
	QWORD qwIndex = pHandleID->HandleCount;
	HandleSetValue(hHandleID, qwIndex, qwHandle);
	pHandleID->HandleCount++;

	return qwHandle;
}

vhmsize_t HandleGetBlockSize(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (vhmsize_t)-1;

	/* �ڵ� ��� ũ�⸦ ���� */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	return pHandleID->HandleBlockSize;
}

QWORD HandleGetHandleCount(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (vhmsize_t)-1;

	/* �ڵ� ������ ���� */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	return pHandleID->HandleCount;
}

QWORD HandleSetAllocationUnit(HHandleID hHandleID, QWORD qwNewAllocationUnit)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	if (qwNewAllocationUnit <= 0)
		return (QWORD)-1;

	/* �ڵ� ��� �޸� �Ҵ� ���� ũ�⸦ ���� */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	QWORD qwOldAllocationUnit = pHandleID->BlockAllocationUnit;
	pHandleID->BlockAllocationUnit = qwNewAllocationUnit;

	return qwOldAllocationUnit;
}

int HandleRemove(HHandleID hHandleID, QWORD qwHandle)
{
	if (!HandleIsHandleValid(hHandleID, qwHandle))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* �ڵ��� �ε����� ��ȯ */
	QWORD qwIndex = HandleValueToIndex(hHandleID, qwHandle);
	if (qwIndex == (QWORD)-1)
		return -1;

	/* �ε����� �����ͷ� ��ȯ */
	vhmsize_t szRealBlock = pHandleID->HandleBlockSize + HANDLE_SIZE;
	BYTE *pPointer = pHandleID->HandlePointer;
	pPointer += szRealBlock * qwIndex;

	/* ���� ������ ����̶�� �ڵ� ����� 0���� ä�� */
	if (qwIndex == (pHandleID->HandleCount - 1))
		HandleMemZero((POINTER)pPointer, szRealBlock);
	else
	{
		/* ���� ������ ����� �����͸� �� ������� ������ �� ������ �ڵ� ����� 0���� ä�� */
		BYTE *pLastBlockPtr = pHandleID->HandlePointer + (szRealBlock * (pHandleID->HandleCount - 1));
		HandleMemCopy((POINTER)pPointer, (POINTER)pLastBlockPtr, szRealBlock);
		HandleMemZero((POINTER)pLastBlockPtr, szRealBlock);
	}

	/* �ڵ� ī��Ʈ 1�� ���� */
	pHandleID->HandleCount--;

	/* �̻�� ����� �Ҵ� ���� ũ�� �̻��̸� �Ҵ� ���� */
	QWORD qwUnusedBlockCount = HandleGetUnusedBlockCount(hHandleID);
	if (qwUnusedBlockCount >= pHandleID->BlockAllocationUnit)
	{
		HandleFreeOneBlockUnit(hHandleID);
	}

	return 0;
}

int HandleRemoveAll(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* �ڵ� �޸� �Ҵ� ���� �� �� �ʱ�ȭ */
	if (pHandleID->HandlePointer != nullptr)
		HandleMemFree(pHandleID->HandlePointer);

	pHandleID->HandleCount = 0;
	pHandleID->AllocatedBlockCount = 0;
	pHandleID->LastAllocatedHandleValue = (QWORD)-1;
	pHandleID->HandlePointer = nullptr;

	return 0;
}

QWORD HandleGetFirstHandle(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	/* �ڵ��� ������ 0���� Ŀ�� �� */
	if (HandleGetHandleCount(hHandleID) == 0)
		return (QWORD)-1;

	/* �ڵ� ���� �޸��� ���� �պκп� �ִ� �ڵ� ���� */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	BYTE *pPointer = pHandleID->HandlePointer;
	QWORD qwHandle = *((QWORD *)pPointer);

	return qwHandle;
}

QWORD HandleGetNextHandle(HHandleID hHandleID, QWORD qwHandle)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	/* �ڵ��� ������ 0���� Ŀ�� �� */
	if (HandleGetHandleCount(hHandleID) == 0)
		return (QWORD)-1;

	/* �ڵ��� �ε����� ��ȯ */
	QWORD qwIndex;
	if ((qwIndex = HandleValueToIndex(hHandleID, qwHandle)) == (QWORD)-1)
		return (QWORD)-1;

	/* qwIndex + 1 ��°�� �ڵ� ���� */
	return HandleGetValue(hHandleID, qwIndex + 1);
}

QWORD HandleGetLastHandle(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	/* �ڵ��� ������ 0���� Ŀ�� �� */
	if (HandleGetHandleCount(hHandleID) == 0)
		return (QWORD)-1;

	/* �ڵ� ���� �޸��� ���� �޺κп� �ִ� �ڵ� ���� */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	BYTE *pPointer = pHandleID->HandlePointer;
	pPointer += (HandleGetHandleCount(hHandleID) - 1) * (HandleGetBlockSize(hHandleID) + sizeof(QWORD));
	QWORD qwHandle = *((QWORD *)pPointer);

	return qwHandle;
}

int HandleGetData(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, vhmsize_t szData, vhmsize_t szBuffer, void *pBuffer)
{
	if (!HandleIsHandleValid(hHandleID, qwHandle))
		return -1;

	/* ����� �ϴ� �������� ũ�Ⱑ ������ ũ�⺸�� Ŭ �� ���� */
	if (szData > szBuffer)
		return -1;

	/* ����� �ϴ� �����Ͱ� �ش� �ڵ鿡 �Ҵ�� ������ ���� �� ���� */
	QWORD qwBlockSize = HandleGetBlockSize(hHandleID);
	if ((uiOffset + szData) > qwBlockSize)
		return -1;

	/* �ڵ��� �����Ͱ� ����� ������ �����͸� ��� ��û�� ���������� ������ ���� */
	BYTE *pPointer;
	pPointer = HandleGetPointerFromHandle(hHandleID, qwHandle);
	if (!pPointer)
		return -1;

	pPointer += uiOffset;

	BYTE *pByteBuffer = (BYTE *)pBuffer;

	/* szData��ŭ �����͸� �ű� */
	while (szData != 0)
	{
		*pByteBuffer = *pPointer;
		*pByteBuffer++;
		*pPointer++;
		szData--;
	}

	return 0;
}

int HandleSetData(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, vhmsize_t szData, vhmsize_t szBuffer, const void *pBuffer)
{
	if (!HandleIsHandleValid(hHandleID, qwHandle))
		return -1;

	/* ������ �ϴ� �������� ũ�Ⱑ ������ ũ�⺸�� Ŭ �� ���� */
	if (szData > szBuffer)
		return -1;

	/* ������ �ϴ� �����Ͱ� �ش� �ڵ鿡 �Ҵ�� ������ ���� �� ���� */
	QWORD qwBlockSize = HandleGetBlockSize(hHandleID);
	if ((uiOffset + szData) > qwBlockSize)
		return -1;

	/* �ڵ��� �����Ͱ� ����� ������ �����͸� ��� ��û�� ���������� ������ ���� */
	BYTE *pPointer;
	pPointer = HandleGetPointerFromHandle(hHandleID, qwHandle);
	if (!pPointer)
		return -1;

	pPointer += uiOffset;

	BYTE *pByteBuffer = (BYTE *)pBuffer;

	/* szData��ŭ �����͸� �ű� */
	while (szData != 0)
	{
		*pPointer = *pByteBuffer;
		*pByteBuffer++;
		*pPointer++;
		szData--;
	}

	return 0;
}

int HandleGetDataByte(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, BYTE *Output)
{
	return HandleGetData(hHandleID, qwHandle, uiOffset, 1, 1, (BYTE *)Output);
}

int HandleSetDataByte(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, BYTE NewValue)
{
	return HandleSetData(hHandleID, qwHandle, uiOffset, 1, 1, (BYTE *)(&NewValue));
}

int HandleGetDataWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, WORD *Output)
{
	return HandleGetData(hHandleID, qwHandle, uiOffset, 2, 2, (BYTE *)Output);
}

int HandleSetDataWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, WORD NewValue)
{
	return HandleSetData(hHandleID, qwHandle, uiOffset, 2, 2, (BYTE *)(&NewValue));
}

int HandleGetDataDWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, DWORD *Output)
{
	return HandleGetData(hHandleID, qwHandle, uiOffset, 4, 4, (BYTE *)Output);
}

int HandleSetDataDWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, DWORD NewValue)
{
	return HandleSetData(hHandleID, qwHandle, uiOffset, 4, 4, (BYTE *)(&NewValue));
}

int HandleGetDataQWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, QWORD *Output)
{
	return HandleGetData(hHandleID, qwHandle, uiOffset, 8, 8, (BYTE *)Output);
}

int HandleSetDataQWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, QWORD NewValue)
{
	return HandleSetData(hHandleID, qwHandle, uiOffset, 8, 8, (BYTE *)(&NewValue));
}

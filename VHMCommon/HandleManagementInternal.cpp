
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

	/* 핸들 갯수만큼 루프를 돌면서 해당 핸들 값을 찾음 */
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

	/* qwIndex가 핸들 갯수를 넘으면 안 됨 */
	QWORD qwHandleCount = HandleGetHandleCount(hHandleID);
	if (qwIndex >= qwHandleCount)
		return nullptr;

	/* 핸들 1개의 전체 크기 = (핸들 블록 크기) + (핸들 값의 크기) */
	vhmsize_t szHandleBlockTotalSize = pHandleID->HandleBlockSize + HANDLE_SIZE;

	/* (qwIndex번째의 포인터) =  (핸들 전체 크기) * (qwIndex) + (핸들 값의 크기) */
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

	/* 핸들 값을 인덱스로 변환하고, 그 인덱스르 포인터로 변환 */
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

	/* 요청된 새로운 블록 수가 현재 할당된 핸들 수보다 작아서는 안 됨 */
	if (pHandleID->HandleCount > qwNewBlockCount)
		return -1;

	/* 모든 블록에 대한 할당 해제가 요청되었으면 HandleMemFree 호출 */
	if (qwNewBlockCount == 0)
	{
		HandleMemFree(pHandleID->HandlePointer);
		pHandleID->HandlePointer = nullptr;
		pHandleID->AllocatedBlockCount = 0;
		return 0;
	}

	/* HandleMemRealloc 함수를 이용하여 메모리 재할당 */
	QWORD szNewMem = (pHandleID->HandleBlockSize + HANDLE_SIZE) * qwNewBlockCount;
	POINTER ptrNew = (POINTER)HandleMemRealloc(pHandleID->HandlePointer, szNewMem);
	if (!ptrNew)
		return -1; /* HandleMemRealloc 실패 */

	pHandleID->HandlePointer = (BYTE *)ptrNew;
	pHandleID->AllocatedBlockCount = qwNewBlockCount;
	return 0;
}

int HandleAllocateOneBlockUnit(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* 현재 할당된 블록을 기준하여 최소 1개에서 최대 BlockAllocationUnit개만큼 할당하여 BlockAllocationUnit에 맞춤 */
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

	/* 현재 할당된 블록을 기준하여 최소 1개에서 최대 BlockAllocationUnit개만큼 할당 해제하여 BlockAllocationUnit에 맞춤 */
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

	/* 여유 블록 갯수 = 전체 할당된 블록의 갯수 - 핸들로 할당된 블록 갯수 */
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

	/* qwValue를 1부터 1씩 증가시키면서 사용되지 않은 핸들 값을 찾음 */
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

	/* qwIndex번째의 핸들 값 위치를 얻어서 핸들 값을 반환 */
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

	/* qwIndex번째의 핸들 값 위치를 얻어서 qwHandleValue를 저장 */
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

	/* 핸들 구조체에서 해당 값을 리턴*/
	return pHandleID->LastAllocatedHandleValue;
}

int HandleSetLastAllocatedHandleValue(HHandleID hHandleID, QWORD qwValue)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return -1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* 핸들 구조체에 해당 값을 설정 */
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

	/* 올바른 핸들이라면 qwHandle에 대응하는 올바른 핸들 인덱스가 존재해야 함 */
	if (HandleValueToIndex(hHandleID, qwHandle) == ((QWORD)-1))
		return FALSE;

	return TRUE;
}

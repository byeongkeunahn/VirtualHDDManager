
#include "stdafx.h"
#include "VHMBase.h"
#include "HandleManagement.h"
#include "HandleManagementInternal.h"

HHandleID HandleIDCreate(vhmsize_t szBlock, QWORD qwAllocationUnit)
{
	if (szBlock <= 0 || qwAllocationUnit <= 0)
		return nullptr;

	/* 핸들 ID 정보 저장 공간 할당 */
	HANDLE_ID *pHandleID = (HANDLE_ID *)HandleMemAlloc(sizeof(HANDLE_ID));
	if (pHandleID == nullptr)
		return nullptr;

	/* 핸들 ID에 기본적인 정보 설정 */
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

	/* 모든 핸들을 파괴 */
	if (HandleRemoveAll(hHandleID) != 0)
		return -1;

	/* 핸들 ID 정보 저장 공간 메모리 해제 */
	HandleMemFree(hHandleID);

	return 0;
}

QWORD HandleAdd(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;

	/* 핸들로 할당되지 않은 블록 수를 조사하고, 없으면 할당을 요청  */
	if (HandleGetUnusedBlockCount(hHandleID) == 0)
	{
		if (HandleAllocateOneBlockUnit(hHandleID) == -1)
			return (QWORD)-1;
	}

	/* HandleGetUnusedBlockCount 함수가 또다시 0을 반환하면 오류 */
	if (HandleGetUnusedBlockCount(hHandleID) == 0)
		return (QWORD)-1;

	/* 핸들 값을 생성 후 핸들 1개를 할당되지 않은 블록에서 할당 */
	QWORD qwHandle = HandleGenerateValue(hHandleID);
	if (qwHandle == (QWORD)-1)
		return -1;

	/* 새로운 핸들의 값을 설정하고 핸들 카운트를 1만큼 증가시킴 */
	QWORD qwIndex = pHandleID->HandleCount;
	HandleSetValue(hHandleID, qwIndex, qwHandle);
	pHandleID->HandleCount++;

	return qwHandle;
}

vhmsize_t HandleGetBlockSize(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (vhmsize_t)-1;

	/* 핸들 블록 크기를 리턴 */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	return pHandleID->HandleBlockSize;
}

QWORD HandleGetHandleCount(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (vhmsize_t)-1;

	/* 핸들 갯수를 리턴 */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	return pHandleID->HandleCount;
}

QWORD HandleSetAllocationUnit(HHandleID hHandleID, QWORD qwNewAllocationUnit)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	if (qwNewAllocationUnit <= 0)
		return (QWORD)-1;

	/* 핸들 블록 메모리 할당 단위 크기를 설정 */
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

	/* 핸들을 인덱스로 변환 */
	QWORD qwIndex = HandleValueToIndex(hHandleID, qwHandle);
	if (qwIndex == (QWORD)-1)
		return -1;

	/* 인덱스를 포인터로 변환 */
	vhmsize_t szRealBlock = pHandleID->HandleBlockSize + HANDLE_SIZE;
	BYTE *pPointer = pHandleID->HandlePointer;
	pPointer += szRealBlock * qwIndex;

	/* 가장 마지막 블록이라면 핸들 블록을 0으로 채움 */
	if (qwIndex == (pHandleID->HandleCount - 1))
		HandleMemZero((POINTER)pPointer, szRealBlock);
	else
	{
		/* 가장 마지막 블록의 데이터를 이 블록으로 복사한 후 마지막 핸들 블록을 0으로 채움 */
		BYTE *pLastBlockPtr = pHandleID->HandlePointer + (szRealBlock * (pHandleID->HandleCount - 1));
		HandleMemCopy((POINTER)pPointer, (POINTER)pLastBlockPtr, szRealBlock);
		HandleMemZero((POINTER)pLastBlockPtr, szRealBlock);
	}

	/* 핸들 카운트 1개 감소 */
	pHandleID->HandleCount--;

	/* 미사용 블록이 할당 단위 크기 이상이면 할당 해제 */
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

	/* 핸들 메모리 할당 해제 및 값 초기화 */
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

	/* 핸들의 갯수가 0보다 커야 함 */
	if (HandleGetHandleCount(hHandleID) == 0)
		return (QWORD)-1;

	/* 핸들 집합 메모리의 가장 앞부분에 있는 핸들 리턴 */
	HANDLE_ID *pHandleID = (HANDLE_ID *)hHandleID;
	BYTE *pPointer = pHandleID->HandlePointer;
	QWORD qwHandle = *((QWORD *)pPointer);

	return qwHandle;
}

QWORD HandleGetNextHandle(HHandleID hHandleID, QWORD qwHandle)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	/* 핸들의 갯수가 0보다 커야 함 */
	if (HandleGetHandleCount(hHandleID) == 0)
		return (QWORD)-1;

	/* 핸들을 인덱스로 변환 */
	QWORD qwIndex;
	if ((qwIndex = HandleValueToIndex(hHandleID, qwHandle)) == (QWORD)-1)
		return (QWORD)-1;

	/* qwIndex + 1 번째의 핸들 리턴 */
	return HandleGetValue(hHandleID, qwIndex + 1);
}

QWORD HandleGetLastHandle(HHandleID hHandleID)
{
	if (!HandleIsHandleIDValid(hHandleID))
		return (QWORD)-1;

	/* 핸들의 갯수가 0보다 커야 함 */
	if (HandleGetHandleCount(hHandleID) == 0)
		return (QWORD)-1;

	/* 핸들 집합 메모리의 가장 뒷부분에 있는 핸들 리턴 */
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

	/* 얻고자 하는 데이터의 크기가 버퍼의 크기보다 클 수 없음 */
	if (szData > szBuffer)
		return -1;

	/* 얻고자 하는 데이터가 해당 핸들에 할당된 영역을 넘을 수 없음 */
	QWORD qwBlockSize = HandleGetBlockSize(hHandleID);
	if ((uiOffset + szData) > qwBlockSize)
		return -1;

	/* 핸들의 데이터가 저장된 영역의 포인터를 얻고 요청된 오프셋으로 포인터 설정 */
	BYTE *pPointer;
	pPointer = HandleGetPointerFromHandle(hHandleID, qwHandle);
	if (!pPointer)
		return -1;

	pPointer += uiOffset;

	BYTE *pByteBuffer = (BYTE *)pBuffer;

	/* szData만큼 데이터를 옮김 */
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

	/* 쓰고자 하는 데이터의 크기가 버퍼의 크기보다 클 수 없음 */
	if (szData > szBuffer)
		return -1;

	/* 쓰고자 하는 데이터가 해당 핸들에 할당된 영역을 넘을 수 없음 */
	QWORD qwBlockSize = HandleGetBlockSize(hHandleID);
	if ((uiOffset + szData) > qwBlockSize)
		return -1;

	/* 핸들의 데이터가 저장된 영역의 포인터를 얻고 요청된 오프셋으로 포인터 설정 */
	BYTE *pPointer;
	pPointer = HandleGetPointerFromHandle(hHandleID, qwHandle);
	if (!pPointer)
		return -1;

	pPointer += uiOffset;

	BYTE *pByteBuffer = (BYTE *)pBuffer;

	/* szData만큼 데이터를 옮김 */
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

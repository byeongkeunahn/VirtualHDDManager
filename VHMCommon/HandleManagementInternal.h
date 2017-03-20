
#pragma once

/*

핸들 관리 - 내부 정의 및 함수

*/

// 핸들 크기 -> 8바이트
#define HANDLE_SIZE 8

typedef struct _handle_management
{
	vhmsize_t	HandleBlockSize; // 핸들 블록 크기
	QWORD	HandleCount; // 핸들 갯수
	QWORD	AllocatedBlockCount; // 메모리에 영역이 할당된 핸들 블록 갯수 (속도 향상을 위해 예약된)
	QWORD	BlockAllocationUnit; // 핸들 블록으로 쓸 메모리를 할당할 때 쓰는 단위
	QWORD	LastAllocatedHandleValue; // 가장 마지막으로 할당된 핸들 값
	BYTE	*HandlePointer; // HandlePointer의 메모리 크기는 HandleCount * (HandleBlockSize + HANDLE_SIZE)

}HANDLE_ID;

POINTER HandleMemAlloc(vhmsize_t szMem); // 메모리 할당
POINTER HandleMemRealloc(POINTER ptrMemory, vhmsize_t szMem); // 메모리 재할당
POINTER HandleMemCopy(POINTER ptrDest, POINTER ptrSrc, vhmsize_t szCopy); // 메모리 복사 (ptrSrc -> ptrDest)
void HandleMemZero(POINTER ptrMemory, QWORD szZero); // 메모리를 0으로 채움
void HandleMemFree(POINTER ptrMemory); // 메모리 할당 해제

QWORD HandleValueToIndex(HHandleID hHandleID, QWORD qwHandle); // qwHandle의 인덱스 얻기 (오류 발생 시 -1 리턴)
BYTE *HandleIndexToPointer(HHandleID hHandleID, QWORD qwIndex); // qwIndex의 포인터 얻기 (오류 발생 시 nullptr 리턴)
BYTE *HandleGetPointerFromHandle(HHandleID hHandleID, QWORD qwHandle); // qwHandle에 해당하는 블록의 시작 위치 포인터 얻기

int HandleChangeAllocatedBlockCount(HHandleID hHandleID, QWORD qwNewBlockCount); // 미리 할당된 블록의 수를 qwNewBlockCount로 설정 (단, qwNewBlockCount가 HandleCount보다 작을 수 없음)
int HandleAllocateOneBlockUnit(HHandleID hHandleID); // BlockAllocationUnit 단위로 올림하여 메모리 할당
int HandleFreeOneBlockUnit(HHandleID hHandleID); // BlockAllocationUnit 단위로 내림하여 메모리 할당 해제
QWORD HandleGetUnusedBlockCount(HHandleID hHandleID); // 미리 할당된 블록의 총 갯수에서 핸들 용도로 할당된 블록의 갯수를 뺀 여유 블록의 갯수를 리턴

QWORD HandleGenerateValue(HHandleID hHandleID); // hHandleID에 포함되지 않은 핸들을 생성하여 리턴
QWORD HandleGetValue(HHandleID hHandleID, QWORD qwIndex); // HandlePointer의 qwIndex번째 블록의 핸들 값을 리턴
int HandleSetValue(HHandleID hHandleID, QWORD qwIndex, QWORD qwHandleValue); // HandlePointer의 qwIndex번째 블록의 핸들 값을 qwHandle로 설정

QWORD HandleGetLastAllocatedHandleValue(HHandleID hHandleID); // 가장 마지막으로 할당된 핸들 값을 리턴
int HandleSetLastAllocatedHandleValue(HHandleID hHandleID, QWORD qwValue); // 가장 마지막으로 할당된 핸들 값을 설정

BOOL HandleIsHandleIDValid(HHandleID hHandleID); // hHandleID가 유효한 핸들 ID인지 검사
BOOL HandleIsHandleValid(HHandleID hHandleID, QWORD qwHandle); // qwHandle이 hHandleID의 유효한 핸들 ID인지 검사

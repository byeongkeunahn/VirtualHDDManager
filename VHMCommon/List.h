
/* List.h */

#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMCOMMON_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*

C 이중 연결 리스트

*/

DEFINE_HANDLE(LIST_STRUCT);
DEFINE_HANDLE(LIST_HANDLE);

VHMDLL int ListCreate(LIST_HANDLE *ppListHandle); // 리스트 생성 및 핸들 리턴

VHMDLL int ListInsertAt(LIST_HANDLE pListHandle, QWORD qwPosition, void *pData); // qwPosition에 pData를 가지는 리스트 삽입
VHMDLL int ListInsertPrev(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pList 앞에 pData를 가지는 리스트 삽입
VHMDLL int ListInsertNext(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pList 뒤에 pData를 가지는 리스트 삽입
VHMDLL int ListInsertAtHead(LIST_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Head로 삽입
VHMDLL int ListInsertAtTail(LIST_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Tail로 삽입

VHMDLL int ListRemoveAt(LIST_HANDLE pListHandle, QWORD qwPosition); // pListHandle의 qwPosition 번째 리스트 제거
VHMDLL int ListRemove(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle의 pList 제거
VHMDLL int ListRemoveHead(LIST_HANDLE pListHandle); // pListHandle의 Head 제거
VHMDLL int ListRemoveTail(LIST_HANDLE pListHandle); // pListHandle의 Tail 제거
VHMDLL int ListRemoveAll(LIST_HANDLE pListHandle); // pListHandle의 모든 리스트 제거

VHMDLL void* ListSetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pListHandle의 pList에 pData 지정, 이전 값 반환
VHMDLL void* ListGetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle의 pList의 data 반환

VHMDLL LIST_STRUCT ListGetPrev(LIST_STRUCT pList); // pList의 이전 리스트 반환
VHMDLL LIST_STRUCT ListGetNext(LIST_STRUCT pList); // pList의 다음 리스트 반환
VHMDLL LIST_STRUCT ListGetHead(LIST_HANDLE pListHandle); // pListHandle의 Head 반환
VHMDLL LIST_STRUCT ListGetTail(LIST_HANDLE pListHandle); // pListHandle의 Tail 반환

VHMDLL LIST_STRUCT ListFindData(LIST_HANDLE pListHandle, void *pDataToFind); // pListHandle의 pDataToFind를 가지는 리스트 반환
VHMDLL QWORD ListDataToPosition(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle에서 pList의 순서 반환
VHMDLL LIST_STRUCT ListPositionToData(LIST_HANDLE pListHandle, QWORD qwPosition); // qwPosition 번째의 리스트 반환

VHMDLL QWORD ListGetItemCount(LIST_HANDLE pListHandle); // pListHandle의 리스트 수 반환
VHMDLL BOOL ListIsEmpty(LIST_HANDLE pListHandle); // pListHandle의 리스트 수가 0이면 TRUE 반환

VHMDLL int ListDestroy(LIST_HANDLE pListHandle); // pListHandle 파괴

#ifdef __cplusplus
}
#endif

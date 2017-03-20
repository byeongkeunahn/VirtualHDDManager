
/* ListV2.h */

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

C 이중 연결 리스트 V2 (CStreamMemDyn-based)

*/

DEFINE_HANDLE(LISTV2_ITEM);
DEFINE_HANDLE(LISTV2_HANDLE);

VHMDLL int ListV2Create(LISTV2_HANDLE *ppListHandle); // 리스트 생성 및 핸들 리턴

VHMDLL int ListV2InsertAt(LISTV2_HANDLE pListHandle, QWORD qwPosition, void *pData); // qwPosition에 pData를 가지는 리스트 삽입
VHMDLL int ListV2InsertPrev(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pList 앞에 pData를 가지는 리스트 삽입
VHMDLL int ListV2InsertNext(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pList 뒤에 pData를 가지는 리스트 삽입
VHMDLL int ListV2InsertAtHead(LISTV2_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Head로 삽입
VHMDLL int ListV2InsertAtTail(LISTV2_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Tail로 삽입

VHMDLL int ListV2RemoveAt(LISTV2_HANDLE pListHandle, QWORD qwPosition); // pListHandle의 qwPosition 번째 리스트 제거
VHMDLL int ListV2Remove(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle의 pList 제거
VHMDLL int ListV2RemoveHead(LISTV2_HANDLE pListHandle); // pListHandle의 Head 제거
VHMDLL int ListV2RemoveTail(LISTV2_HANDLE pListHandle); // pListHandle의 Tail 제거
VHMDLL int ListV2RemoveAll(LISTV2_HANDLE pListHandle); // pListHandle의 모든 리스트 제거

VHMDLL void* ListV2SetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pListHandle의 pList에 pData 지정, 이전 값 반환
VHMDLL void* ListV2GetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle의 pList의 data 반환

VHMDLL LISTV2_ITEM ListV2GetPrev(LISTV2_ITEM pList); // pList의 이전 리스트 반환
VHMDLL LISTV2_ITEM ListV2GetNext(LISTV2_ITEM pList); // pList의 다음 리스트 반환
VHMDLL LISTV2_ITEM ListV2GetHead(LISTV2_HANDLE pListHandle); // pListHandle의 Head 반환
VHMDLL LISTV2_ITEM ListV2GetTail(LISTV2_HANDLE pListHandle); // pListHandle의 Tail 반환

VHMDLL LISTV2_ITEM ListV2FindData(LISTV2_HANDLE pListHandle, void *pDataToFind); // pListHandle의 pDataToFind를 가지는 리스트 반환
VHMDLL QWORD ListV2DataToPosition(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle에서 pList의 순서 반환
VHMDLL LISTV2_ITEM ListV2PositionToData(LISTV2_HANDLE pListHandle, QWORD qwPosition); // qwPosition 번째의 리스트 반환

VHMDLL QWORD ListV2GetItemCount(LISTV2_HANDLE pListHandle); // pListHandle의 리스트 수 반환
VHMDLL BOOL ListV2IsEmpty(LISTV2_HANDLE pListHandle); // pListHandle의 리스트 수가 0이면 TRUE 반환

VHMDLL int ListV2Destroy(LISTV2_HANDLE pListHandle); // pListHandle 파괴

#ifdef __cplusplus
}
#endif

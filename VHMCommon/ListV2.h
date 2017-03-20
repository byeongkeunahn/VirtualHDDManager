
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

C ���� ���� ����Ʈ V2 (CStreamMemDyn-based)

*/

DEFINE_HANDLE(LISTV2_ITEM);
DEFINE_HANDLE(LISTV2_HANDLE);

VHMDLL int ListV2Create(LISTV2_HANDLE *ppListHandle); // ����Ʈ ���� �� �ڵ� ����

VHMDLL int ListV2InsertAt(LISTV2_HANDLE pListHandle, QWORD qwPosition, void *pData); // qwPosition�� pData�� ������ ����Ʈ ����
VHMDLL int ListV2InsertPrev(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pList �տ� pData�� ������ ����Ʈ ����
VHMDLL int ListV2InsertNext(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pList �ڿ� pData�� ������ ����Ʈ ����
VHMDLL int ListV2InsertAtHead(LISTV2_HANDLE pListHandle, void *pData); // pData�� ������ ����Ʈ�� Head�� ����
VHMDLL int ListV2InsertAtTail(LISTV2_HANDLE pListHandle, void *pData); // pData�� ������ ����Ʈ�� Tail�� ����

VHMDLL int ListV2RemoveAt(LISTV2_HANDLE pListHandle, QWORD qwPosition); // pListHandle�� qwPosition ��° ����Ʈ ����
VHMDLL int ListV2Remove(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle�� pList ����
VHMDLL int ListV2RemoveHead(LISTV2_HANDLE pListHandle); // pListHandle�� Head ����
VHMDLL int ListV2RemoveTail(LISTV2_HANDLE pListHandle); // pListHandle�� Tail ����
VHMDLL int ListV2RemoveAll(LISTV2_HANDLE pListHandle); // pListHandle�� ��� ����Ʈ ����

VHMDLL void* ListV2SetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pListHandle�� pList�� pData ����, ���� �� ��ȯ
VHMDLL void* ListV2GetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle�� pList�� data ��ȯ

VHMDLL LISTV2_ITEM ListV2GetPrev(LISTV2_ITEM pList); // pList�� ���� ����Ʈ ��ȯ
VHMDLL LISTV2_ITEM ListV2GetNext(LISTV2_ITEM pList); // pList�� ���� ����Ʈ ��ȯ
VHMDLL LISTV2_ITEM ListV2GetHead(LISTV2_HANDLE pListHandle); // pListHandle�� Head ��ȯ
VHMDLL LISTV2_ITEM ListV2GetTail(LISTV2_HANDLE pListHandle); // pListHandle�� Tail ��ȯ

VHMDLL LISTV2_ITEM ListV2FindData(LISTV2_HANDLE pListHandle, void *pDataToFind); // pListHandle�� pDataToFind�� ������ ����Ʈ ��ȯ
VHMDLL QWORD ListV2DataToPosition(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle���� pList�� ���� ��ȯ
VHMDLL LISTV2_ITEM ListV2PositionToData(LISTV2_HANDLE pListHandle, QWORD qwPosition); // qwPosition ��°�� ����Ʈ ��ȯ

VHMDLL QWORD ListV2GetItemCount(LISTV2_HANDLE pListHandle); // pListHandle�� ����Ʈ �� ��ȯ
VHMDLL BOOL ListV2IsEmpty(LISTV2_HANDLE pListHandle); // pListHandle�� ����Ʈ ���� 0�̸� TRUE ��ȯ

VHMDLL int ListV2Destroy(LISTV2_HANDLE pListHandle); // pListHandle �ı�

#ifdef __cplusplus
}
#endif

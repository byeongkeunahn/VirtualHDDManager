
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

C ���� ���� ����Ʈ

*/

DEFINE_HANDLE(LIST_STRUCT);
DEFINE_HANDLE(LIST_HANDLE);

VHMDLL int ListCreate(LIST_HANDLE *ppListHandle); // ����Ʈ ���� �� �ڵ� ����

VHMDLL int ListInsertAt(LIST_HANDLE pListHandle, QWORD qwPosition, void *pData); // qwPosition�� pData�� ������ ����Ʈ ����
VHMDLL int ListInsertPrev(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pList �տ� pData�� ������ ����Ʈ ����
VHMDLL int ListInsertNext(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pList �ڿ� pData�� ������ ����Ʈ ����
VHMDLL int ListInsertAtHead(LIST_HANDLE pListHandle, void *pData); // pData�� ������ ����Ʈ�� Head�� ����
VHMDLL int ListInsertAtTail(LIST_HANDLE pListHandle, void *pData); // pData�� ������ ����Ʈ�� Tail�� ����

VHMDLL int ListRemoveAt(LIST_HANDLE pListHandle, QWORD qwPosition); // pListHandle�� qwPosition ��° ����Ʈ ����
VHMDLL int ListRemove(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle�� pList ����
VHMDLL int ListRemoveHead(LIST_HANDLE pListHandle); // pListHandle�� Head ����
VHMDLL int ListRemoveTail(LIST_HANDLE pListHandle); // pListHandle�� Tail ����
VHMDLL int ListRemoveAll(LIST_HANDLE pListHandle); // pListHandle�� ��� ����Ʈ ����

VHMDLL void* ListSetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pListHandle�� pList�� pData ����, ���� �� ��ȯ
VHMDLL void* ListGetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle�� pList�� data ��ȯ

VHMDLL LIST_STRUCT ListGetPrev(LIST_STRUCT pList); // pList�� ���� ����Ʈ ��ȯ
VHMDLL LIST_STRUCT ListGetNext(LIST_STRUCT pList); // pList�� ���� ����Ʈ ��ȯ
VHMDLL LIST_STRUCT ListGetHead(LIST_HANDLE pListHandle); // pListHandle�� Head ��ȯ
VHMDLL LIST_STRUCT ListGetTail(LIST_HANDLE pListHandle); // pListHandle�� Tail ��ȯ

VHMDLL LIST_STRUCT ListFindData(LIST_HANDLE pListHandle, void *pDataToFind); // pListHandle�� pDataToFind�� ������ ����Ʈ ��ȯ
VHMDLL QWORD ListDataToPosition(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle���� pList�� ���� ��ȯ
VHMDLL LIST_STRUCT ListPositionToData(LIST_HANDLE pListHandle, QWORD qwPosition); // qwPosition ��°�� ����Ʈ ��ȯ

VHMDLL QWORD ListGetItemCount(LIST_HANDLE pListHandle); // pListHandle�� ����Ʈ �� ��ȯ
VHMDLL BOOL ListIsEmpty(LIST_HANDLE pListHandle); // pListHandle�� ����Ʈ ���� 0�̸� TRUE ��ȯ

VHMDLL int ListDestroy(LIST_HANDLE pListHandle); // pListHandle �ı�

#ifdef __cplusplus
}
#endif

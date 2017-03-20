
#include "stdafx.h"
#include "VHMBase.h"
#include "List.h"

/*

int ListCreate(LIST_HANDLE *ppListHandle); // 리스트 생성 및 핸들 리턴

int ListInsertAt(LIST_HANDLE pListHandle, QWORD qwPosition, void *pData); // qwPosition에 pData를 가지는 리스트 삽입
int ListInsertPrev(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pList 앞에 pData를 가지는 리스트 삽입
int ListInsertNext(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pList 뒤에 pData를 가지는 리스트 삽입
int ListInsertAtHead(LIST_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Head로 삽입
int ListInsertAtTail(LIST_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Tail로 삽입

int ListRemoveAt(LIST_HANDLE pListHandle, QWORD qwPosition); // pListHandle의 qwPosition 번째 리스트 제거
int ListRemove(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle의 pList 제거
int ListRemoveHead(LIST_HANDLE pListHandle); // pListHandle의 Head 제거
int ListRemoveTail(LIST_HANDLE pListHandle); // pListHandle의 Tail 제거
int ListRemoveAll(LIST_HANDLE pListHandle); // pListHandle의 모든 리스트 제거

void *ListSetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData); // pListHandle의 pList에 pData 지정, 이전 값 반환
void *ListGetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle의 pList의 data 반환

LIST_STRUCT ListGetPrev(LIST_STRUCT pList); // pList의 이전 리스트 반환
LIST_STRUCT ListGetNext(LIST_STRUCT pList); // pList의 다음 리스트 반환
LIST_STRUCT ListGetHead(LIST_HANDLE pListHandle); // pListHandle의 Head 반환
LIST_STRUCT ListGetTail(LIST_HANDLE pListHandle); // pListHandle의 Tail 반환

LIST_STRUCT ListFindData(LIST_HANDLE pListHandle, void *pDataToFind); // pListHandle의 pDataToFind를 가지는 리스트 반환
QWORD ListDataToPosition(LIST_HANDLE pListHandle, LIST_STRUCT pList); // pListHandle에서 pList의 순서 반환
LIST_STRUCT ListPositionToData(LIST_HANDLE pListHandle, QWORD qwPosition); // qwPosition 번째의 리스트 반환

QWORD ListGetItemCount(LIST_HANDLE pListHandle); // pListHandle의 리스트 수 반환
BOOL ListIsEmpty(LIST_HANDLE pListHandle); // pListHandle의 리스트 수가 0이면 TRUE 반환

int ListDestroy(LIST_HANDLE pListHandle); // pListHandle 파괴

*/

typedef struct _list_struct
{
	_list_struct *prev;
	_list_struct *next;

	void *data;

}_LIST_STRUCT;

typedef struct _list_handle
{
	_list_struct *head;
	_list_struct *tail;
	QWORD itemcount;

}_LIST_HANDLE;

int ListCreate(LIST_HANDLE *ppListHandle)
{
	if (!ppListHandle)
		return -1;

	_LIST_HANDLE *_pListHandle;
	_pListHandle = (_LIST_HANDLE *)malloc(sizeof(_LIST_HANDLE));
	_pListHandle->head = nullptr;
	_pListHandle->tail = nullptr;
	_pListHandle->itemcount = 0;
	*ppListHandle = (LIST_HANDLE)_pListHandle;
	return 0;
}

int ListInsertAt(LIST_HANDLE pListHandle, QWORD qwPosition, void *pData)
{
	if (!pListHandle)
		return -1;

	QWORD qwItemCount = ListGetItemCount(pListHandle);
	if (qwPosition > qwItemCount)
		return -1;

	_LIST_HANDLE *_pListHandle = (_LIST_HANDLE *)pListHandle;

	_LIST_STRUCT *_pListNew = (_LIST_STRUCT *)malloc(sizeof(_LIST_STRUCT));
	_pListNew->data = pData;

	if (qwItemCount == 0)
	{
		_pListNew->prev = nullptr;
		_pListNew->next = nullptr;
		_pListHandle->head = _pListNew;
		_pListHandle->tail = _pListNew;

		_pListHandle->itemcount++;

		return 0;
	}

	_LIST_STRUCT *_pListPrev, *_pListNext;
	if (qwPosition == 0)
	{
		_pListPrev = nullptr;
		_pListNext = _pListHandle->head;

		// _pListNew is new head
		_pListNew->prev = nullptr;
		_pListNew->next = _pListNext;
		_pListNext->prev = _pListNew;
		_pListHandle->head = _pListNew;
	}
	else if (qwPosition == qwItemCount)
	{
		_pListPrev = _pListHandle->tail;
		_pListNext = nullptr;

		// pListNew is new tail
		_pListNew->prev = _pListPrev;
		_pListNew->next = nullptr;
		_pListPrev->next = _pListNew;
		_pListHandle->tail = _pListNew;
	}
	else /* _pListHandle->itemcount >= 2, qwPosition != 0 && qwPosition != _pListHandle->itemcount
		  -->> _pListPrev != nullptr, _pListNext != nullptr */
	{
		_pListNext = _pListHandle->head;
		for (QWORD i = 0; i < qwPosition; i++)
			_pListNext = _pListNext->next;

		_pListPrev = _pListNext->prev;

		// _pListPrev->next and _pListNext->prev is _pListNew
		_pListPrev->next = _pListNew;
		_pListNew->prev = _pListPrev;
		_pListNew->next = _pListNext;
		_pListNext->prev = _pListNew;
	}

	_pListHandle->itemcount++;

	return 0;
}

int ListInsertPrev(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData)
{
	if (!pListHandle || !pList)
		return -1;

	QWORD qwPosition;
	if ((qwPosition = ListDataToPosition(pListHandle, pList)) == (QWORD)-1)
		return -1;

	return ListInsertAt(pListHandle, qwPosition, pData);
}

int ListInsertNext(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData)
{
	if (!pListHandle || !pList)
		return -1;

	QWORD qwPosition;
	if ((qwPosition = ListDataToPosition(pListHandle, pList)) == (QWORD)-1)
		return -1;

	return ListInsertAt(pListHandle, qwPosition + 1, pData);
}

int ListInsertAtHead(LIST_HANDLE pListHandle, void *pData)
{
	return ListInsertAt(pListHandle, 0, pData);
}

int ListInsertAtTail(LIST_HANDLE pListHandle, void *pData)
{
	return ListInsertAt(pListHandle, ListGetItemCount(pListHandle), pData);
}

int ListRemoveAt(LIST_HANDLE pListHandle, QWORD qwPosition)
{
	if (!pListHandle)
		return -1;

	LIST_STRUCT pList;
	if ((pList = ListPositionToData(pListHandle, qwPosition)) == nullptr)
		return -1;

	return ListRemove(pListHandle, pList);
}

int ListRemove(LIST_HANDLE pListHandle, LIST_STRUCT pList)
{
	if (!pListHandle || !pList)
		return -1;

	_LIST_HANDLE *_pListHandle = (_LIST_HANDLE *)pListHandle;
	_LIST_STRUCT *_pList = (_LIST_STRUCT *)pList;

	QWORD qwPosition;
	if ((qwPosition = ListDataToPosition(pListHandle, pList)) == -1)
		return -1;

	if (_pListHandle->itemcount == 0)
		return -1;

	if (_pListHandle->itemcount == 1) /* _pList is both head and tail */
	{
		_pListHandle->head = nullptr;
		_pListHandle->tail = nullptr;

		goto final;
	}

	if (qwPosition == 0) /* _pList->head is nullptr */
	{
		_LIST_STRUCT *_pNewHead = _pList->next;
		_pNewHead->prev = nullptr;

		_pListHandle->head = _pNewHead;

		goto final;
	}
	else if (qwPosition == (ListGetItemCount(pListHandle) - 1))
	{
		_LIST_STRUCT *_pNewTail = _pList->prev;
		_pNewTail->next = nullptr;

		_pListHandle->tail = _pNewTail;
	}
	else /* 1 <= qwPosition <= ListGetItemCount(pListHandle)-1
		  -->> _pList->prev != nullptr and _pList->next != nullptr */
	{
		_LIST_STRUCT *_pNewPrev = _pList->prev;
		_LIST_STRUCT *_pNewNext = _pList->next;

		_pNewPrev->next = _pNewNext;
		_pNewNext->prev = _pNewPrev;
	}

	final:
	free(pList);
	_pListHandle->itemcount--;

	return 0;
}

int ListRemoveHead(LIST_HANDLE pListHandle)
{
	return ListRemove(pListHandle, (LIST_STRUCT)((_LIST_HANDLE *)pListHandle)->head);
}

int ListRemoveTail(LIST_HANDLE pListHandle)
{
	return ListRemove(pListHandle, (LIST_STRUCT)((_LIST_HANDLE *)pListHandle)->tail);
}

int ListRemoveAll(LIST_HANDLE pListHandle)
{
	if (!pListHandle)
		return -1;

	QWORD qwItemCount = ListGetItemCount(pListHandle);
	if (qwItemCount == 0)
		return 0;
	else
	{
		for (QWORD i = 0; i < qwItemCount; i++)
			ListRemoveAt(pListHandle, 0);
	}

	return 0;
}

void *ListSetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList, void *pData)
{
	if (!pListHandle || !pList)
		return nullptr;

	_LIST_STRUCT *_pList = (_LIST_STRUCT *)pList;

	void *pDataPrev = _pList->data;
	_pList->data = pData;
	return pDataPrev;
}

void *ListGetAt(LIST_HANDLE pListHandle, LIST_STRUCT pList)
{
	if (!pListHandle || !pList)
		return nullptr;

	return ((_LIST_STRUCT *)pList)->data;
}

LIST_STRUCT ListGetPrev(LIST_STRUCT pList)
{
	if (!pList)
		return nullptr;

	return (LIST_STRUCT)((_LIST_STRUCT *)pList)->prev;
}

LIST_STRUCT ListGetNext(LIST_STRUCT pList)
{
	if (!pList)
		return nullptr;

	return (LIST_STRUCT)((_LIST_STRUCT *)pList)->next;
}

LIST_STRUCT ListGetHead(LIST_HANDLE pListHandle)
{
	if (!pListHandle)
		return nullptr;

	return (LIST_STRUCT)((_LIST_HANDLE *)pListHandle)->head;
}

LIST_STRUCT ListGetTail(LIST_HANDLE pListHandle)
{
	if (!pListHandle)
		return nullptr;

	return (LIST_STRUCT)((_LIST_HANDLE *)pListHandle)->tail;
}

LIST_STRUCT ListFindData(LIST_HANDLE pListHandle, void *pDataToFind)
{
	if (!pListHandle)
		return nullptr;

	_LIST_STRUCT *_pList = ((_LIST_HANDLE *)pListHandle)->head;
	if (_pList == nullptr)
		return nullptr;

	_LIST_STRUCT *_pFind = nullptr;
	while (1)
	{
		if (_pList->next == nullptr)
			break;

		if (_pList->data == pDataToFind)
		{
			_pFind = _pList;
			break;
		}
	}

	return (LIST_STRUCT)_pFind;
}

QWORD ListDataToPosition(LIST_HANDLE pListHandle, LIST_STRUCT pList)
{
	if (!pListHandle)
		return (QWORD)-1;

	QWORD qwPosition = 0;
	BOOL bFind = FALSE;

	_LIST_STRUCT *_pListI = ((_LIST_HANDLE *)pListHandle)->head;
	_LIST_STRUCT *_pList = (_LIST_STRUCT *)pList;
	if (!_pList || !_pListI)
		return qwPosition;

	while (1)
	{
		if (_pListI == _pList)
		{
			bFind = TRUE;
			break;
		}

		if (_pListI->next == nullptr)
			break;

		_pListI = _pListI->next;

		qwPosition++;
	}

	if (bFind == FALSE)
		return (QWORD)-1;

	return qwPosition;
}

LIST_STRUCT ListPositionToData(LIST_HANDLE pListHandle, QWORD qwPosition)
{
	if (!pListHandle)
		return nullptr;

	if (qwPosition >= ListGetItemCount(pListHandle))
		return nullptr;

	if (ListGetItemCount(pListHandle) == 0)
		return nullptr;

	_LIST_STRUCT *_pList = ((_LIST_HANDLE *)pListHandle)->head;
	for (QWORD i = 0; i < qwPosition; i++)
		_pList = _pList->next;

	return (LIST_STRUCT)_pList;
}

QWORD ListGetItemCount(LIST_HANDLE pListHandle)
{
	if (!pListHandle)
		return (QWORD)-1;

	return ((_LIST_HANDLE *)pListHandle)->itemcount;
}

BOOL ListIsEmpty(LIST_HANDLE pListHandle)
{
	if (!pListHandle)
		return FALSE;

	return (ListGetItemCount(pListHandle) == 0) ? TRUE : FALSE;
}

int ListDestroy(LIST_HANDLE pListHandle)
{
	if (!pListHandle)
		return -1;

	if (ListRemoveAll(pListHandle) == -1)
		return -1;

	free(pListHandle);

	return 0;
}


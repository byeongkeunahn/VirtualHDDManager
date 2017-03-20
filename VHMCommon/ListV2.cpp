
#include "stdafx.h"
#include "VHMBase.h"
#include "Stream.h"
#include "StreamMemDyn.h"
#include "ListV2.h"

/*

int ListV2Create(LISTV2_HANDLE *ppListHandle); // 리스트 생성 및 핸들 리턴

int ListV2InsertAt(LISTV2_HANDLE pListHandle, QWORD qwPosition, void *pData); // qwPosition에 pData를 가지는 리스트 삽입
int ListV2InsertPrev(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pList 앞에 pData를 가지는 리스트 삽입
int ListV2InsertNext(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pList 뒤에 pData를 가지는 리스트 삽입
int ListV2InsertAtHead(LISTV2_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Head로 삽입
int ListV2InsertAtTail(LISTV2_HANDLE pListHandle, void *pData); // pData를 가지는 리스트를 Tail로 삽입

int ListV2RemoveAt(LISTV2_HANDLE pListHandle, QWORD qwPosition); // pListHandle의 qwPosition 번째 리스트 제거
int ListV2Remove(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle의 pList 제거
int ListV2RemoveHead(LISTV2_HANDLE pListHandle); // pListHandle의 Head 제거
int ListV2RemoveTail(LISTV2_HANDLE pListHandle); // pListHandle의 Tail 제거
int ListV2RemoveAll(LISTV2_HANDLE pListHandle); // pListHandle의 모든 리스트 제거

void* ListV2SetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData); // pListHandle의 pList에 pData 지정, 이전 값 반환
void* ListV2GetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle의 pList의 data 반환

LISTV2_ITEM ListV2GetPrev(LISTV2_ITEM pList); // pList의 이전 리스트 반환
LISTV2_ITEM ListV2GetNext(LISTV2_ITEM pList); // pList의 다음 리스트 반환
LISTV2_ITEM ListV2GetHead(LISTV2_HANDLE pListHandle); // pListHandle의 Head 반환
LISTV2_ITEM ListV2GetTail(LISTV2_HANDLE pListHandle); // pListHandle의 Tail 반환

LISTV2_ITEM ListV2FindData(LISTV2_HANDLE pListHandle, void *pDataToFind); // pListHandle의 pDataToFind를 가지는 리스트 반환
QWORD ListV2DataToPosition(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList); // pListHandle에서 pList의 순서 반환
LISTV2_ITEM ListV2PositionToData(LISTV2_HANDLE pListHandle, QWORD qwPosition); // qwPosition 번째의 리스트 반환

QWORD ListV2GetItemCount(LISTV2_HANDLE pListHandle); // pListHandle의 리스트 수 반환
BOOL ListV2IsEmpty(LISTV2_HANDLE pListHandle); // pListHandle의 리스트 수가 0이면 TRUE 반환

int ListV2Destroy(LISTV2_HANDLE pListHandle); // pListHandle 파괴

*/

typedef struct _listv2_item
{
	_listv2_item	*prev;
	_listv2_item	*next;

	void			*data;

}_LISTV2_ITEM;

typedef struct _listv2_handle
{
	_listv2_item	*head;
	_listv2_item	*tail;
	QWORD			itemcount;

}_LISTV2_HANDLE;

int ListV2Create(LISTV2_HANDLE *ppListHandle)
{
	if(!ppListHandle)
		return -1;

	_LISTV2_HANDLE *_pListHandle;
	_pListHandle = (_LISTV2_HANDLE *)malloc(sizeof(_LISTV2_HANDLE));
	_pListHandle->head = nullptr;
	_pListHandle->tail = nullptr;
	_pListHandle->itemcount = 0;
	*ppListHandle = (LISTV2_HANDLE)_pListHandle;
	return 0;
}

int ListV2InsertAt(LISTV2_HANDLE pListHandle, QWORD qwPosition, void *pData)
{
	if(!pListHandle)
		return -1;

	QWORD qwItemCount = ListV2GetItemCount(pListHandle);
	if(qwPosition > qwItemCount)
		return -1;

	_LISTV2_HANDLE *_pListHandle = (_LISTV2_HANDLE *)pListHandle;

	_LISTV2_ITEM *_pListNew = (_LISTV2_ITEM *)malloc(sizeof(_LISTV2_ITEM));
	_pListNew->data = pData;

	if(qwItemCount == 0)
	{
		_pListNew->prev = nullptr;
		_pListNew->next = nullptr;
		_pListHandle->head = _pListNew;
		_pListHandle->tail = _pListNew;
		
		_pListHandle->itemcount++;

		return 0;
	}

	_LISTV2_ITEM *_pListPrev, *_pListNext;
	if(qwPosition == 0)
	{
		_pListPrev = nullptr;
		_pListNext = _pListHandle->head;

		// _pListNew is new head
		_pListNew->prev = nullptr;
		_pListNew->next = _pListNext;
		_pListNext->prev = _pListNew;
		_pListHandle->head = _pListNew;
	}
	else if(qwPosition == qwItemCount)
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
		for(QWORD i=0; i<qwPosition; i++)
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

int ListV2InsertPrev(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData)
{
	if(!pListHandle || !pList)
		return -1;

	QWORD qwPosition;
	if((qwPosition = ListV2DataToPosition(pListHandle, pList)) == (QWORD) -1)
		return -1;

	return ListV2InsertAt(pListHandle, qwPosition, pData);
}

int ListV2InsertNext(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData)
{
	if(!pListHandle || !pList)
		return -1;

	QWORD qwPosition;
	if((qwPosition = ListV2DataToPosition(pListHandle, pList)) == (QWORD) -1)
		return -1;

	return ListV2InsertAt(pListHandle, qwPosition + 1, pData);
}

int ListV2InsertAtHead(LISTV2_HANDLE pListHandle, void *pData)
{
	return ListV2InsertAt(pListHandle, 0, pData);
}

int ListV2InsertAtTail(LISTV2_HANDLE pListHandle, void *pData)
{
	return ListV2InsertAt(pListHandle, ListV2GetItemCount(pListHandle), pData);
}

int ListV2RemoveAt(LISTV2_HANDLE pListHandle, QWORD qwPosition)
{
	if(!pListHandle)
		return -1;

	LISTV2_ITEM pList;
	if((pList = ListV2PositionToData(pListHandle, qwPosition)) == nullptr)
		return -1;

	return ListV2Remove(pListHandle, pList);
}

int ListV2Remove(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList)
{
	if(!pListHandle || !pList)
		return -1;
	
	_LISTV2_HANDLE *_pListHandle = (_LISTV2_HANDLE *)pListHandle;
	_LISTV2_ITEM *_pList = (_LISTV2_ITEM *)pList;

	QWORD qwPosition;
	if((qwPosition = ListV2DataToPosition(pListHandle, pList)) == -1)
		return -1;

	if (_pListHandle->itemcount == 0)
		return -1;

	if (_pListHandle->itemcount == 1) /* _pList is both head and tail */
	{
		_pListHandle->head = nullptr;
		_pListHandle->tail = nullptr;

		goto final;
	}

	if(qwPosition == 0) /* _pList->head is nullptr */
	{
		_LISTV2_ITEM *_pNewHead = _pList->next;
		_pNewHead->prev = nullptr;

		_pListHandle->head = _pNewHead;

		goto final;
	}
	else if(qwPosition == (ListV2GetItemCount(pListHandle) - 1))
	{
		_LISTV2_ITEM *_pNewTail = _pList->prev;
		_pNewTail->next = nullptr;
		
		_pListHandle->tail = _pNewTail;
	}
	else /* 1 <= qwPosition <= ListGetItemCount(pListHandle)-1
		  -->> _pList->prev != nullptr and _pList->next != nullptr */
	{
		_LISTV2_ITEM *_pNewPrev = _pList->prev;
		_LISTV2_ITEM *_pNewNext = _pList->next;

		_pNewPrev->next = _pNewNext;
		_pNewNext->prev = _pNewPrev;
	}

final:
	free(pList);
	_pListHandle->itemcount--;

	return 0;
}

int ListV2RemoveHead(LISTV2_HANDLE pListHandle)
{
	return ListV2Remove(pListHandle, (LISTV2_ITEM)((_LISTV2_HANDLE *)pListHandle)->head);
}

int ListV2RemoveTail(LISTV2_HANDLE pListHandle)
{
	return ListV2Remove(pListHandle, (LISTV2_ITEM)((_LISTV2_HANDLE *)pListHandle)->tail);
}

int ListV2RemoveAll(LISTV2_HANDLE pListHandle)
{
	if (!pListHandle)
		return -1;

	QWORD qwItemCount = ListV2GetItemCount(pListHandle);
	if (qwItemCount == 0)
		return 0;
	else
	{
		for (QWORD i = 0; i < qwItemCount; ++i)
			ListV2RemoveAt(pListHandle, 0);
	}

	return 0;
}

void *ListV2SetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList, void *pData)
{
	if (!pListHandle || !pList)
		return nullptr;

	_LISTV2_ITEM *_pList = (_LISTV2_ITEM *)pList;

	void *pDataPrev = _pList->data;
	_pList->data = pData;
	return pDataPrev;
}

void *ListV2GetAt(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList)
{
	if(!pListHandle || !pList)
		return nullptr;

	return ((_LISTV2_ITEM *)pList)->data;
}

LISTV2_ITEM ListV2GetPrev(LISTV2_ITEM pList)
{
	if(!pList)
		return nullptr;

	return (LISTV2_ITEM)((_LISTV2_ITEM *)pList)->prev;
}

LISTV2_ITEM ListV2GetNext(LISTV2_ITEM pList)
{
	if(!pList)
		return nullptr;

	return (LISTV2_ITEM)((_LISTV2_ITEM *)pList)->next;
}

LISTV2_ITEM ListV2GetHead(LISTV2_HANDLE pListHandle)
{
	if(!pListHandle)
		return nullptr;

	return (LISTV2_ITEM)((_LISTV2_HANDLE *)pListHandle)->head;
}

LISTV2_ITEM ListV2GetTail(LISTV2_HANDLE pListHandle)
{
	if(!pListHandle)
		return nullptr;

	return (LISTV2_ITEM)((_LISTV2_HANDLE *)pListHandle)->tail;
}

LISTV2_ITEM ListV2FindData(LISTV2_HANDLE pListHandle, void *pDataToFind)
{
	if(!pListHandle)
		return nullptr;

	_LISTV2_ITEM *_pList = ((_LISTV2_HANDLE *)pListHandle)->head;
	if(_pList == nullptr)
		return nullptr;

	_LISTV2_ITEM *_pFind = nullptr;
	while(1)
	{
		if(_pList->next == nullptr)
			break;

		if(_pList->data == pDataToFind)
		{
			_pFind = _pList;
			break;
		}
	}

	return (LISTV2_ITEM)_pFind;
}

QWORD ListV2DataToPosition(LISTV2_HANDLE pListHandle, LISTV2_ITEM pList)
{
	if(!pListHandle)
		return (QWORD) -1;

	QWORD qwPosition = 0;
	BOOL bFind = FALSE;

	_LISTV2_ITEM *_pListI = ((_LISTV2_HANDLE *)pListHandle)->head;
	_LISTV2_ITEM *_pList = (_LISTV2_ITEM *)pList;
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
		return (QWORD) -1;

	return qwPosition;
}

LISTV2_ITEM ListV2PositionToData(LISTV2_HANDLE pListHandle, QWORD qwPosition)
{
	if (!pListHandle)
		return nullptr;

	if (qwPosition >= ListV2GetItemCount(pListHandle))
		return nullptr;

	if (ListV2GetItemCount(pListHandle) == 0)
		return nullptr;

	_LISTV2_ITEM *_pList = ((_LISTV2_HANDLE *)pListHandle)->head;
	for (QWORD i = 0; i < qwPosition; i++)
		_pList = _pList->next;
	
	return (LISTV2_ITEM)_pList;
}

QWORD ListV2GetItemCount(LISTV2_HANDLE pListHandle)
{
	if(!pListHandle)
		return (QWORD) -1;

	return ((_LISTV2_HANDLE *)pListHandle)->itemcount;
}

BOOL ListV2IsEmpty(LISTV2_HANDLE pListHandle)
{
	if(!pListHandle)
		return FALSE;

	return (ListV2GetItemCount(pListHandle) == 0) ? TRUE : FALSE;
}

int ListV2Destroy(LISTV2_HANDLE pListHandle)
{
	if(!pListHandle)
		return -1;

	if(ListV2RemoveAll(pListHandle) == -1)
		return -1;

	free(pListHandle);

	return 0;
}

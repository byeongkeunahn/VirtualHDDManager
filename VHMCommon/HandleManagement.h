
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

�ڵ� ����

*/

VHMDLL DEFINE_HANDLE(HHandleID);

VHMDLL HHandleID HandleIDCreate(vhmsize_t szBlock, QWORD qwAllocationUnit); // ���ο� �ڵ� ���� ���� (�ϳ��� ����� szBlock ũ��)
VHMDLL int HandleIDDestroy(HHandleID hHandleID); // �ڵ� ���� �ı�

VHMDLL QWORD HandleAdd(HHandleID hHandleID); // �ڵ� ���տ� �ڵ� ����
VHMDLL vhmsize_t HandleGetBlockSize(HHandleID hHandleID); // �ڵ� ������ ��� ũ�� ���
VHMDLL QWORD HandleGetHandleCount(HHandleID hHandleID); // �ڵ� ���� ���
VHMDLL QWORD HandleSetAllocationUnit(HHandleID hHandleID, QWORD qwNewAllocationUnit); // �ڵ� ��� �޸� �Ҵ� ������ ���� (���� �޸� �Ҵ� ������ ����)
VHMDLL int HandleRemove(HHandleID hHandleID, QWORD qwHandle); // �ڵ� ������ �ڵ� �ı�
VHMDLL int HandleRemoveAll(HHandleID hHandleID); // �ڵ� ������ ��� �ڵ� �ı�

VHMDLL QWORD HandleGetFirstHandle(HHandleID hHandleID); // �ڵ� ���� �޸��� ���� �պκп� �ִ� �ڵ� ����
VHMDLL QWORD HandleGetNextHandle(HHandleID hHandleID, QWORD qwHandle); // �ڵ� ���� �޸��� ���� �κп� �ִ� �ڵ� ����
VHMDLL QWORD HandleGetLastHandle(HHandleID hHandleID); // �ڵ� ���� �޸��� ���� �޺κп� �ִ� �ڵ� ����

VHMDLL int HandleGetData(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, vhmsize_t szData, vhmsize_t szBuffer, void *pBuffer); // �ڵ� �� ���
VHMDLL int HandleSetData(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, vhmsize_t szData, vhmsize_t szBuffer, const void *pBuffer); // �ڵ� �� ����

VHMDLL int HandleGetDataByte(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, BYTE *Output); // �ڵ� �� ��� (1����Ʈ)
VHMDLL int HandleSetDataByte(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, BYTE NewValue); // �ڵ� �� ���� (1����Ʈ), ���� �� ����
VHMDLL int HandleGetDataWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, WORD *Output); // �ڵ� �� ��� (2����Ʈ)
VHMDLL int HandleSetDataWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, WORD NewValue); // �ڵ� �� ���� (2����Ʈ), ���� �� ����
VHMDLL int HandleGetDataDWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, DWORD *Output); // �ڵ� �� ��� (4����Ʈ)
VHMDLL int HandleSetDataDWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, DWORD NewValue); // �ڵ� �� ���� (4����Ʈ), ���� �� ����
VHMDLL int HandleGetDataQWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, QWORD *Output); // �ڵ� �� ��� (8����Ʈ)
VHMDLL int HandleSetDataQWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, QWORD NewValue); // �ڵ� �� ���� (8����Ʈ), ���� �� ����


#ifdef __cplusplus
}
#endif

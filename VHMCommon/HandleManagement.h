
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

핸들 관리

*/

VHMDLL DEFINE_HANDLE(HHandleID);

VHMDLL HHandleID HandleIDCreate(vhmsize_t szBlock, QWORD qwAllocationUnit); // 새로운 핸들 집합 생성 (하나의 블록은 szBlock 크기)
VHMDLL int HandleIDDestroy(HHandleID hHandleID); // 핸들 집합 파괴

VHMDLL QWORD HandleAdd(HHandleID hHandleID); // 핸들 집합에 핸들 생성
VHMDLL vhmsize_t HandleGetBlockSize(HHandleID hHandleID); // 핸들 집합의 블록 크기 얻기
VHMDLL QWORD HandleGetHandleCount(HHandleID hHandleID); // 핸들 갯수 얻기
VHMDLL QWORD HandleSetAllocationUnit(HHandleID hHandleID, QWORD qwNewAllocationUnit); // 핸들 블록 메모리 할당 단위를 변경 (예전 메모리 할당 단위를 리턴)
VHMDLL int HandleRemove(HHandleID hHandleID, QWORD qwHandle); // 핸들 집합의 핸들 파괴
VHMDLL int HandleRemoveAll(HHandleID hHandleID); // 핸들 집합의 모든 핸들 파괴

VHMDLL QWORD HandleGetFirstHandle(HHandleID hHandleID); // 핸들 집합 메모리의 가장 앞부분에 있는 핸들 리턴
VHMDLL QWORD HandleGetNextHandle(HHandleID hHandleID, QWORD qwHandle); // 핸들 집합 메모리의 다음 부분에 있는 핸들 리턴
VHMDLL QWORD HandleGetLastHandle(HHandleID hHandleID); // 핸들 집합 메모리의 가장 뒷부분에 있는 핸들 리턴

VHMDLL int HandleGetData(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, vhmsize_t szData, vhmsize_t szBuffer, void *pBuffer); // 핸들 값 얻기
VHMDLL int HandleSetData(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, vhmsize_t szData, vhmsize_t szBuffer, const void *pBuffer); // 핸들 값 설정

VHMDLL int HandleGetDataByte(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, BYTE *Output); // 핸들 값 얻기 (1바이트)
VHMDLL int HandleSetDataByte(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, BYTE NewValue); // 핸들 값 설정 (1바이트), 예전 값 리턴
VHMDLL int HandleGetDataWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, WORD *Output); // 핸들 값 얻기 (2바이트)
VHMDLL int HandleSetDataWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, WORD NewValue); // 핸들 값 설정 (2바이트), 예전 값 리턴
VHMDLL int HandleGetDataDWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, DWORD *Output); // 핸들 값 얻기 (4바이트)
VHMDLL int HandleSetDataDWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, DWORD NewValue); // 핸들 값 설정 (4바이트), 예전 값 리턴
VHMDLL int HandleGetDataQWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, QWORD *Output); // 핸들 값 얻기 (8바이트)
VHMDLL int HandleSetDataQWORD(HHandleID hHandleID, QWORD qwHandle, UINT64 uiOffset, QWORD NewValue); // 핸들 값 설정 (8바이트), 예전 값 리턴


#ifdef __cplusplus
}
#endif

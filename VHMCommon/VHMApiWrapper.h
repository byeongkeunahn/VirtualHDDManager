
/* VHMApiWrapper.h */

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


/* Registry Functions */

#define HKLM HKEY_LOCAL_MACHINE
#define HKCU HKEY_CURRENT_USER
#define HKCR HKEY_CLASSES_ROOT

VHMDLL UINT VHMRegReadInt(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, INT nDefault);
VHMDLL BOOL VHMRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault,
	LPTSTR lpRet, DWORD nSize);
VHMDLL BOOL VHMRegReadStringGetSize(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, DWORD *pnSize);
VHMDLL BOOL VHMRegWriteInt(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, UINT nData);
VHMDLL BOOL VHMRegWriteString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData);
VHMDLL BOOL VHMRegDeleteKey(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue);


/* File Functions */

VHMDLL DWORD VHMReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead);
VHMDLL DWORD VHMWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite);
VHMDLL UINT64 VHMGetFileSize64(HANDLE hFile);
VHMDLL BOOL VHMSetFilePointer64(HANDLE hFile, UINT64 liDistanceToMove, UINT64 *lpNewFilePointer, DWORD dwMoveMethod);
VHMDLL BOOL CheckFileExistence(LPCTSTR lpszFilePath);


#ifdef __cplusplus
}
#endif

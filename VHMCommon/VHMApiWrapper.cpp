
#include "stdafx.h"
#include "VHMBase.h"
#include "VHMApiWrapper.h"

/* Registry Functions */


// 정수값을 읽는다.
UINT VHMRegReadInt(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, INT nDefault)
{
	HKEY key;
	DWORD dwDisp;
	UINT Result;
	DWORD Size;

	if (::RegCreateKeyEx(hKey, lpKey, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_READ, nullptr, &key, &dwDisp)
		!= ERROR_SUCCESS) 
		return 0;

	Size = sizeof(LONG);
	if (::RegQueryValueEx(key, lpValue, 0, nullptr, (LPBYTE)&Result, &Size)
		!= ERROR_SUCCESS)
		Result = nDefault;

	RegCloseKey(key);
	return Result;
}

// 문자열을 읽는다.
BOOL VHMRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault, 
   LPTSTR lpRet, DWORD nSize) 
{
	HKEY key;
	DWORD dwDisp;
	DWORD Size;
	if (::RegCreateKeyEx(hKey, lpKey, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_READ, nullptr, &key, &dwDisp)
		!= ERROR_SUCCESS) 
		return FALSE;

	Size = nSize;
	if (::RegQueryValueEx(key, lpValue, 0, nullptr, (LPBYTE)lpRet, &Size)
		!= ERROR_SUCCESS)
	{
		lstrcpy(lpRet, lpDefault);
		return FALSE;
	}

	RegCloseKey(key);
	return TRUE;
}

// 문자열 바이트 크기를 얻는다.
BOOL VHMRegReadStringGetSize(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, DWORD *pnSize)
{
	HKEY key;
	DWORD dwDisp;

	if (::RegCreateKeyEx(hKey, lpKey, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_READ, nullptr, &key, &dwDisp)
		!= ERROR_SUCCESS)
		return FALSE;

	if (::RegQueryValueEx(key, lpValue, 0, nullptr, nullptr, pnSize)
		!= ERROR_SUCCESS)
	{
		*pnSize = 0;
		return FALSE;
	}

	RegCloseKey(key);
	return TRUE;

}

// 정수값을 쓴다.
BOOL VHMRegWriteInt(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, UINT nData)
{
	HKEY key;
	DWORD dwDisp;

	if (::RegCreateKeyEx(hKey, lpKey, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, &dwDisp)
		!= ERROR_SUCCESS)
		return FALSE;

	if (::RegSetValueEx(key, lpValue, 0, REG_DWORD, (LPBYTE)&nData, sizeof(UINT))
		!= ERROR_SUCCESS) 
		return FALSE;

	RegCloseKey(key);
	return TRUE;
}

// 문자열을 쓴다.
BOOL VHMRegWriteString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData)
{
	HKEY key;
	DWORD dwDisp;

	if (::RegCreateKeyEx(hKey, lpKey, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, &dwDisp)
		!= ERROR_SUCCESS)
		return FALSE;

	if (::RegSetValueEx(key, lpValue, 0, REG_SZ, (LPBYTE)lpData, (_tcslen(lpData) + 1) * sizeof(TCHAR))
		!= ERROR_SUCCESS) 
		return FALSE;

	RegCloseKey(key);
	return TRUE;
}

// 값을 지운다.
BOOL VHMRegDeleteKey(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue)
{
	HKEY key;
	DWORD dwDisp;

	if (::RegCreateKeyEx(hKey, lpKey, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, &dwDisp)
		!= ERROR_SUCCESS)
		return FALSE;

	if (::RegDeleteKey(key, lpValue) != ERROR_SUCCESS)
		return FALSE;

	RegCloseKey(key);
	return TRUE;
}

/* File Functions */


DWORD VHMReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead)
{
	DWORD nNumberOfBytesRead;
	::ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &nNumberOfBytesRead, nullptr);

	return nNumberOfBytesRead;
}

DWORD VHMWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite)
{
	DWORD nNumberOfBytesWritten;
	::WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &nNumberOfBytesWritten, nullptr);
	
	return nNumberOfBytesWritten;
}

UINT64 VHMGetFileSize64(HANDLE hFile)
{
	LARGE_INTEGER li;
	if (::GetFileSizeEx(hFile, &li) == FALSE)
		return VHM_INVALID_VALUE;

	return li.QuadPart;
}

BOOL VHMSetFilePointer64(HANDLE hFile, UINT64 liDistanceToMove, UINT64 *lpNewFilePointer, DWORD dwMoveMethod)
{
	LARGE_INTEGER li1, li2;
	li1.QuadPart = liDistanceToMove;
	if (::SetFilePointerEx(hFile, li1, &li2, dwMoveMethod) == FALSE)
		return FALSE;

	if (lpNewFilePointer) *lpNewFilePointer = li2.QuadPart;
	return TRUE;
}

BOOL CheckFileExistence(LPCTSTR lpszFilePath)
{
	HANDLE hFile;
	if ((hFile = ::CreateFile(lpszFilePath, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,	NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return FALSE;

	::CloseHandle(hFile);
	return TRUE;
}

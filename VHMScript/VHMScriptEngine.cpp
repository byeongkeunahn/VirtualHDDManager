
#include "stdafx.h"
#include "VHMCommon/VHMCommonInc.h"
#include "VHMScriptBase.h"
#include "VHMScriptEngine.h"

CVHMScriptEngine::CVHMScriptEngine()
{
}

CVHMScriptEngine::~CVHMScriptEngine()
{
}

int CVHMScriptEngine::Create()
{
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptEngine::Destroy()
{
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptEngine::RunScript(BYTE *pScript, vhmsize_t szScript)
{
	if (!pScript || szScript == 0)
		return VHM_ERROR_INVALID_PARAMETER;

	int err_code;

	vhmsize_t szConverted;
	vhmsize_t ccConverted;
	BYTE *pScriptBufferByte;
	WCHAR *pwszScriptBuffer;
	CStreamMemDynTmpl<WCHAR> smd_prep_1;

	pScriptBufferByte = nullptr;
	pwszScriptBuffer = nullptr;

	/* Convert to Unicode */

	szConverted = 0;
	ConvertToUnicode(pScript, szScript, nullptr, &szConverted);
	pScriptBufferByte = new BYTE[szConverted];
	
	err_code = ConvertToUnicode(pScript, szScript, pScriptBufferByte, &szConverted);
	VHM_ECHECK(err_code);

	ccConverted = szConverted / 2;
	pwszScriptBuffer = (WCHAR *)pScriptBufferByte;

	/* Tokenize script */

	/* Preprocess script */
	smd_prep_1.Create();
	

	/* Run Script */

	

cleanup:

	if (pScriptBufferByte) delete[] pScriptBufferByte;

	return VHM_ERROR_SUCCESS;
}

int CVHMScriptEngine::RunScript(LPCTSTR lpszPath)
{
	int err_code;

	HANDLE hFile;
	vhmsize_t szFileLength;
	BYTE *pFileBuffer;

	pFileBuffer = nullptr;

	// 스크립트 파일을 메모리에 적재
	hFile = ::CreateFile(lpszPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE)
		return VHM_ERROR_INVALID;

	szFileLength = VHMGetFileSize64(hFile);
	if (!szFileLength)
		VHM_ERROR(VHM_ERROR_INVALID);

	pFileBuffer = new BYTE[szFileLength];
	::VHMSetFilePointer64(hFile, 0, NULL, FILE_BEGIN);
	if (VHMReadFile(hFile, pFileBuffer, (DWORD)szFileLength) != szFileLength)
		VHM_ERROR(VHM_ERROR_INVALID);

	::CloseHandle(hFile);

	// RunScript 호출
	err_code = RunScript(pFileBuffer, szFileLength);

cleanup:

	if (pFileBuffer) delete[] pFileBuffer;
	return err_code;
}

int CVHMScriptEngine::ThrowException()
{
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptEngine::Tokenize()
{
	return VHM_ERROR_SUCCESS;
}
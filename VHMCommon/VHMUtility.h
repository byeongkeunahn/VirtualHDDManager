
/* VHMUtility.h */

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

/* VHM Error Message Functions */
VHMDLL const TCHAR* VHMErrorCodeToMessage(int err_code);

/* CRC Calculation Functions */
VHMDLL void CRC32GenerateTable(DWORD dwPolynomial);
VHMDLL DWORD CRC32Calc(const BYTE *pInput, SQWORD szInput, DWORD dwCRC = 0);

/* Mathematical Functions */

// returns TRUE if qwNumber is multiple of qwBase
VHMDLL BOOL IsMultipleOf(QWORD qwNumber, QWORD qwBase);

/* Compare UUIDs (returns TRUE if two inputs are the same) */
VHMDLL BOOL CompareUuid(VHMUUID uuid1, VHMUUID uuid2);

/* Memory Management Functions  */
VHMDLL void CopyMemoryBlock(void *pDest, const void *pSource, vhmsize_t uiLength);

/* String Functions */
VHMDLL WCHAR ConvToUpperChar(WCHAR wChar);
VHMDLL WCHAR ConvToLowerChar(WCHAR wChar);
VHMDLL void ConvToUpper(WCHAR *pwString);
VHMDLL void ConvToLower(WCHAR *pwString);
VHMDLL INT64 CompareStr(const WCHAR *pwString1, const WCHAR *pwString2, BOOL bCaseSensitive = TRUE);
VHMDLL INT64 FindChar(const WCHAR *pwString, WCHAR wCharToFind);
VHMDLL INT64 FindCharLen(const WCHAR *pwString, QWORD qwStrLen, WCHAR wCharToFind);
VHMDLL void SplitInit(const WCHAR *pwPath, QWORD *pqwTempVal);
VHMDLL void SplitNext(const WCHAR *pwPath, WCHAR wChar, PCWCHAR *ppwStart, QWORD *pqwLen, QWORD *pqwTempVal); // *ppwStart에 nullptr이 리턴되면 끝
VHMDLL void SplitNextMultiple(const WCHAR *pwPath, const WCHAR *pwChar, QWORD qwCharCount,
	PCWCHAR *ppwStart, QWORD *pqwLen, QWORD *pqwTempVal); // *ppwStart에 nullptr이 리턴되면 끝

/* Text Encoding Functions */

// Convert to Unicode
VHMDLL int ConvertToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);

VHMDLL int ConvAnsiToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);
VHMDLL int ConvUtf8ToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);
VHMDLL int ConvU16BToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);
VHMDLL int ConvU16LToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);
VHMDLL int ConvU32BToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);
VHMDLL int ConvU32LToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput);

#ifdef __cplusplus
}
#endif

/* Template Functions */
#include "VHMUtility.inl"

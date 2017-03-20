
#include "stdafx.h"
#include "VHMBase.h"
#include "VHMUtility.h"

BOOL g_bCRC32TableInitialized = FALSE;
DWORD g_dwCRC32Table[256];

typedef struct VHM_ERROR_MESSAGE
{
	int err_code;
	const TCHAR *err_message;

} VHM_ERROR_MESSAGE;

#define VHM_ERR_MSG_COUNT	19

static const VHM_ERROR_MESSAGE g_vhmErrMsg[VHM_ERR_MSG_COUNT] =
{
	{ VHM_ERROR_GENERIC, _T("일반적 오류") },
	{ VHM_ERROR_SUCCESS, _T("성공") },
	{ VHM_ERROR_NOT_INITIALIZED, _T("초기화되지 않음") },
	{ VHM_ERROR_INVALID_VHMDISK, _T("올바르지 않은 VHMDisk 객체") },
	{ VHM_ERROR_ALREADY_OPENED, _T("이미 열려 있음") },
	{ VHM_ERROR_MEMORY_ALLOCATION_FAILED, _T("메모리 할당 실패") },
	{ VHM_ERROR_NOT_OPENED, _T("열려있지 않음") },
	{ VHM_ERROR_INVALID_PARAMETER, _T("올바르지 않은 매개변수") },
	{ VHM_ERROR_INVALID, _T("올바르지 않음") },
	{ VHM_ERROR_INTERNAL, _T("내부 오류") },
	{ VHM_ERROR_DISK_IO, _T("디스크 I/O 오류") },
	{ VHM_ERROR_DRIVER_NOT_FOUND, _T("(해당 항목에 대한) 드라이버를 찾을 수 없음 (파일시스템 드라이버 등)") },
	{ VHM_ERROR_UNSUPPORTED, _T("지원되지 않는 기능") },
	{ VHM_ERROR_BUFFER_TOO_SMALL, _T("버퍼 크기가 충분하지 않음") },
	{ VHM_ERROR_PERMISSION_DENIED, _T("권한 거부") },
	{ VHM_ERROR_NOT_FOUND, _T("찾을 수 없음") },
	{ VHM_ERROR_NOT_AVAILABLE, _T("사용 불가능") },
	{ VHM_ERROR_TOO_BIG, _T("너무 큼") },
	{ VHM_ERROR_END, _T("끝 (파일, 버퍼, 스택, ...)") }
};

const TCHAR* VHMErrorCodeToMessage(int err_code)
{
	for (int i = 0; i < VHM_ERR_MSG_COUNT; ++i)
	{
		if (g_vhmErrMsg[i].err_code == err_code)
			return g_vhmErrMsg[i].err_message;
	}
	return nullptr;
}

void CRC32GenerateTable(DWORD dwPolynomial)
{
	DWORD i, j, k;

	for (i = 0; i < 256; ++i)
	{
		k = i;
		for (j = 0; j < 8; ++j)
		{
			if (k & 1) k = (k >> 1) ^ dwPolynomial;
			else k >>= 1;
		}
		g_dwCRC32Table[i] = k;
	}
}

DWORD CRC32Calc(const BYTE *pInput, SQWORD szInput, DWORD dwCRC)
{
	if (!g_bCRC32TableInitialized)
		CRC32GenerateTable(0xEDB88320);

	dwCRC = ~dwCRC;

	while (szInput--)
		dwCRC = g_dwCRC32Table[(dwCRC ^ *(pInput++)) & 0xFF] ^ (dwCRC >> 8);

	return ~dwCRC;
}

BOOL IsMultipleOf(QWORD qwNumber, QWORD qwBase)
{
	if (!qwBase)
		return FALSE;

	if (!qwNumber)
		return TRUE;

	QWORD qwTemp;
	qwTemp = qwNumber / qwBase;
	qwTemp *= qwBase;
	if (qwTemp == qwNumber)
		return TRUE;

	return FALSE;
}

BOOL CompareUuid(VHMUUID uuid1, VHMUUID uuid2)
{
	QWORD *pqwUuid1 = (QWORD *)(&uuid1);
	QWORD *pqwUuid2 = (QWORD *)(&uuid2);
	if (pqwUuid1[0] == pqwUuid2[0] && pqwUuid1[1] == pqwUuid2[1])
		return TRUE;

	return FALSE;
}

void CopyMemoryBlock(void *pDest, const void *pSource, vhmsize_t uiLength)
{
	const BYTE *src = (const BYTE *)pSource;
	BYTE *dst = (BYTE *)pDest;

	while (uiLength != 0)
	{
		*dst = *src;
		src++;
		dst++;
		uiLength--;
	}
}

WCHAR ConvToUpperChar(WCHAR wChar)
{
	return ((wChar >= L'a' && wChar <= L'z') ? (wChar - L'a' + L'A') : wChar);
}

WCHAR ConvToLowerChar(WCHAR wChar)
{
	return ((wChar >= L'A' && wChar <= L'Z') ? (wChar - L'A' + L'a') : wChar);
}

void ConvToUpper(WCHAR *pwString)
{
	for (;;)
	{
		if (*pwString == L'\0')
			break;

		// Convert to uppercase
		if (*pwString >= L'a' && *pwString <= L'z')
			*pwString += L'A' - L'a';
	}
}

void ConvToLower(WCHAR *pwString)
{
	for (;;)
	{
		if (*pwString == L'\0')
			break;

		// Convert to lowercase
		if (*pwString >= L'A' && *pwString <= L'Z')
			*pwString += L'a' - L'A';
	}
}

INT64 CompareStr(const WCHAR *pwString1, const WCHAR *pwString2, BOOL bCaseSensitive)
{
	if (bCaseSensitive)
	{
		while (ConvToUpperChar(*pwString1) == ConvToUpperChar(*pwString2) && *pwString1 && *pwString2)
			++pwString1, ++pwString2;
	}
	else
	{
		while (*pwString1 == *pwString2 && *pwString1 && *pwString2)
			++pwString1, ++pwString2;
	}

	return (INT64)(*pwString1 - *pwString2);
}

INT64 FindChar(const WCHAR *pwString, WCHAR wCharToFind)
{
	return FindCharLen(pwString, wcslen(pwString), wCharToFind);
}

INT64 FindCharLen(const WCHAR *pwString, QWORD qwStrLen, WCHAR wCharToFind)
{
	if (!pwString)
		return -1;

	for (QWORD i = 0; i < qwStrLen; ++i)
	{
		if (*pwString == wCharToFind)
			return i;

		pwString++;
	}

	return -1;
}

void SplitInit(const WCHAR *pwPath, QWORD *pqwTempVal)
{
	if (!pwPath || !pqwTempVal)
		return;

	*pqwTempVal = 0;
}

void SplitNext(const WCHAR *pwPath, WCHAR wChar, PCWCHAR *ppwStart, QWORD *pqwLen, QWORD *pqwTempVal)
{
	if (!pwPath || !ppwStart || !pqwLen || !pqwTempVal)
		return;

	QWORD qwPosition = *pqwTempVal;
	QWORD qwLength;

	const WCHAR *pwCurPos = pwPath + qwPosition;

	// wChar의 중복을 삭제
	for (;;)
	{
		if (*pwCurPos == wChar)
		{
			// 중복되는 wChar 발견
			pwCurPos++;
			qwPosition++;
			continue;
		}
		else if (*pwCurPos == L'\0')
		{
			// 문자열을 모두 검색했으나 wChar 이외의 문자가 발견되지 않음
			*ppwStart = pwPath + qwPosition;
			*pqwLen = 0;
			*pqwTempVal = qwPosition;
			return;
		}
		else break;
	}

	// 새로운 wChar 검색
	for (qwLength = 0;;)
	{
		if (*pwCurPos == wChar || *pwCurPos == L'\0')
			break;

		pwCurPos++;
		qwLength++;
	}

	*ppwStart = pwPath + qwPosition;
	*pqwLen = qwLength;
	*pqwTempVal = qwPosition + qwLength;
}

void SplitNextMultiple(const WCHAR *pwPath, const WCHAR *pwChar, QWORD qwCharCount,
	PCWCHAR *ppwStart, QWORD *pqwLen, QWORD *pqwTempVal)
{
	if (!pwPath || !pwChar || qwCharCount <= 0 || !ppwStart || !pqwLen || !pqwTempVal)
		return;

	QWORD qwPosition = *pqwTempVal;
	QWORD qwLength;

	const WCHAR *pwCurPos = pwPath + qwPosition;

	// wChar의 중복을 삭제
	for (;;)
	{
		if (FindCharLen(pwChar, qwCharCount, *pwCurPos) >= 0)
		{
			// 중복되는 wChar 발견
			pwCurPos++;
			qwPosition++;
			continue;
		}
		else if (*pwCurPos == L'\0')
		{
			// 문자열을 모두 검색했으나 wChar 이외의 문자가 발견되지 않음
			*ppwStart = pwPath + qwPosition;
			*pqwLen = 0;
			*pqwTempVal = qwPosition;
			return;
		}
		else break;
	}

	// 새로운 wChar 검색
	for (qwLength = 0;;)
	{
		if (FindCharLen(pwChar, qwCharCount, *pwCurPos) >= 0 || *pwCurPos == L'\0')
			break;

		pwCurPos++;
		qwLength++;
	}

	*ppwStart = pwPath + qwPosition;
	*pqwLen = qwLength;
	*pqwTempVal = qwPosition + qwLength;
}

int ConvertToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	if (!pInput)
		return VHM_ERROR_INVALID_PARAMETER;

	if (!szInput)
		return VHM_ERROR_SUCCESS;

	BYTE *pBInput;
	ENCODING eEnc;

	pBInput = (BYTE *)pInput;

	// 인코딩 판별
	if (szInput >= 4 && pBInput[0] == 0x00 && pBInput[1] == 0x00 &&
		pBInput[2] == 0xFE && pBInput[3] == 0xFF)
		eEnc = ENC_U32B, pBInput += 4, szInput -= 4;
	else if (szInput >= 4 && pBInput[0] == 0xFF && pBInput[1] == 0xFE &&
		pBInput[2] == 0x00 && pBInput[3] == 0x00)
		eEnc = ENC_U32L, pBInput += 4, szInput -= 4;
	else if (szInput >= 3 && pBInput[0] == 0xEF && pBInput[1] == 0xBB && pBInput[2] == 0xBF)
		eEnc = ENC_UTF8, pBInput += 3, szInput -= 3;
	else if (szInput >= 2 && pBInput[0] == 0xFE && pBInput[1] == 0xFF)
		eEnc = ENC_U16B, pBInput += 2, szInput -= 3;
	else if (szInput >= 2 && pBInput[0] == 0xFF && pBInput[1] == 0xFE)
		eEnc = ENC_U16L, pBInput += 2, szInput -= 2;
	else
		eEnc = ENC_ANSI;

	// 해당 인코딩을 지원하는 함수 호출
	switch (eEnc)
	{
	case ENC_ANSI: return ConvAnsiToUnicode(pBInput, szInput, pOutput, pszOutput);
	case ENC_UTF8: return ConvUtf8ToUnicode(pBInput, szInput, pOutput, pszOutput);
	case ENC_U16B: return ConvU16BToUnicode(pBInput, szInput, pOutput, pszOutput);
	case ENC_U16L: return ConvU16LToUnicode(pBInput, szInput, pOutput, pszOutput);
	case ENC_U32B: return ConvU32BToUnicode(pBInput, szInput, pOutput, pszOutput);
	case ENC_U32L: return ConvU32LToUnicode(pBInput, szInput, pOutput, pszOutput);
	}

	return VHM_ERROR_UNSUPPORTED;
}

int ConvAnsiToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	int ccRequired;
	vhmsize_t szRequired;
	const char *pszInput;

	// 버퍼 크기 확인
	pszInput = (const char *)pInput;
	ccRequired = MultiByteToWideChar(CP_ACP, 0, pszInput, (int)szInput, nullptr, 0);
	szRequired = ccRequired * 2;

	if (!pOutput || *pszOutput < szRequired)
	{
		*pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// ANSI를 Unicode(native)로 변환
	if (!MultiByteToWideChar(CP_ACP, 0, pszInput, (int)szInput, (LPWSTR)pOutput, ccRequired))
		return VHM_ERROR_GENERIC;

	return VHM_ERROR_SUCCESS;
}

int ConvUtf8ToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	BYTE *pBInput;
	BYTE *pBInputCur;
	WORD *pWOutput;
	BYTE bCur;
	vhmsize_t szRequired;
	vhmsize_t szInputRemaining;
	vhmsize_t i;

	pBInput = (BYTE *)pInput;
	szRequired = 0;

	// 필요한 크기 계산
	for (i = 0; i < szInput;)
	{
		pBInputCur = pBInput + i;
		bCur = *(pBInput + i);
		szInputRemaining = szInput - i;

		if ((bCur & 0xE0) == 0xE0)
		{
			// 3 byte -> 16 bits
			if (szInputRemaining < 3) break;
			i += 3;
			szRequired += 2;
		}
		else if ((bCur & 0xC0) == 0xC0)
		{
			// 2 byte -> 11 bits
			if (szInputRemaining < 2) break;
			i += 2;
			szRequired += 2;
		}
		else if ((bCur & 0x80) == 0x00)
		{
			// 1 byte -> 7 bits
			i += 1;
			szRequired += 2;
		}
		else if ((bCur & 0xF0) == 0xF0)
		{
			// (code length) > 16 bits : unsupported
			i++;
		}
		else
		{
			// invalid UTF-8 byte
			i++;
		}
	}

	szRequired += 2; // 2: NULL-terminator

	// 버퍼 크기 확인
	if (!pOutput || *pszOutput < szRequired)
	{
		if (!pOutput) *pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// UTF-8을 Unicode(native)로 변환
	pWOutput = (WORD *)pOutput;
	for (i = 0; i < szInput;)
	{
		pBInputCur = pBInput + i;
		bCur = *(pBInput + i);
		szInputRemaining = szInput - i;

		if ((bCur & 0xE0) == 0xE0)
		{
			// 3 byte -> 16 bits
			*pWOutput++ = (WORD)(*pBInputCur & 0x0F) << 12 | (WORD)(*(pBInputCur + 1) & 0x3F) << 6
				| (WORD)(*(pBInputCur + 2) & 0x3F);
			i += 3;
		}
		else if ((bCur & 0xC0) == 0xC0)
		{
			// 2 byte -> 11 bits
			*pWOutput++ = (WORD)(*pBInputCur & 0x1F) << 6 | (WORD)(*(pBInputCur + 1) & 0x3F);
			i += 2;
		}
		else if ((bCur & 0x80) == 0x00)
		{
			// 1 byte -> 7 bits
			*pWOutput++ = (WORD)(*pBInputCur & 0x7F);
			i += 1;
		}
		else if ((bCur & 0xF0) == 0xF0)
		{
			// (code length) >= 16 bits : unsupported
			i++;
		}
		else
		{
			// invalid UTF-8 byte
			i++;
		}
	}

	*pWOutput++ = 0;

	return VHM_ERROR_SUCCESS;
}

int ConvU16BToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	vhmsize_t szRequired;
	vhmsize_t ccLoop;
	vhmsize_t i;
	const UINT16 *pBInput;
	UINT16 *pBOutput;

	// 버퍼 크기 확인
	szRequired = szInput + 2; // 2: NULL-terminator
	if (!pOutput || *pszOutput < szRequired)
	{
		if (!pOutput) *pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// UTF-16 Big Endian을 Unicode(native)로 변환
	pBInput = (UINT16 *)pInput;
	pBOutput = (UINT16 *)pOutput;
	ccLoop = szInput / 2;

	for (i = 0; i < ccLoop; ++i)
		pBOutput[i] = BToN(pBInput[i]);

	pBOutput[ccLoop] = 0;

	return VHM_ERROR_SUCCESS;
}

int ConvU16LToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	vhmsize_t szRequired;
	vhmsize_t ccLoop;
	vhmsize_t i;
	const UINT16 *pBInput;
	UINT16 *pBOutput;

	// 버퍼 크기 확인
	szRequired = szInput + 2; // 2: NULL-terminator
	if (!pOutput || *pszOutput < szRequired)
	{
		if (!pOutput) *pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// UTF-16 Little Endian을 Unicode(native)로 변환
	pBInput = (UINT16 *)pInput;
	pBOutput = (UINT16 *)pOutput;
	ccLoop = szInput / 2;

	for (i = 0; i < ccLoop; ++i)
		pBOutput[i] = LToN(pBInput[i]);

	pBOutput[ccLoop] = 0;

	return VHM_ERROR_SUCCESS;
}

int ConvU32BToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	return VHM_ERROR_UNSUPPORTED;
}

int ConvU32LToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
	return VHM_ERROR_UNSUPPORTED;
}


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
	{ VHM_ERROR_GENERIC, _T("�Ϲ��� ����") },
	{ VHM_ERROR_SUCCESS, _T("����") },
	{ VHM_ERROR_NOT_INITIALIZED, _T("�ʱ�ȭ���� ����") },
	{ VHM_ERROR_INVALID_VHMDISK, _T("�ùٸ��� ���� VHMDisk ��ü") },
	{ VHM_ERROR_ALREADY_OPENED, _T("�̹� ���� ����") },
	{ VHM_ERROR_MEMORY_ALLOCATION_FAILED, _T("�޸� �Ҵ� ����") },
	{ VHM_ERROR_NOT_OPENED, _T("�������� ����") },
	{ VHM_ERROR_INVALID_PARAMETER, _T("�ùٸ��� ���� �Ű�����") },
	{ VHM_ERROR_INVALID, _T("�ùٸ��� ����") },
	{ VHM_ERROR_INTERNAL, _T("���� ����") },
	{ VHM_ERROR_DISK_IO, _T("��ũ I/O ����") },
	{ VHM_ERROR_DRIVER_NOT_FOUND, _T("(�ش� �׸� ����) ����̹��� ã�� �� ���� (���Ͻý��� ����̹� ��)") },
	{ VHM_ERROR_UNSUPPORTED, _T("�������� �ʴ� ���") },
	{ VHM_ERROR_BUFFER_TOO_SMALL, _T("���� ũ�Ⱑ ������� ����") },
	{ VHM_ERROR_PERMISSION_DENIED, _T("���� �ź�") },
	{ VHM_ERROR_NOT_FOUND, _T("ã�� �� ����") },
	{ VHM_ERROR_NOT_AVAILABLE, _T("��� �Ұ���") },
	{ VHM_ERROR_TOO_BIG, _T("�ʹ� ŭ") },
	{ VHM_ERROR_END, _T("�� (����, ����, ����, ...)") }
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

	// wChar�� �ߺ��� ����
	for (;;)
	{
		if (*pwCurPos == wChar)
		{
			// �ߺ��Ǵ� wChar �߰�
			pwCurPos++;
			qwPosition++;
			continue;
		}
		else if (*pwCurPos == L'\0')
		{
			// ���ڿ��� ��� �˻������� wChar �̿��� ���ڰ� �߰ߵ��� ����
			*ppwStart = pwPath + qwPosition;
			*pqwLen = 0;
			*pqwTempVal = qwPosition;
			return;
		}
		else break;
	}

	// ���ο� wChar �˻�
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

	// wChar�� �ߺ��� ����
	for (;;)
	{
		if (FindCharLen(pwChar, qwCharCount, *pwCurPos) >= 0)
		{
			// �ߺ��Ǵ� wChar �߰�
			pwCurPos++;
			qwPosition++;
			continue;
		}
		else if (*pwCurPos == L'\0')
		{
			// ���ڿ��� ��� �˻������� wChar �̿��� ���ڰ� �߰ߵ��� ����
			*ppwStart = pwPath + qwPosition;
			*pqwLen = 0;
			*pqwTempVal = qwPosition;
			return;
		}
		else break;
	}

	// ���ο� wChar �˻�
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

	// ���ڵ� �Ǻ�
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

	// �ش� ���ڵ��� �����ϴ� �Լ� ȣ��
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

	// ���� ũ�� Ȯ��
	pszInput = (const char *)pInput;
	ccRequired = MultiByteToWideChar(CP_ACP, 0, pszInput, (int)szInput, nullptr, 0);
	szRequired = ccRequired * 2;

	if (!pOutput || *pszOutput < szRequired)
	{
		*pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// ANSI�� Unicode(native)�� ��ȯ
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

	// �ʿ��� ũ�� ���
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

	// ���� ũ�� Ȯ��
	if (!pOutput || *pszOutput < szRequired)
	{
		if (!pOutput) *pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// UTF-8�� Unicode(native)�� ��ȯ
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

	// ���� ũ�� Ȯ��
	szRequired = szInput + 2; // 2: NULL-terminator
	if (!pOutput || *pszOutput < szRequired)
	{
		if (!pOutput) *pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// UTF-16 Big Endian�� Unicode(native)�� ��ȯ
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

	// ���� ũ�� Ȯ��
	szRequired = szInput + 2; // 2: NULL-terminator
	if (!pOutput || *pszOutput < szRequired)
	{
		if (!pOutput) *pszOutput = szRequired;
		return VHM_ERROR_BUFFER_TOO_SMALL;
	}

	// UTF-16 Little Endian�� Unicode(native)�� ��ȯ
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

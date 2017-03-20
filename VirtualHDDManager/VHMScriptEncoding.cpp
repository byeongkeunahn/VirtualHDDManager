
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMScript.h"

int CVHMScript::ConvertToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
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
        eEnc = ENC_U32B;
    else if (szInput >= 4 && pBInput[0] == 0xFF && pBInput[1] == 0xFE &&
        pBInput[2] == 0x00 && pBInput[3] == 0x00)
        eEnc = ENC_U32L;
    else if (szInput >= 3 && pBInput[0] == 0xEF && pBInput[1] == 0xBB && pBInput[2] == 0xBF)
        eEnc = ENC_UTF8;
    else if (szInput >= 2 && pBInput[0] == 0xFE && pBInput[1] == 0xFF)
        eEnc = ENC_U16B;
    else if (szInput >= 2 && pBInput[0] == 0xFF && pBInput[1] == 0xFE)
        eEnc = ENC_U16L;
    else
        eEnc = ENC_ANSI;

    // 해당 인코딩을 지원하는 함수 호출
    switch (eEnc)
    {
    case ENC_ANSI: return ConvAnsiToUnicode(pInput, szInput, pOutput, pszOutput);
    case ENC_UTF8: return ConvUtf8ToUnicode(pInput, szInput, pOutput, pszOutput);
    case ENC_U16B: return ConvU16BToUnicode(pInput, szInput, pOutput, pszOutput);
    case ENC_U16L: return ConvU16LToUnicode(pInput, szInput, pOutput, pszOutput);
    case ENC_U32B: return ConvU32BToUnicode(pInput, szInput, pOutput, pszOutput);
    case ENC_U32L: return ConvU32LToUnicode(pInput, szInput, pOutput, pszOutput);
    }

    return VHM_ERROR_UNSUPPORTED;
}

int CVHMScript::ConvAnsiToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
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

int CVHMScript::ConvUtf8ToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
    BYTE *pBInput;
    vhmsize_t szOutput;
    vhmsize_t i;

    pBInput = (BYTE *)pInput;
    szOutput = 0;

    for (i = 3 /* skip BOM */; i < szInput;)
    {

    }


    return VHM_ERROR_UNSUPPORTED;
}

int CVHMScript::ConvU16BToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
    vhmsize_t szRequired;
    vhmsize_t ccLoop;
    vhmsize_t i;
    const UINT16 *pBInput;
    UINT16 *pBOutput;

    // 버퍼 크기 확인
    szRequired = szInput - 2;
    if (!pOutput || *pszOutput < szRequired)
    {
        *pszOutput = szRequired;
        return VHM_ERROR_BUFFER_TOO_SMALL;
    }

    // UTF-16 Big Endian을 Unicode(native)로 변환
    pBInput = (UINT16 *)((BYTE *)pInput + 2);
    pBOutput = (UINT16 *)pOutput;
    ccLoop = szRequired / 2;

    for (i = 0; i < ccLoop; ++i)
        pBOutput[i] = BToN(pBInput[i]);

    return VHM_ERROR_SUCCESS;
}

int CVHMScript::ConvU16LToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
    vhmsize_t szRequired;
    vhmsize_t ccLoop;
    vhmsize_t i;
    const UINT16 *pBInput;
    UINT16 *pBOutput;

    // 버퍼 크기 확인
    szRequired = szInput - 2;
    if (!pOutput || *pszOutput < szRequired)
    {
        *pszOutput = szRequired;
        return VHM_ERROR_BUFFER_TOO_SMALL;
    }

    // UTF-16 Little Endian을 Unicode(native)로 변환
    pBInput = (UINT16 *)((BYTE *)pInput + 2);
    pBOutput = (UINT16 *)pOutput;
    ccLoop = szRequired / 2;

    for (i = 0; i < ccLoop; ++i)
        pBOutput[i] = LToN(pBInput[i]);

    return VHM_ERROR_SUCCESS;
}

int CVHMScript::ConvU32BToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
    return VHM_ERROR_UNSUPPORTED;
}

int CVHMScript::ConvU32LToUnicode(const void *pInput, vhmsize_t szInput, void *pOutput, vhmsize_t *pszOutput)
{
    return VHM_ERROR_UNSUPPORTED;
}

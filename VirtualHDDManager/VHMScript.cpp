
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/List.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMDisk.h"
#include "VHMIOWrapper.h"
#include "VHMPartition.h"
#include "VHMFilesystem.h"
#include "VHMRoot.h"
#include "VHMScript.h"

/*
Loading a script
1) Convert to Unicode
2) Convert script to lists of commands and labels
3) Convert each command into intermediate form
*/

CVHMScript::CVHMScript()
{
    m_pScript = NULL;
    m_pVHMRoot = NULL;
}

CVHMScript::~CVHMScript()
{
}

int CVHMScript::SetVHMRoot(CVHMRoot *pVHMRoot)
{
    m_pVHMRoot = pVHMRoot;
    return VHM_ERROR_SUCCESS;
}

CVHMRoot *CVHMScript::GetVHMRoot()
{
    return m_pVHMRoot;
}

int CVHMScript::LoadScript(LPCTSTR lpszScriptFile)
{
    if (!lpszScriptFile)
        return VHM_ERROR_INVALID_PARAMETER;

    if (m_pScript)
        return VHM_ERROR_ALREADY_OPENED;

    int err_code;
    CFile file;
    BYTE *pScript;
    size_t szScript;

    pScript = NULL;

    // 스크립트 파일을 메모리에 적재

    if (file.Open(lpszScriptFile, CFile::modeRead | CFile::shareDenyWrite) == FALSE)
        return VHM_ERROR_INVALID;

    szScript = file.GetLength();
    if (!szScript)
        VHM_ERROR(VHM_ERROR_INVALID);

    if (szScript >= VHM_SCRIPT_SIZE_MAX)
        VHM_ERROR(VHM_ERROR_TOO_BIG);

    pScript = new BYTE[szScript];
    file.Seek(0, CFile::begin);
    if (file.Read(pScript, (UINT)szScript) != szScript)
        VHM_ERROR(VHM_ERROR_INVALID);

    m_pScript = pScript;
    m_szScript = szScript;

cleanup:
    if (err_code != VHM_ERROR_SUCCESS)
    {
        if (pScript) delete[] pScript;
    }


    file.Close();
    return err_code;
}

int CVHMScript::LoadScript(BYTE *pScript, size_t szScript)
{
    if (!pScript || !szScript)
        return VHM_ERROR_INVALID_PARAMETER;

    if (m_pScript)
        return VHM_ERROR_ALREADY_OPENED;

    // 메모리 상의 스크립트를 내부 버퍼로 복사
    m_pScript = new BYTE[szScript];
    CopyMemoryBlock(pScript, m_pScript, szScript);

    // 스크립트 파싱
    return ParseScript();
}

int CVHMScript::UnloadScript()
{
    if (!m_pScript)
        return VHM_ERROR_SUCCESS;

    DestroyInternalStorage();

    delete[] m_pScript;
    m_pScript = NULL;

    return VHM_ERROR_SUCCESS;
}

int CVHMScript::ExecuteScript(LPCTSTR lpszScriptFile)
{
    if (!lpszScriptFile)
        return VHM_ERROR_INVALID_PARAMETER;

    int err_code;
    int err_code_command;

    CFile file;
    size_t szFileLength;
    size_t szConverted;
    size_t ccConverted;
    BYTE *pFileBuffer;
    BYTE *pScriptBufferByte;
    WCHAR *pwszScriptBuffer;
    size_t ccProcessed;

    pFileBuffer = NULL;
    pScriptBufferByte = NULL;

    // 스크립트 파일을 메모리에 적재
    if (file.Open(lpszScriptFile, CFile::modeRead | CFile::shareDenyWrite) == FALSE)
        return VHM_ERROR_INVALID;

    szFileLength = file.GetLength();
    if (!szFileLength)
        VHM_ERROR(VHM_ERROR_INVALID);

    if (szFileLength >= VHM_SCRIPT_SIZE_MAX)
        VHM_ERROR(VHM_ERROR_TOO_BIG);

    pFileBuffer = new BYTE[szFileLength];
    file.Seek(0, CFile::begin);
    if (file.Read(pFileBuffer, (UINT)szFileLength) != szFileLength)
        VHM_ERROR(VHM_ERROR_INVALID);

    // 파일 인코딩 변환 (Unicode화)
    szConverted = 0;
    ConvertToUnicode(pFileBuffer, szFileLength, NULL, &szConverted);
    pScriptBufferByte = new BYTE[szConverted];
    VERIFY(szConverted % 2 == 0);

    ConvertToUnicode(pFileBuffer, szFileLength, pScriptBufferByte, &szConverted);
    ccConverted = szConverted / 2;
    pwszScriptBuffer = (WCHAR *)pScriptBufferByte;

    // 파싱
    ccProcessed = 0;
    while (ccProcessed < ccConverted)
    {
        err_code_command = ParseAndExecuteCommand(pwszScriptBuffer, ccConverted, &ccProcessed, m_pVHMRoot);
        if (err_code_command == VHM_ERROR_END)
            break;
    }

    // 처리 성공
    err_code = VHM_ERROR_SUCCESS;

cleanup:
    if (pFileBuffer) delete[] pFileBuffer;
    if (pScriptBufferByte) delete[] pScriptBufferByte;

    file.Close();
    return err_code;
}

int CVHMScript::ParseScript()
{
    if (!m_pScript)
        return VHM_ERROR_INVALID;

    /*
    2) Convert script to lists of commands and labels
    */


    return VHM_ERROR_SUCCESS;
}

int CVHMScript::ConvertCommand(const WCHAR *pwszData, size_t ccData, WCHAR *pwszOutput, size_t *pccOutput)
{
    return VHM_ERROR_SUCCESS;
}

int CVHMScript::ParseAndExecuteCommand(const WCHAR *pwszData, size_t ccData, size_t *pccProcessed, CVHMRoot *pVHMRoot)
{
    if (!pwszData)
        return VHM_ERROR_INVALID_PARAMETER;

    if (*pccProcessed >= ccData)
        return VHM_ERROR_END;

    int err_code;

    size_t ccProcessed;
    size_t ccProcessedThisTime;
    size_t ccRemaining;
    const WCHAR *pwszCurPos;

    size_t ccLength; // 이번 명령어의 길이
    size_t ccCommand; // 이번 명령어의 개수
    LPCWCHAR *ppwszCommandArray; // 명령 단어 배열
    size_t *pccLengthArray; // 명령 단어 길이

    ccProcessed = *pccProcessed;
    ppwszCommandArray = NULL;
    pccLengthArray = NULL;

    // 필요 없는 문자(열) 건너뜀
    SkipUnwantedCharacter(pwszData, ccData, &ccProcessed);

    if (ccProcessed == ccData)
        VHM_ERROR(VHM_ERROR_END);

    ccRemaining = ccData - ccProcessed;
    pwszCurPos = pwszData + ccProcessed;

    /* 이번 명령을 명령 단어 단위로 분할 */

    // 필요한 메모리 크기를 얻음
    GetCommandLength(pwszCurPos, ccRemaining, &ccLength);
    GetCommandWordCount(pwszCurPos, ccLength, &ccCommand);

    // 메모리 할당 및 명령 분할
    ppwszCommandArray = new LPCWCHAR[ccCommand];
    pccLengthArray = new size_t[ccCommand];
    SplitCommand(pwszCurPos, ccLength, ccCommand, ppwszCommandArray, pccLengthArray);
    ccProcessed += ccLength;

    // 명령 처리
    err_code = ExecuteCommand(ccCommand, ppwszCommandArray, pccLengthArray, pVHMRoot);

cleanup:

    if (ppwszCommandArray) delete[] ppwszCommandArray;
    if (pccLengthArray) delete[] pccLengthArray;

    *pccProcessed = ccProcessed;

    return VHM_ERROR_SUCCESS;
}

int CVHMScript::SkipUnwantedCharacter(const WCHAR *pwszData, size_t ccData, size_t *pccProcessed)
{
    size_t i;

    for (i = *pccProcessed; i < ccData;)
    {
        if (pwszData[i] == L'#')
        {
            // 한 줄 주석
            SkipLine(pwszData, ccData, &i);
            continue;
        }

        else if (pwszData[i] == L'*')
        {
            // 여러 줄 주석
            for (++i; i < ccData; ++i)
                if (pwszData[i] == L'*')
                    break;
        }
        else if (pwszData[i] > L' ')
            break;

        ++i;
    }

    *pccProcessed = i;
    return VHM_ERROR_SUCCESS;
}

int CVHMScript::SkipLine(const WCHAR *pwszData, size_t ccData, size_t *pccProcessed)
{
    size_t i;
    for (i = *pccProcessed; i < ccData; ++i)
    {
        if (pwszData[i] == L'\r')
        {
            if ((i + 1) < ccData && pwszData[i + 1] == L'\n')
                i += 1;
            break;
        }
        else if (pwszData[i] == L'\n')
            break;
    }

    *pccProcessed = i + 1;
    return VHM_ERROR_SUCCESS;
}

int CVHMScript::SkipUntilCharacter(const WCHAR *pwszData, size_t ccData, WCHAR wszChar, size_t *pccProcessed)
{
    size_t i;
    for (i = *pccProcessed; i < ccData; ++i)
    {
        if (pwszData[i] == wszChar)
            break;
    }

    *pccProcessed = i + 1;
    return VHM_ERROR_SUCCESS;
}

int CVHMScript::GetCommandLength(const WCHAR *pwszData, size_t ccData, size_t *pccLength)
{
    size_t i;
    for (i = 0; i < ccData;)
    {
        // 큰따옴표로 묶인 문자열
        if (pwszData[i] == L'\"')
        {
            i += 1;
            SkipUntilCharacter(pwszData, ccData, L'\"', &i);
            continue;
        }

        // 명령은 개행에 의해 구분됨
        if (pwszData[i] == L'\r' || pwszData[i] == L'\n')
            break;

        ++i;
    }

    *pccLength = i;
    return VHM_ERROR_SUCCESS;
}

int CVHMScript::GetCommandWordCount(const WCHAR *pwszData, size_t ccLength, size_t *pccCommand)
{
    size_t ccCommand;
    size_t i;
    BOOL bValidCharPrev;
    BOOL bValidChar;

    ccCommand = 0;
    bValidCharPrev = FALSE;
    for (i = 0; i < ccLength;)
    {
        if (pwszData[i] > L' ') bValidChar = TRUE;
        else bValidChar = FALSE;

        /* 유효하지 않은 문자(공백,탭 등)가 나오다가 유효한 문자가 나옴 */
        if (!bValidCharPrev && bValidChar) ++ccCommand;

        bValidCharPrev = bValidChar;

        /* 큰따옴표 처리 */
        if (pwszData[i] == L'\"')
        {
            i += 1;
            SkipUntilCharacter(pwszData, ccLength, L'\"', &i);
            continue;
        }

        ++i;
    }

    *pccCommand = ccCommand;

    return VHM_ERROR_SUCCESS;
}

int CVHMScript::SplitCommand(const WCHAR *pwszData, size_t ccLength, size_t ccCommand, LPCWCHAR *ppwszCommandArray, size_t *pccLengthArray)
{
    size_t ccCommandProcessed;
    size_t i;
    BOOL bValidCharPrev;
    BOOL bValidChar;

    // Ttc (This time command)
    size_t ccTtcStartPos;
    size_t ccTtcLen;
    BOOL bTtcProcessing;

    ccCommandProcessed = 0;
    bValidCharPrev = FALSE;

    bTtcProcessing = FALSE;
    for (i = 0; i < ccLength;)
    {
        if (pwszData[i] > L' ') bValidChar = TRUE;
        else bValidChar = FALSE;

        /* 유효하지 않은 문자(공백,탭 등)가 나오다가 유효한 문자가 나옴 */
        if (!bValidCharPrev && bValidChar)
        {
            ccTtcStartPos = i;
            bTtcProcessing = TRUE;
        }

        /* 유효한 문자가 나오다가 유효하지 않은 문자가 나옴 */
        if (bValidCharPrev && !bValidChar)
        {
            ccTtcLen = i - ccTtcStartPos;

            ppwszCommandArray[ccCommandProcessed] = pwszData + ccTtcStartPos;
            pccLengthArray[ccCommandProcessed] = ccTtcLen;

            ++ccCommandProcessed;
            bTtcProcessing = FALSE;

            if (ccCommandProcessed == ccCommand)
                break;
        }

        bValidCharPrev = bValidChar;

        /* 큰따옴표 처리 */
        if (pwszData[i] == L'\"')
        {
            i += 1;
            SkipUntilCharacter(pwszData, ccLength, L'\"', &i);
            continue;
        }

        ++i;
    }

    if (bTtcProcessing)
    {
        /* 끝 명령 처리 */

        ccTtcLen = i - ccTtcStartPos;

        ppwszCommandArray[ccCommandProcessed] = pwszData + ccTtcStartPos;
        pccLengthArray[ccCommandProcessed] = ccTtcLen;

        ++ccCommandProcessed;
        bTtcProcessing = FALSE;

    }

    return VHM_ERROR_SUCCESS;
}

int CVHMScript::ExecuteCommand(size_t ccCommand, LPCWCHAR *ppwszCommandArray, size_t *pccLengthArray, CVHMRoot *pVHMRoot)
{
    WCHAR *pwszArray = new WCHAR[pccLengthArray[0] + 1];
    for (size_t i = 0; i < pccLengthArray[0]; ++i)
        pwszArray[i] = ppwszCommandArray[0][i];

    pwszArray[pccLengthArray[0]] = L'\0';

    AfxMessageBox(pwszArray);

    delete[] pwszArray;

    return VHM_ERROR_SUCCESS;
}

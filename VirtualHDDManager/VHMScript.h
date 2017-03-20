
#pragma once

#define VHM_SCRIPT_SIZE_MAX		(1048576)

FWDDEF_HANDLE(LIST_HANDLE);

class CVHMRoot;

class CVHMScript
{
public:
    CVHMScript();
    virtual ~CVHMScript();

    int SetVHMRoot(CVHMRoot *pVHMRoot);
    CVHMRoot *GetVHMRoot();

    // 스크립트 로드
    int LoadScript(LPCTSTR lpszScriptFile);
    int LoadScript(BYTE *pScript, size_t szScript);
    int UnloadScript();

    int ExecuteScript(LPCTSTR lpszScriptFile);

protected:

    /* 스크립트에서 명령과 레이블을 추출 */
    int ParseScript();

    // ParseScript 내부 구현

    /* 추출된 명령을 실행 직전 포맷으로 변환 */
    int ConvertCommand(const WCHAR *pwszData, size_t ccData, WCHAR *pwszOutput, size_t *pccOutput);

    int ParseAndExecuteCommand(const WCHAR * pwszData, size_t ccData, size_t * pccProcessed, CVHMRoot * pVHMRoot);

    int SkipUnwantedCharacter(const WCHAR * pwszData, size_t ccData, size_t * pccProcessed);

    int SkipLine(const WCHAR * pwszData, size_t ccData, size_t * pccProcessed);

    int SkipUntilCharacter(const WCHAR * pwszData, size_t ccData, WCHAR wszChar, size_t * pccProcessed);

    int GetCommandLength(const WCHAR * pwszData, size_t ccData, size_t * pccLength);

    int GetCommandWordCount(const WCHAR * pwszData, size_t ccLength, size_t * pccCommand);

    int SplitCommand(const WCHAR * pwszData, size_t ccLength, size_t ccCommand, LPCWCHAR * ppwszCommandArray, size_t * pccLengthArray);

    int ExecuteCommand(size_t ccCommand, LPCWCHAR * ppwszCommandArray, size_t * pccLengthArray, CVHMRoot * pVHMRoot);

protected:

    /* 유니코드로 변환 (BOM 없는 형태로) */
    int ConvertToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);

    // ConvertToUnicode 내부 구현
    int ConvAnsiToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvUtf8ToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU16BToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU16LToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU32BToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU32LToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);

protected:

    /* 내부 데이터 저장소 관리 */

    // 저장소 생성 및 파괴
    int InitializeInternalStorage();
    int DestroyInternalStorage();
    BOOL IsInternalStorageInitialized();
    int EmptyInternalStorage();

    // 저장소 항목 삽입 및 삭제

protected:

    /**** 내부 변수 ****
    * NOTE: NULL으로 포인터의 유효성을 나타냄
    */

    BYTE *m_pScript; // 스크립트를 통째로 읽어들인 형태
    size_t m_szScript;

    BOOL m_bInternalStorageInitialized;
    LIST_HANDLE m_plCommand;
    LIST_HANDLE m_plLabel;
    LIST_HANDLE m_plError;

    CVHMRoot *m_pVHMRoot;

};

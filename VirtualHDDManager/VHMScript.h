
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

    // ��ũ��Ʈ �ε�
    int LoadScript(LPCTSTR lpszScriptFile);
    int LoadScript(BYTE *pScript, size_t szScript);
    int UnloadScript();

    int ExecuteScript(LPCTSTR lpszScriptFile);

protected:

    /* ��ũ��Ʈ���� ��ɰ� ���̺��� ���� */
    int ParseScript();

    // ParseScript ���� ����

    /* ����� ����� ���� ���� �������� ��ȯ */
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

    /* �����ڵ�� ��ȯ (BOM ���� ���·�) */
    int ConvertToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);

    // ConvertToUnicode ���� ����
    int ConvAnsiToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvUtf8ToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU16BToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU16LToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU32BToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);
    int ConvU32LToUnicode(const void *pInput, size_t szInput, void *pOutput, size_t *pszOutput);

protected:

    /* ���� ������ ����� ���� */

    // ����� ���� �� �ı�
    int InitializeInternalStorage();
    int DestroyInternalStorage();
    BOOL IsInternalStorageInitialized();
    int EmptyInternalStorage();

    // ����� �׸� ���� �� ����

protected:

    /**** ���� ���� ****
    * NOTE: NULL���� �������� ��ȿ���� ��Ÿ��
    */

    BYTE *m_pScript; // ��ũ��Ʈ�� ��°�� �о���� ����
    size_t m_szScript;

    BOOL m_bInternalStorageInitialized;
    LIST_HANDLE m_plCommand;
    LIST_HANDLE m_plLabel;
    LIST_HANDLE m_plError;

    CVHMRoot *m_pVHMRoot;

};

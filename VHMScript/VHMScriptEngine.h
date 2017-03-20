
#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMSCRIPT_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


class VHMDLL CVHMScriptEngine
{
public:
	CVHMScriptEngine();
	virtual ~CVHMScriptEngine();

	/* Run Engine */
	int Create();
	int Destroy();
	BOOL IsCreated();

	/* Control Engine Execution */
	int RunScript(BYTE *pScript, vhmsize_t szScript);
	int RunScript(LPCTSTR lpszPath);
	int ThrowException();

protected:
	int Tokenize();
	int PreprocessScript(WCHAR *pwszScript, vhmsize_t szScript, CStreamMemDynTmpl<WCHAR> *pStream);

protected:
	BOOL m_bCreated;

};

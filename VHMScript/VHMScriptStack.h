
#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMSCRIPT_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


class VHMDLL CVHMScriptStack
{
public:
	CVHMScriptStack();
	virtual ~CVHMScriptStack();

	int Create(vhmsize_t szStack);
	int Destroy();

	int GetStackSize(vhmsize_t *sz);
	int GetUsedSize(vhmsize_t *sz);
	int GetRemainingSize(vhmsize_t *sz);

	int push(BYTE *data, vhmsize_t szData);
	int pop(BYTE *data, vhmsize_t szData);

protected:
	BOOL m_bInit;
	BYTE *m_pStack;
	vhmsize_t m_StackSz;
	vhmsize_t m_StackPos;
};


#pragma once

class VMStack
{
public:
	VMStack();
	virtual ~VMStack();

	int Create(vhmsize_t szStack);
	int Destroy();

	int push(BYTE *data, vhmsize_t szData);
	int pop(BYTE *data, vhmsize_t szData);

protected:
	BOOL m_bInit;
	vhmsize_t m_szStack;
	BYTE *m_pStackMem;
	vhmsize_t m_ptrStack;
};


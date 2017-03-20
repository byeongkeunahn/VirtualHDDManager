
#pragma once

class CVHMScriptEngine;

class CVHMScriptInterface
{
public:
	CVHMScriptInterface();
	virtual ~CVHMScriptInterface();

protected:
	BOOL m_bInit;
	CVHMScriptEngine *m_pEngine;

};

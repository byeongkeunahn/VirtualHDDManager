
// VirtualHDDManager.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CVirtualHDDManagerApp:
// �� Ŭ������ ������ ���ؼ��� VirtualHDDManager.cpp�� �����Ͻʽÿ�.
//

class CVirtualHDDManagerApp : public CWinApp
{
public:
	CVirtualHDDManagerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CVirtualHDDManagerApp theApp;

#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/VHMUtility.h"
#include "VHMScriptBase.h"
#include "VHMScriptStack.h"

CVHMScriptStack::CVHMScriptStack()
{
	m_bInit = FALSE;
}

CVHMScriptStack::~CVHMScriptStack()
{
	Destroy();
}

int CVHMScriptStack::Create(vhmsize_t szStack)
{
	if (!szStack)
		return VHM_ERROR_INVALID_PARAMETER;
	
	if (m_bInit)
		return VHM_ERROR_ALREADY_OPENED;

	m_pStack = (BYTE *)malloc(szStack);
	m_StackSz = szStack;
	m_StackPos = szStack;

	m_bInit = TRUE;

	return VHM_ERROR_SUCCESS;
}

int CVHMScriptStack::Destroy()
{
	if (!m_bInit)
		return VHM_ERROR_SUCCESS;

	m_bInit = FALSE;

	free(m_pStack);
	
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptStack::GetStackSize(vhmsize_t *sz)
{
	if (!m_bInit)
		return VHM_ERROR_NOT_INITIALIZED;

	if (!sz)
		return VHM_ERROR_INVALID_PARAMETER;

	*sz = m_StackSz;
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptStack::GetUsedSize(vhmsize_t *sz)
{
	if (!m_bInit)
		return VHM_ERROR_NOT_INITIALIZED;

	if (!sz)
		return VHM_ERROR_INVALID_PARAMETER;

	*sz = m_StackSz - m_StackPos;
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptStack::GetRemainingSize(vhmsize_t *sz)
{
	if (!m_bInit)
		return VHM_ERROR_NOT_INITIALIZED;

	if (!sz)
		return VHM_ERROR_INVALID_PARAMETER;

	*sz = m_StackPos;
	return VHM_ERROR_SUCCESS;
}

int CVHMScriptStack::push(BYTE *data, vhmsize_t szData)
{
	if (!m_bInit)
		return VHM_ERROR_NOT_INITIALIZED;

	if (!data || !szData)
		return VHM_ERROR_INVALID_PARAMETER;

	vhmsize_t pos;
	vhmsize_t i;

	GetRemainingSize(&pos);

	if (pos < szData)
		return VHM_ERROR_END;

	// data[szData - 1] 부터 push
	for (i = 0; i < szData; ++i)
		m_pStack[pos - szData + i] = data[i];

	// 새로운 스택 위치 저장
	pos = pos - szData;
	m_StackPos = pos;

	return VHM_ERROR_SUCCESS;
}

int CVHMScriptStack::pop(BYTE *data, vhmsize_t szData)
{
	if (!m_bInit)
		return VHM_ERROR_NOT_INITIALIZED;

	if (!data || !szData)
		return VHM_ERROR_INVALID_PARAMETER;

	vhmsize_t used;
	vhmsize_t pos;
	vhmsize_t i;

	GetUsedSize(&used);
	GetRemainingSize(&pos);

	if (used < szData)
		return VHM_ERROR_END;

	// data[0] 부터 pop
	for (i = 0; i < szData; ++i)
		data[i] = m_pStack[pos + i];

	// 새로운 스택 위치 저장
	used = used - szData;
	m_StackPos = m_StackSz - used;

	return VHM_ERROR_SUCCESS;
}

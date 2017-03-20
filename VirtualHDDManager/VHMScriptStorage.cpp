
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/List.h"
#include "VHMScript.h"

int CVHMScript::InitializeInternalStorage()
{
	if (IsInternalStorageInitialized())
		return VHM_ERROR_ALREADY_OPENED;

	ListCreate(&m_plCommand);
	ListCreate(&m_plLabel);
	ListCreate(&m_plError);

	return VHM_ERROR_SUCCESS;
}

int CVHMScript::DestroyInternalStorage()
{
	if (!IsInternalStorageInitialized())
		return VHM_ERROR_SUCCESS;

	ListDestroy(m_plCommand);
	ListDestroy(m_plLabel);
	ListDestroy(m_plError);

	return VHM_ERROR_SUCCESS;
}

int CVHMScript::EmptyInternalStorage()
{
	int err_code;
	if (IsInternalStorageInitialized())
	{
		err_code = DestroyInternalStorage();
		if (err_code != VHM_ERROR_SUCCESS)
			return err_code;
	}

	return InitializeInternalStorage();
}

BOOL CVHMScript::IsInternalStorageInitialized()
{
	return m_bInternalStorageInitialized;
}

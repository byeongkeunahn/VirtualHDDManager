
#include "stdafx.h"
#include "VHMCommon/VHMBase.h"
#include "VHMCommon/VHMUtility.h"
#include "VMBase.h"
#include "VMStack.h"

VMStack::VMStack()
{
}

VMStack::~VMStack()
{
}

int VMStack::Create(vhmsize_t szStack)
{
	return VHM_ERROR_SUCCESS;
}

int VMStack::Destroy()
{
	return VHM_ERROR_SUCCESS;
}

int VMStack::push(BYTE *data, vhmsize_t szData)
{
	return VHM_ERROR_SUCCESS;
}

int VMStack::pop(BYTE *data, vhmsize_t szData)
{
	return VHM_ERROR_SUCCESS;
}

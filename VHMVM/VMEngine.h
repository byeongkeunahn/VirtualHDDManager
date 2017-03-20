
#pragma once

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMVM_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif


class VHMDLL VMEngine
{
public:
	VMEngine();
	virtual ~VMEngine();
};


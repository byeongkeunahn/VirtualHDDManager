
#pragma once

/* VHM Virtual Machine (VHMVM) Library */

#ifdef VHMDLL
#undef VHMDLL
#endif

#ifdef _DLL_VHMVM_
#define VHMDLL __declspec(dllexport)
#else
#define VHMDLL __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

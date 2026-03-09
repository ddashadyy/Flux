#pragma once


#ifdef FL_PLATFORM_WINDOWS
	#ifdef FL_BUILD_DLL
		#define FLUX_API __declspec(dllexport)
	#else
		#define FLUX_API __declspec(dllimport)
	#endif 
#else 
	#error Flux only supports Windows!
#endif 

#define BIT(x) (1 << x)
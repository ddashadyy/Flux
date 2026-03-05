#pragma once


#ifdef FL_PLATFORM_WINDOWS
	#ifdef FL_BUILD_DLL
		#define FLUX_API __declspec(dllexport)
	#else
		#define FLUX_API __declspec(dllimport)
	#endif 
#endif 
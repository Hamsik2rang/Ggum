#pragma once

#if !defined(_WIN32) && !defined(_WIN64)
	#error GG Only Can support Windows!
#endif



// System�� lib�� �̾Ƴ��Ƿ� dll�� �̿����� �ʽ��ϴ�.
#ifdef GG_ENGINE
#define GG_API __declspec(dllexport)
#else
	#ifdef GG_CLIENT
		#define GG_API __declspec(dllimport)
	#endif
#endif

#include <cstdint>


typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
#if ((ULONG_MAX) == (UINT_MAX))
typedef long int32;
typedef unsigned long uint32;
#endif

typedef long long int64;
typedef unsigned long long uint64;

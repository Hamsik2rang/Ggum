#pragma once

#if !defined(_WIN32) && !defined(_WIN64)
	#error GG Only Can support Windows!
#endif



// System은 lib을 뽑아내므로 dll을 이용하지 않습니다.
#ifdef GG_ENGINE
#define GG_API __declspec(dllexport)
#else
	#ifdef GG_CLIENT
		#define GG_API __declspec(dllimport)
	#endif
#endif

#include <cstdint>
#include <cassert>


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

// BIT(x) = 2^x
#define BIT_INT8(x)		(static_cast<int8>(1) << (x))
#define BIT_UINT8(x)	(static_cast<uint8>(1) << (x))
#define BIT_INT16(x)	(static_cast<int16>(1) << (x))
#define BIT_UINT16(x)	(static_cast<uint16>(1u) << (x))
#define BIT_INT32(x)	(static_cast<int32>(1) << (x))
#define BIT_UINT32(x)	(static_cast<uint32>(1u) << (x))
#define BIT_INT64(x)	(static_cast<int64>(1) << (x))
#define BIT_UINT64(x)	(static_cast<uint64>(1u) << (x))

#define BIT(x)			BIT_INT32(x)
	
#ifdef _DEBUG
	#ifndef GG_ASSERTION_ENABLED
		#define GG_ASSERTION_ENABLED
	#endif
#else
	#ifdef GG_ASSERTION_ENABLED
		#undef GG_ASSERTION_ENABLED
	#endif
#endif

#ifdef GG_ASSERTION_ENABLED
#define GG_ASSERT(x, ...) do { if(!(x)) { GG_CRITICAL("Assertion failed: {0} in {1}(Line: {2})", __VA_ARGS__, __FILE__, __LINE__); __debugbreak(); } } while(0);
#define GG_ASSERT_WITHOUG_MESSAGE(x) do { if(!(x)) { GG_CRITICAL("Assertion faled: {0} (Line: {1})", __FILE__, __LINE__); __debugbreak(); } } while(0);
#define GG_NEVER_HAPPEN static_assert(false);

#define GG_ASSERT_WITH_ERROR_CODE(x) \
	do\
	{\
		GG_CRITICAL("Assertion failed: ErrorCode{0} in {1}(line: {2})", GetLastError(), __FILE__, __LINE__);\
		__debugbreak();\
	} while (0);

#else
#define GG_ASSERT(x, ...)
#define GG_ASSERT_WITHOUT_MESSAEG(x) 
#define GG_NEVER_HAPPEN	__assume(0);
#define GG_ASSERT_WITH_ERROR_CODE

#endif
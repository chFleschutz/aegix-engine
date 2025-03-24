#pragma once

#include "core/logging.h"

#include <cstdlib>

#ifdef _MSC_VER
#define AGX_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define AGX_DEBUG_BREAK() __builtin_trap()
#else
#define AGX_DEBUG_BREAK() std::abort()
#endif

#ifndef NDEBUG
#define AGX_ASSERTS_ENABLED 
#endif // !NDEBUG

#ifdef AGX_ASSERTS_ENABLED
#define AGX_ASSERT(expr)					\
	((expr) ? static_cast<void>(0) :		\
	(Aegix::assertFailed(#expr, "", __FILE__, __LINE__, __FUNCTION__), AGX_DEBUG_BREAK()))
#define AGX_ASSERT_X(expr, msg)				\
	((expr) ? static_cast<void>(0) :		\
	(Aegix::assertFailed(#expr, msg, __FILE__, __LINE__, __FUNCTION__), AGX_DEBUG_BREAK()))
#else
#define AGX_ASSERT(expr) static_cast<void>(0)
#define AGX_ASSERT_X(expr, msg) static_cast<void>(0)
#endif // AGX_ASSERTS_ENABLED

namespace Aegix
{
	inline void assertFailed(const char* expr, const char* what, const char* file, int line, const char* function)
	{
		ALOG::fatal("Assertion failed: '{}'\n\tFile: {}, Line: {}, Function: {}\n\t{}",
			expr, file, line, function, what);
	}
}

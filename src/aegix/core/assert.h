#pragma once

#include "core/logging.h"

#include <cassert>
#include <cstdlib>


#ifdef _MSC_VER
#define AGX_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define AGX_DEBUG_BREAK() __builtin_trap()
#else
#define AGX_DEBUG_BREAK() std::abort()
#endif


#ifndef NDEBUG
#define AGX_ENABLE_ASSERTS
#endif // !NDEBUG


#ifdef AGX_ENABLE_ASSERTS
#define AGX_ASSERT(expr, msg)									\
		if (expr) {}											\
		else													\
		{														\
			ALOG::fatal("Assertion failed: {}\n\tFile: {}, Line: {}, Function: {}\n\tExpression '{}' is false", \
				msg, __FILE__, __LINE__, __FUNCTION__, #expr);	\
			AGX_DEBUG_BREAK();									\
		}
#else
#define AGX_ASSERT(expr, msg) ((void)0)
#endif // AGX_ENABLE_ASSERTS

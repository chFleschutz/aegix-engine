#pragma once

#include <cstdint>
#include <limits>

namespace Aegix::Graphics
{
	constexpr uint32_t DEFAULT_WIDTH = 1920;
	constexpr uint32_t DEFAULT_HEIGHT = 1080;
	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
	constexpr bool ENABLE_VALIDATION = false;
#else
	constexpr bool ENABLE_VALIDATION = true;
#endif

	constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max();

	constexpr uint32_t MAX_POINT_LIGHTS = 128;
}
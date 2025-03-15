#pragma once

#include <cstdint>
#include <limits>

namespace Aegix::Core
{
	constexpr uint32_t DEFAULT_WIDTH = 1920;
	constexpr uint32_t DEFAULT_HEIGHT = 1080;
	
	constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max();
}

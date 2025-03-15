#pragma once

#include <cstdint>
#include <limits>
#include <chrono>

namespace Aegix::Core
{
	constexpr uint32_t DEFAULT_WIDTH{ 1920 };
	constexpr uint32_t DEFAULT_HEIGHT{ 1080 };

	constexpr bool ENABLE_FPS_LIMIT{ true };
	constexpr uint32_t TARGET_FPS{ 144 };
	constexpr std::chrono::milliseconds TARGET_FRAME_TIME{ 1000 / TARGET_FPS };

	constexpr uint32_t INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };
}

#pragma once

#include "utils/math_utils.h"

namespace Aegix::Graphics
{
	struct DirectionalLight
	{
		Vector4 direction{};	// w is ignored
		Vector4 color{};		// w is intensity
	};

	struct PointLight
	{
		Vector4 position{}; // w is ignored
		Vector4 color{};	// w is intensity
	};
}

#pragma once

#include "utils/math_utils.h"

namespace Aegix::Graphics
{
	struct DirectionalLight
	{
		glm::vec4 direction{};	// w is ignored
		glm::vec4 color{};		// w is intensity
	};

	struct PointLight
	{
		glm::vec4 position{}; // w is ignored
		glm::vec4 color{};	// w is intensity
	};
}

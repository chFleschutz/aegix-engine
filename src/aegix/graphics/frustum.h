#pragma once

#include "scene/components.h"

#include <glm/glm.hpp>

namespace Aegix::Graphics
{
	struct Frustum
	{
		// Plane equation: Ax + By + Cz + D = 0  -> represented as (A, B, C, D)
		using Plane = glm::vec4;

		std::array<Plane, 6> planes;

		static auto extractFrom(const glm::mat4& viewProjection) -> Frustum
		{
			Frustum frustum{
				.planes = {
					Frustum::Plane{ viewProjection[3] + viewProjection[0] }, // Left plane
					Frustum::Plane{ viewProjection[3] - viewProjection[0] }, // Right plane
					Frustum::Plane{ viewProjection[3] + viewProjection[1] }, // Bottom plane
					Frustum::Plane{ viewProjection[3] - viewProjection[1] }, // Top plane
					Frustum::Plane{ viewProjection[2]					  }, // Near plane (Vulkan specific: Z is [0, 1])
					Frustum::Plane{ viewProjection[3] - viewProjection[2] }  // Far plane
				}
			};

			// Normalize planes
			for (auto& plane : frustum.planes)
			{
				plane /= glm::length(glm::vec3(plane));
			}

			return frustum;
		}
	};
}
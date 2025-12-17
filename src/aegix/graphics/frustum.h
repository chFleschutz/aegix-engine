#pragma once

#include "scene/components.h"
#include "math/math.h"

namespace Aegix::Graphics
{
	struct Frustum
	{
		enum Side
		{
			Left = 0,
			Right = 1,
			Bottom = 2,
			Top = 3,
			Near = 4,
			Far = 5
		};

		// Plane equation: Ax + By + Cz + D = 0  -> represented as { A, B, C, D }
		using Plane = glm::vec4;

		std::array<Plane, 6> planes;

		static auto extractFrom(const glm::mat4& viewProjection) -> Frustum
		{
			auto row0 = glm::row(viewProjection, 0);
			auto row1 = glm::row(viewProjection, 1);
			auto row2 = glm::row(viewProjection, 2);
			auto row3 = glm::row(viewProjection, 3);

			Frustum frustum{};
			frustum.planes[Left]   = row3 + row0;
			frustum.planes[Right]  = row3 - row0;
			frustum.planes[Bottom] = row3 + row1;
			frustum.planes[Top]	   = row3 - row1;
			frustum.planes[Near]   = row2;	  // Note: Vulkan uses z depth range [0, 1]
			frustum.planes[Far]	   = row3 - row2;

			// Normalize planes
			for (auto& plane : frustum.planes)
			{
				float len = glm::length(glm::vec3(plane));
				AGX_ASSERT_X(len > 0.0f, "Frustum Extraction: Plane has zero length (Div by 0 hazard)");
				plane /= glm::length(glm::vec3(plane));
			}

			return frustum;
		}
	};
}
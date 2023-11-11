#pragma once

#include "scene/entity.h"
#include "utils/math_utils.h"

namespace VEAI
{
	struct Knowledge
	{
		Knowledge() = default;
		virtual ~Knowledge() = default;
	};

	struct PositionKnowledge : public Knowledge
	{
		Vector3 position;
	};

	struct EntityKnowledge : public Knowledge
	{
		VEScene::Entity entity;

		explicit EntityKnowledge(VEScene::Entity entity) : entity(entity) {}
	};

} // namespace VEAI
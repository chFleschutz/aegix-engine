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

	struct EntityGroupKnowledge : public Knowledge
	{
		std::vector<VEScene::Entity>& entities; // TODO: ensure reference is valid (e.g. by using a shared_ptr)

		explicit EntityGroupKnowledge(std::vector<VEScene::Entity>& entities) : entities(entities) {}
	};

} // namespace VEAI
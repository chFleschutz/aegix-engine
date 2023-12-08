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

	struct BoolKnowledge : public Knowledge
	{
		bool value = false;

		BoolKnowledge() = default;
		explicit BoolKnowledge(bool boolValue) : value(boolValue) {}
	};

	struct FloatKnowledge : public Knowledge
	{
		float value = 0.0f;

		FloatKnowledge() = default;
		explicit FloatKnowledge(float floatValue) : value(floatValue) {}
	};

	struct PositionKnowledge : public Knowledge
	{
		Vector3 position;

		PositionKnowledge() = default;
		explicit PositionKnowledge(Vector3 position) : position(position) {}
	};

	struct EntityKnowledge : public Knowledge
	{
		VEScene::Entity entity;

		EntityKnowledge() = default;
		explicit EntityKnowledge(VEScene::Entity entity) : entity(entity) {}
	};

	struct EntityGroupKnowledge : public Knowledge
	{
		std::vector<VEScene::Entity>& entities; // TODO: ensure reference is valid (e.g. by using a shared_ptr or make a copy and store on blackboard <-!)

		explicit EntityGroupKnowledge(std::vector<VEScene::Entity>& entities) : entities(entities) {}
	};

	struct PathKnowledge : public Knowledge
	{
		std::vector<Vector3> path;

		PathKnowledge() = default;
		explicit PathKnowledge(std::vector<Vector3> path) : path(path) {}
	};

} // namespace VEAI
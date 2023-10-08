#pragma once

#include "model.h"

#include <glm/glm.hpp>

#include <string>
#include <memory>

namespace vre
{
	struct NameComponent
	{
		std::string Name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& name)
			: Name(name) {}
	};

	struct TransformComponent
	{
		glm::vec3 Location = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& location)
			: Location(location) {}
	};

	struct MeshComponent
	{
		std::shared_ptr<VreModel> Model;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(std::shared_ptr<VreModel> model)
			: Model(model) {}
	};

	struct PointLightComponent
	{
		float intensity = 1.0f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct CameraComponent
	{

	};

} // namespace vre
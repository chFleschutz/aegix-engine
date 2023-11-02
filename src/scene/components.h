#pragma once

#include "utils/color.h"
#include "utils/math_utils.h"
#include "renderer/model.h"
#include "renderer/camera.h"

#include <string>
#include <memory>

namespace VEScripting
{
	class ScriptComponentBase;
}

namespace VEComponents
{
	/// @brief Gives a name to the entity
	struct NameComponent
	{
		std::string name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& entityName)
			: name(entityName) {}
	};

	/// @brief Stores the transformation of the entity
	struct TransformComponent
	{
		Vector3 location = { 0.0f, 0.0f, 0.0f };
		Vector3 rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Vector3& entityLocation)
			: location(entityLocation) {}
	};

	/// @brief Holds a pointer to a model
	struct MeshComponent
	{
		std::shared_ptr<VEGraphics::Model> model;
		Color color;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(std::shared_ptr<VEGraphics::Model> entityModel, const Color& baseColor = Color())
			: model(entityModel), color(baseColor) {}
	};

	/// @brief Creates a light 
	struct PointLightComponent
	{
		Color color;
		float intensity = 0.2f;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent(const Color& lightColor, float lightIntensity = 0.2f)
			: color(lightColor), intensity(lightIntensity) {}
	};

	/// @brief Holds a camera to view the scene
	struct CameraComponent
	{
		VEGraphics::Camera camera{};
	};

	/// @brief Stores a custom script
	struct ScriptComponent
	{
		std::unique_ptr<VEScripting::ScriptComponentBase> script;
	};

} // namespace vre
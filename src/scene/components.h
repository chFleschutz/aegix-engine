#pragma once

#include "utils/color.h"
#include "utils/math_utils.h"
#include "renderer/model.h"
#include "renderer/camera.h"

#include <string>
#include <memory>

namespace VEScripting
{
	class ScriptBase;
}

namespace VEComponent
{
	/// @brief Gives a name to the entity
	struct Name
	{
		std::string name;

		Name() = default;
		Name(const Name&) = default;
		Name(const std::string& entityName)
			: name(entityName) {}
	};

	/// @brief Stores the transformation of the entity
	struct Transform
	{
		Vector3 location = { 0.0f, 0.0f, 0.0f };
		Vector3 rotation = { 0.0f, 0.0f, 0.0f };
		Vector3 scale = { 1.0f, 1.0f, 1.0f };

		Transform() = default;
		Transform(const Transform&) = default;
		Transform(const Vector3& entityLocation)
			: location(entityLocation) {}
	};

	/// @brief Holds a pointer to a model
	struct Mesh
	{
		std::shared_ptr<VEGraphics::Model> model;
		Color color;

		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(std::shared_ptr<VEGraphics::Model> entityModel, const Color& baseColor = Color())
			: model(entityModel), color(baseColor) {}
	};

	/// @brief Creates a light 
	struct PointLight
	{
		Color color;
		float intensity = 0.2f;

		PointLight() = default;
		PointLight(const PointLight&) = default;
		PointLight(const Color& lightColor, float lightIntensity = 0.2f)
			: color(lightColor), intensity(lightIntensity) {}
	};

	/// @brief Holds a camera to view the scene
	struct Camera
	{
		VEGraphics::Camera camera{};
	};

	/// @brief Stores a custom script
	struct Script
	{
		std::unique_ptr<VEScripting::ScriptBase> script;
	};

} // namespace vre
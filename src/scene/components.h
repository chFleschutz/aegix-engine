#pragma once

#include "graphics/camera.h"
#include "graphics/model.h"
#include "utils/color.h"
#include "utils/math_utils.h"

#include <memory>
#include <string>

namespace Aegix::Scripting
{
	class ScriptBase;
}

namespace Aegix::Component
{
	/// @brief Gives a name to the entity
	struct Name
	{
		std::string name;
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

		Vector3 forward() const { return Aegix::MathLib::forward(rotation); }
		Vector3 right() const { return Aegix::MathLib::right(rotation);  }
		Vector3 up() const { return Aegix::MathLib::up(rotation); }
	};

	/// @brief Holds a pointer to a model
	struct Mesh
	{
		std::shared_ptr<Graphics::Model> model;
	};

	/// @brief Creates a light 
	struct PointLight
	{
		Color color;
		float intensity = 0.2f;
	};

	/// @brief Holds a camera to view the scene
	struct Camera
	{
		Graphics::Camera camera{};
	};

	/// @brief Stores a custom script
	struct Script
	{
		std::unique_ptr<Aegix::Scripting::ScriptBase> script;
	};
}
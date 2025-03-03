#pragma once

#include "graphics/static_mesh.h"
#include "utils/color.h"
#include "utils/math_utils.h"

#include <memory>
#include <string>

namespace Aegix::Scripting
{
	class ScriptBase;
}

namespace Aegix
{
	/// @brief Gives a name to the entity
	struct Name
	{
		std::string name = "Entity";
	};

	/// @brief Stores the transformation of the entity
	struct Transform
	{
		glm::vec3 location = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		auto forward() const -> glm::vec3 { return Aegix::MathLib::forward(rotation); }
		auto right() const -> glm::vec3 { return Aegix::MathLib::right(rotation); }
		auto up() const -> glm::vec3 { return Aegix::MathLib::up(rotation); }
	};

	/// @brief Holds a pointer to a static mesh
	struct Mesh
	{
		std::shared_ptr<Graphics::StaticMesh> staticMesh;
	};

	struct AmbientLight
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 0.1f;
	};

	struct DirectionalLight
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 1.0f;
	};

	/// @brief Creates a light 
	struct PointLight
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 100.0f;
	};

	/// @brief Holds a camera to view the scene
	struct Camera
	{
		float fov = glm::radians(45.0f);
		float aspect = 1.0f;
		float near = 0.1f;
		float far = 100.0f;

		glm::mat4 viewMatrix = glm::mat4{ 1.0f };
		glm::mat4 inverseViewMatrix = glm::mat4{ 1.0f };
		glm::mat4 projectionMatrix = glm::mat4{ 1.0f };
	};
}
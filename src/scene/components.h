#pragma once

#include "model.h"
#include "camera.h"
#include "scene/script_component_base.h"

#include <glm/glm.hpp>

#include <string>
#include <memory>

namespace vre
{
	/// @brief Gives a name to the entity
	struct NameComponent
	{
		std::string Name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& name)
			: Name(name) {}
	};

	/// @brief Stores the transformation of the entity
	struct TransformComponent
	{
		glm::vec3 Location = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& location)
			: Location(location) {}

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	/// @brief Holds a pointer to a model
	struct MeshComponent
	{
		std::shared_ptr<VreModel> Model;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(std::shared_ptr<VreModel> model)
			: Model(model) {}
	};

	/// @brief Creates a light 
	struct PointLightComponent
	{
		glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
	};

	/// @brief Holds a camera to view the scene
	struct CameraComponent
	{
		Camera Camera{};
	};

	/// @brief Stores a custom script
	struct ScriptComponent
	{
		ScriptComponentBase* Script = nullptr;
	};

} // namespace vre
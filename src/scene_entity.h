#pragma once

#include "component.h"
#include "model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace vre
{
	struct TransformComponent
	{
		glm::vec3 location{};
		glm::vec3 rotation{};
		glm::vec3 scale{1.0f, 1.0f, 1.0f};

		// Transformation-matrix: Translate * Ry * Rx * Rz * Scale
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent
	{
		float lightIntensity = 1.0f;
	};

	class SceneEntity
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, SceneEntity>;

		static SceneEntity createEmpty();
		static SceneEntity createPointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));
		static SceneEntity createModel(std::shared_ptr<VreModel> model);

		SceneEntity(const SceneEntity&) = delete;
		SceneEntity& operator=(const SceneEntity&) = delete;
		SceneEntity(SceneEntity&&) = default;
		SceneEntity& operator=(SceneEntity&&) = default;

		id_t id() { return mId; }

		/// @brief Adds a component to the entity
		/// @tparam T Class of the Component that should be added
		/// @tparam Has to be a subclass of Component
		template<class T, class = std::enable_if_t<std::is_base_of_v<Component, T>>>
		void addComponent()
		{
			mComponents.emplace_back(std::move(T{}));
		}

		// Todo remove puplic member
		glm::vec3 color{};
		TransformComponent transform{};

		// Optional components
		std::shared_ptr<VreModel> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		SceneEntity(id_t objId) : mId{ objId } {}

		id_t mId;
		std::vector<Component> mComponents;
	};

} // namespace vre



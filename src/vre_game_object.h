#pragma once

#include "vre_model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace vre
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.0f, 1.0f, 1.0f};
		glm::vec3 rotation{};

		// Transformation-matrix: Translate * Ry * Rx * Rz * Scale
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	class VreGameObject
	{
	public:
		using id_t = unsigned int;

		static VreGameObject createGameObject()
		{
			static id_t currentId = 0;
			return VreGameObject{ currentId++ };
		}

		VreGameObject(const VreGameObject&) = delete;
		VreGameObject& operator=(const VreGameObject&) = delete;
		VreGameObject(VreGameObject&&) = default;
		VreGameObject& operator=(VreGameObject&&) = default;

		id_t id() { return mId; }

		std::shared_ptr<VreModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		VreGameObject(id_t objId) : mId{ objId } {}

		id_t mId;
	};

} // namespace vre



#pragma once

#include "component.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vre
{
	class VreCamera : public Component
	{
	public:
		using Component::Component;

		void update(float deltaSeconds) override;

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& projectionMatrix() const { return mProjectionMatrix; }
		const glm::mat4& viewMatrix() const { return mViewMatrix; }
		const glm::mat4& inverseViewMatrix() const { return mInverseViewMatrix; }
		const glm::vec3 position() const { return glm::vec3(mInverseViewMatrix[3]); }

	private:
		glm::mat4 mProjectionMatrix{ 1.0f };
		glm::mat4 mViewMatrix{1.0f};
		glm::mat4 mInverseViewMatrix{1.0f};
	};

} // namespace vre


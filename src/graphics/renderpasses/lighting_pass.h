#pragma once

#include "graphics/descriptors.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class LightingPass : public RenderPass
	{
	public:
		// TODO: Rename and remove other GlobalUbo in frame_info.h
		struct NewGlobalUbo
		{
			glm::mat4 projection{ 1.0f };
			glm::mat4 view{ 1.0f };
			glm::mat4 inverseView{ 1.0f };
			glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f }; // w is the intesity
			PointLight pointLights[GlobalLimits::MAX_LIGHTS];
			int numLights;
		};

		LightingPass() = default;
		~LightingPass() = default;

		void render(const NewFrameInfo& frameInfo) override;

	private:
		void updateGlobalUBO(const NewFrameInfo& frameInfo, const Component::Camera& camera);

		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
		std::unique_ptr<DescriptorSet> m_globalDescriptorSet;
		std::unique_ptr<UniformBuffer<GlobalUbo>> m_globalUBO;
	};
}
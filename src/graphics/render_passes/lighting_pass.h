#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	struct LightingUniforms
	{
		struct AmbientLight
		{
			glm::vec4 color{ 0.0 };
		};

		struct DirectionalLight
		{
			glm::vec4 direction{ 0.0, 0.0, 1.0, 0.0 };
			glm::vec4 color{ 0.0 };
		};

		struct PointLight
		{
			glm::vec4 position{ 0.0f };
			glm::vec4 color{ 0.0f };
		};

		glm::vec4 cameraPosition{ 0.0f };
		AmbientLight ambient{};
		DirectionalLight directional{};
		std::array<PointLight, MAX_POINT_LIGHTS> pointLights{};
		int32_t pointLightCount{ 0 };
	};


	class LightingPass : public FrameGraphRenderPass
	{
	public:
		LightingPass(VulkanDevice& device, DescriptorPool& pool);

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;

	private:
		void updateLightingUBO(const FrameInfo& frameInfo);

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;

		std::unique_ptr<Pipeline> m_pipeline;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<UniformBuffer> m_ubo;
	};
}
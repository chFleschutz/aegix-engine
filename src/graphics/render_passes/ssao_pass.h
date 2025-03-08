#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/texture.h"

namespace Aegix::Graphics
{
	struct SSAOPushConstants
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	class SSAOPass : public FrameGraphRenderPass
	{
	public:
		inline static constexpr uint32_t SAMPLE_COUNT = 64;
		inline static constexpr uint32_t NOISE_SIZE = 16;

		SSAOPass(VulkanDevice& device, DescriptorPool& pool);

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;

	private:
		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_ssao;

		std::unique_ptr<Pipeline> m_pipeline;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;

		std::unique_ptr<Buffer> m_ssaoSamples;
		std::unique_ptr<Texture> m_ssaoNoise;
	};
}
#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/resources/buffer.h"
#include "graphics/resources/texture.h"

namespace Aegix::Graphics
{
	struct SSAOUniforms
	{
		glm::mat4 view{ 1.0f };
		glm::mat4 projection{ 1.0f };
		glm::vec2 noiseScale{ 4.0f };
		float radius{ 0.5f };
		float bias{ 0.025 };
		float power{ 2.0f };
	};

	class SSAOPass : public FrameGraphRenderPass
	{
	public:
		inline static constexpr uint32_t SAMPLE_COUNT = 64;
		inline static constexpr uint32_t NOISE_SIZE = 16;

		SSAOPass();

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;
		virtual void drawUI() override;

	private:
		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_ssao;

		std::unique_ptr<Pipeline> m_pipeline;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;

		std::unique_ptr<Texture> m_ssaoNoise;
		Buffer m_ssaoSamples;
		Buffer m_uniforms;

		SSAOUniforms m_uniformData;
	};
}
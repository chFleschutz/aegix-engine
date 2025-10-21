#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	enum class ToneMappingMode
	{
		Reinhard,
		ACES
	};

	struct PostProcessingSettings
	{
		ToneMappingMode toneMappingMode = ToneMappingMode::ACES;
		float bloomIntensity = 0.1f;
		float exposure = 1.0f;
		float gamma = 2.2f;
	};

	class PostProcessingPass : public FrameGraphRenderPass
	{
	public:
		PostProcessingPass();

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void createResources(FrameGraphResourcePool& resources) override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;
		virtual void drawUI() override;

	private:
		auto createDescriptorSetLayout() -> DescriptorSetLayout;
		auto createPipeline() -> Pipeline;

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloom;
		FrameGraphResourceHandle m_final;

		PostProcessingSettings m_settings;

		DescriptorSetLayout m_descriptorSetLayout;
		std::vector<DescriptorSet> m_descriptorSets;
		Pipeline m_pipeline;
	};
}
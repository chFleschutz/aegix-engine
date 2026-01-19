#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegis::Graphics
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

	class PostProcessingPass : public FGRenderPass
	{
	public:
		PostProcessingPass(FGResourcePool& pool);

		virtual auto info() -> FGNode::Info override;
		virtual void createResources(FGResourcePool& resources) override;
		virtual void execute(FGResourcePool& resources, const FrameInfo& frameInfo) override;
		virtual void drawUI() override;

	private:
		auto createDescriptorSetLayout() -> DescriptorSetLayout;
		auto createPipeline() -> Pipeline;

		FGResourceHandle m_sceneColor;
		FGResourceHandle m_bloom;
		FGResourceHandle m_final;

		PostProcessingSettings m_settings;

		DescriptorSetLayout m_descriptorSetLayout;
		std::vector<DescriptorSet> m_descriptorSets;
		Pipeline m_pipeline;
	};
}
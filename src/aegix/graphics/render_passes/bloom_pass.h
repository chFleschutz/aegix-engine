#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/resources/image_view.h"
#include "graphics/resources/sampler.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegis::Graphics
{
	struct BloomThreshold
	{
		float threshold = 0.0f;
		float softThreshold = 0.5f;
	};

	struct BloomDownsample
	{
		int mipLevel = 0;
		float filterScale = 1.0f;
	};

	struct BloomUpsample
	{
		float filterScale = 1.0f;
	};

	/// @brief Bloom post-processing effect using a threshold, downsample, and upsample pass
	/// @note Based on: https://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare/
	class BloomPass : public FGRenderPass
	{
	public:
		static constexpr uint32_t BLOOM_MIP_LEVELS = 6;

		BloomPass(FGResourcePool& pool);

		virtual auto info() -> FGNode::Info override;
		virtual void createResources(FGResourcePool& resources) override;
		virtual void execute(FGResourcePool& resources, const FrameInfo& frameInfo) override;
		virtual void drawUI() override;

	private:
		auto createThresholdDescriptorSetLayout() -> DescriptorSetLayout;
		auto createDownsampleDescriptorSetLayout() -> DescriptorSetLayout;
		auto createUpsampleDescriptorSetLayout() -> DescriptorSetLayout;
		void extractBrightRegions(VkCommandBuffer cmd, const FrameInfo& frameInfo);
		void downSample(VkCommandBuffer cmd, Texture& bloom);
		void upSample(VkCommandBuffer cmd, Texture& bloom);

		FGResourceHandle m_sceneColor;
		FGResourceHandle m_bloom;

		std::vector<ImageView> m_mipViews;
		Sampler m_sampler;

		BloomThreshold m_threshold;
		BloomDownsample m_downsample;
		BloomUpsample m_upsample;

		DescriptorSetLayout m_thresholdSetLayout;
		DescriptorSet m_thresholdSet;
		std::unique_ptr<Pipeline> m_thresholdPipeline;

		DescriptorSetLayout m_downsampleSetLayout;
		std::vector<DescriptorSet> m_downsampleSets;
		std::unique_ptr<Pipeline> m_downsamplePipeline;

		DescriptorSetLayout m_upsampleSetLayout;
		std::vector<DescriptorSet> m_upsampleSets;
		std::unique_ptr<Pipeline> m_upsamplePipeline;
	};
}
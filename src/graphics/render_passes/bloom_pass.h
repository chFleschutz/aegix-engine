#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/texture.h"
#include "graphics/vulkan_tools.h"

#include <memory>
#include <vector>

namespace Aegix::Graphics
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
	class BloomPass : public FrameGraphRenderPass
	{
	public:
		static constexpr uint32_t BLOOM_MIP_LEVELS = 6;

		BloomPass(VulkanDevice& device, DescriptorPool& pool);
		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void createResources(FrameGraphResourcePool& resources) override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;
		virtual void drawUI() override;

	private:
		void extractBrightRegions(VkCommandBuffer cmd, const FrameInfo& frameInfo);
		void downSample(VkCommandBuffer cmd, const SampledTexture& bloom);
		void upSample(VkCommandBuffer cmd, const SampledTexture& bloom);


		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloom;

		std::vector<ImageView> m_mipViews;
		Sampler m_sampler;

		BloomThreshold m_threshold;
		BloomDownsample m_downsample;
		BloomUpsample m_upsample;

		std::unique_ptr<Pipeline> m_thresholdPipeline;
		std::unique_ptr<PipelineLayout> m_thresholdPipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_thresholdSetLayout;
		std::unique_ptr<DescriptorSet> m_thresholdSet;

		std::unique_ptr<Pipeline> m_downsamplePipeline;
		std::unique_ptr<PipelineLayout> m_downsamplePipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_downsampleSetLayout;
		std::vector<std::unique_ptr<DescriptorSet>> m_downsampleSets;

		std::unique_ptr<Pipeline> m_upsamplePipeline;
		std::unique_ptr<PipelineLayout> m_upsamplePipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_upsampleSetLayout;
		std::vector<std::unique_ptr<DescriptorSet>> m_upsampleSets;
	};
}
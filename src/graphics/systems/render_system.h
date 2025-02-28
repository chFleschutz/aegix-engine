#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/render_stage.h"
#include "graphics/frame_info.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	/// @brief Base class for all render systems
	class RenderSystem
	{
	public:
		RenderSystem(VulkanDevice& device, VkDescriptorSetLayout globalSetLayout);
		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		virtual ~RenderSystem() = default;

		DescriptorSetLayout& descriptorSetLayout() { return *m_descriptorSetLayout; }

		virtual void render(const FrameInfo& frameInfo, VkDescriptorSet globalSet) = 0;

	protected:
		VulkanDevice& m_device;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<Pipeline> m_pipeline;
	};
}

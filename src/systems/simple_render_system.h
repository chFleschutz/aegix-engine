#pragma once

#include "graphics/device.h"
#include "graphics/frame_info.h"
#include "graphics/pipeline.h"

#include <memory>
#include <vector>

namespace VEGraphics
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VulkanDevice& m_device;

		std::unique_ptr<Pipeline> mPipeline;
		VkPipelineLayout mPipelineLayout;
	};

} // namespace vre

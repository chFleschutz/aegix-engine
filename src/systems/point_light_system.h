#pragma once

#include "renderer/device.h"
#include "renderer/frame_info.h"
#include "renderer/pipeline.h"

#include <memory>
#include <vector>

namespace VEGraphics
{
	class PointLightSystem
	{
	public:
		PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VulkanDevice& m_device;

		std::unique_ptr<Pipeline> mPipeline;
		VkPipelineLayout mPipelineLayout;
	};

} // namespace vre

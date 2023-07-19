#pragma once

#include "vre_camera.h"
#include "vre_device.h"
#include "vre_frame_info.h"
#include "vre_pipeline.h"

#include <memory>
#include <vector>

namespace vre
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(VreDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VreDevice& mVreDevice;

		std::unique_ptr<VrePipeline> mVrePipeline;
		VkPipelineLayout mPipelineLayout;
	};

} // namespace vre

#pragma once

#include "vre_camera.h"
#include "vre_device.h"
#include "vre_frame_info.h"
#include "vre_pipeline.h"

#include <memory>
#include <vector>

namespace vre
{
	class PointLightSystem
	{
	public:
		PointLightSystem(VreDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VreDevice& mVreDevice;

		std::unique_ptr<VrePipeline> mVrePipeline;
		VkPipelineLayout mPipelineLayout;
	};

} // namespace vre

#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/frame_info.h"
#include "graphics/material.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	class RenderSystem
	{
	public:
		RenderSystem(VulkanDevice& device) : m_device{ device } {}
		virtual ~RenderSystem()
		{
			// TODO: Wrap this in own class
			if (m_pipelineLayout)
				vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
		}

		virtual void render(const FrameInfo& frameInfo) = 0;

	protected:
		VulkanDevice& m_device;

		std::unique_ptr<DescriptorPool> m_descriptorPool;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout = nullptr;
		std::unique_ptr<Pipeline> m_pipeline;
	};


	template<typename T>
	struct RenderSystemRef;
}

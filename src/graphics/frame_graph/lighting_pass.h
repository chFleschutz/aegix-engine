#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/uniform_buffer.h"

#include "graphics/systems/render_system.h"

namespace Aegix::Graphics
{
	class LightingPass : public FrameGraphRenderPass
	{
	public:
		LightingPass(VulkanDevice& device, DescriptorPool& pool);

		void execute(const FrameInfo& frameInfo) override;

	private:
		void updateGlobalUBO(const FrameInfo& frameInfo);

		VulkanDevice& m_device;

		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
		std::unique_ptr<DescriptorSet> m_globalDescriptorSet;
		std::unique_ptr<UniformBuffer<GlobalUbo>> m_globalUBO;

		// TODO: Temp
		std::unique_ptr<RenderSystem> m_renderSystem;
	};
}
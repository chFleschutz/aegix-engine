#pragma once

#include "graphics/descriptors.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class LightingPass : public RenderPass
	{
	public:
		LightingPass(VulkanDevice& device, DescriptorPool& pool);
		~LightingPass() = default;

		void render(FrameInfo& frameInfo) override;

	private:
		void updateGlobalUBO(const FrameInfo& frameInfo);
		void beginRenderPass(FrameInfo& frameInfo);
		void endRenderPass(FrameInfo& frameInfo);

		std::unique_ptr<DescriptorSet> m_globalDescriptorSet;
		std::unique_ptr<UniformBuffer<GlobalUbo>> m_globalUBO;
	};
}
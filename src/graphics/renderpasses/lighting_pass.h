#pragma once

#include "graphics/descriptors.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class LightingPass : public RenderPass
	{
	public:
		LightingPass(Builder& builder, DescriptorPool& pool);
		~LightingPass() = default;

	protected:
		void render(FrameInfo& frameInfo) override;

	private:
		void updateGlobalUBO(const FrameInfo& frameInfo);

		std::unique_ptr<DescriptorSet> m_globalDescriptorSet;
		std::unique_ptr<UniformBuffer<GlobalUbo>> m_globalUBO;
	};
}
#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "scene/components.h"

namespace Aegix::Graphics
{
	struct SkyBoxUniforms
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	class SkyBoxPass : public FrameGraphRenderPass
	{
	public:
		SkyBoxPass();

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo, const RenderContext& ctx) override;

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;

		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<Pipeline> m_pipeline;

		std::unique_ptr<Buffer> m_vertexBuffer;
		std::unique_ptr<Buffer> m_indexBuffer;
	};
}

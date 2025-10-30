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
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;

	private:
		auto createDescriptorSetLayout() -> DescriptorSetLayout;

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;

		DescriptorSetLayout m_descriptorSetLayout;
		std::vector<DescriptorSet> m_descriptorSets;
		std::unique_ptr<Pipeline> m_pipeline;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
	};
}

#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "scene/components.h"

namespace Aegis::Graphics
{
	struct SkyBoxUniforms
	{
		glm::mat4 viewProjection;
	};

	class SkyBoxPass : public FGRenderPass
	{
	public:
		SkyBoxPass(FGResourcePool& pool);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		auto createDescriptorSetLayout() -> DescriptorSetLayout;

		FGResourceHandle m_sceneColor;
		FGResourceHandle m_depth;

		DescriptorSetLayout m_descriptorSetLayout;
		std::vector<DescriptorSet> m_descriptorSets;
		std::unique_ptr<Pipeline> m_pipeline;

		Buffer m_vertexBuffer;
		Buffer m_indexBuffer;
	};
}

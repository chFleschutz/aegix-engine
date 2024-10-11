#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/frame_info.h"
#include "graphics/texture.h"

#include <string>
#include <vector>

namespace Aegix::Graphics
{
	enum class RessourceType
	{
		Attachment,
		SampledTexture,
		Reference
	};

	struct FrameGraphRessource
	{
		RessourceType type;
		std::string name;
		VkFormat format;
		VkExtent2D size;
		VkAttachmentLoadOp loadOp;
		VkClearValue clearValue;

		std::shared_ptr<Texture> texture;
	};

	struct FrameGraphNode
	{
		std::vector<FrameGraphRessource> inputs;
		std::vector<FrameGraphRessource> outputs;

		std::unique_ptr<FrameGraphRenderPass> renderPass;
	};

	class FrameGraph
	{
	public:
		FrameGraph();
		FrameGraph(const FrameGraph&) = delete;
		~FrameGraph() = default;

		void render(FrameInfo& frameInfo);

	private:
		std::vector<FrameGraphNode> m_nodes;
	};
}
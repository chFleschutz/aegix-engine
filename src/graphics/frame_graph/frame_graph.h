#pragma once

#include "graphics/texture.h"
#include "graphics/renderpasses/frame_graph_render_pass.h"

#include <vector>
#include <string>

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

	private:
		std::vector<FrameGraphNode> m_nodes;
	};
}
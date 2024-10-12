#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/frame_info.h"
#include "graphics/texture.h"

#include <string>
#include <vector>

namespace Aegix::Graphics
{
	enum class RessourceUsage
	{
		Write,
		Read,
		ReadWrite
	};


	class FrameGraph
	{
	public:
		struct Ressource
		{
			std::string name;
			RessourceUsage type;
			VkFormat format;
			VkExtent2D size;
			VkAttachmentLoadOp loadOp;
			VkClearValue clearValue;

			std::shared_ptr<Texture> texture;
		};


		struct FrameGraphNode
		{
			std::vector<Ressource> ressources;
			std::unique_ptr<FrameGraphRenderPass> renderPass;
		};


		FrameGraph();
		FrameGraph(const FrameGraph&) = delete;
		~FrameGraph() = default;

		void render(FrameInfo& frameInfo);

		void addNode(const FrameGraphNode& node)
		{
			m_nodes.emplace_back(node);
		}

	private:
		std::vector<FrameGraphNode> m_nodes;
	};
}
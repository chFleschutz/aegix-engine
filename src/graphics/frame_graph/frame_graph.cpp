#include "frame_graph.h"

namespace Aegix::Graphics
{
	// FrameGraph::Builder -------------------------------------------------------

	FrameGraph::Builder::Builder(FrameGraph& frameGraph, FrameGraphNode& node)
		: m_frameGraph{ frameGraph }, m_node{ node }
	{
	}

	FrameGraphResourceID FrameGraph::Builder::declareRead(FrameGraphResourceID resource)
	{
		m_node.addRead(resource);
		return resource;
	}

	FrameGraphResourceID FrameGraph::Builder::declareWrite(FrameGraphResourceID resource)
	{
		m_node.addWrite(resource);
		return resource;
	}



	// FrameGraph ----------------------------------------------------------------

	FrameGraphResourceID FrameGraph::addTexture(VulkanDevice& device, const std::string& name, const FrameGraphTexture::Desc& desc)
	{
		return m_resourcePool.addTexture(device, name, desc);
	}

	void FrameGraph::compile()
	{
		// TODO
	}

	void FrameGraph::execute(const FrameInfo& frameInfo)
	{
		for (auto& node : m_nodes)
		{
			placeBarriers(frameInfo.commandBuffer, node);

			node.executePass(m_resourcePool, frameInfo);
		}
	}

	void FrameGraph::swapChainResized()
	{
		// TODO
	}

	void FrameGraph::placeBarriers(VkCommandBuffer commandBuffer, FrameGraphNode& node)
	{
		BarrierPlacement readColor{
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		};
		BarrierPlacement readDepth{
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		};
		placeBarriers(commandBuffer, node.reads(), readColor, readDepth);

		BarrierPlacement writeColor{
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};
		BarrierPlacement writeDepth{
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			.dstStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
		};
		placeBarriers(commandBuffer, node.writes(), writeColor, writeDepth);
	}

	void FrameGraph::placeBarriers(VkCommandBuffer commandBuffer, const std::vector<FrameGraphResourceID>& resources,
		const BarrierPlacement& color, const BarrierPlacement& depth)
	{
		std::vector<VkImageMemoryBarrier> depthBarriers;
		std::vector<VkImageMemoryBarrier> colorBarriers;
		colorBarriers.reserve(resources.size());

		for (auto& resource : resources)
		{
			Texture& texture = m_resourcePool.texture(resource).texture;

			if (Tools::isDepthFormat(texture.format()))
			{
				if (texture.layout() != depth.newLayout)
					depthBarriers.emplace_back(texture.imageMemoryBarrier(depth.newLayout));
			}
			else
			{
				if (texture.layout() != color.newLayout)
					colorBarriers.emplace_back(texture.imageMemoryBarrier(color.newLayout));
			}
		}

		Tools::vk::cmdPipelineBarrier(commandBuffer, depth.srcStage, depth.dstStage, depthBarriers);
		Tools::vk::cmdPipelineBarrier(commandBuffer, color.srcStage, color.dstStage, colorBarriers);
	}
}
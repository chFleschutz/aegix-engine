#include "frame_graph.h"

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"


namespace Aegix::Graphics
{
	// FrameGraph ----------------------------------------------------------------

	void FrameGraph::compile(VulkanDevice& device)
	{
		// Resolve references
		m_resourcePool.resolveReferences();

		// Compute graph edges

		// Topological sort

		// Aliasing

		// Create resources
		m_resourcePool.createResources(device);

		// Print frame graph info
		std::cout << "FrameGraph compiled with " << m_nodes.size() << " passes\n";
		for (const auto& nodeHandle : m_nodes)
		{
			std::cout << "\t- " << m_resourcePool.node(nodeHandle).name << "\n";
		}
	}

	void FrameGraph::execute(const FrameInfo& frameInfo)
	{
		for (auto& nodeHandle : m_nodes)
		{
			auto& node = m_resourcePool.node(nodeHandle);

			placeBarriers(frameInfo.commandBuffer, node);

			node.pass->execute(m_resourcePool, frameInfo);
		}
	}

	void FrameGraph::swapChainResized(VulkanDevice& device, uint32_t width, uint32_t height)
	{
		for (auto& resource : m_resourcePool.m_resources)
		{
			if (resource.type != FrameGraphResourceType::Texture)
				continue;

			auto& info = std::get<FrameGraphResourceTextureInfo>(resource.info);
			if (info.resizePolicy == ResizePolicy::SwapchainRelative)
			{
				auto& texture = m_resourcePool.texture(resource.handle);
				texture.resize(width, height, info.usage);
				info.extent = { width, height };
			}
		}
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
		placeBarriers(commandBuffer, node.inputs, readColor, readDepth);

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
		placeBarriers(commandBuffer, node.outputs, writeColor, writeDepth);
	}

	void FrameGraph::placeBarriers(VkCommandBuffer commandBuffer, const std::vector<FrameGraphResourceHandle>& resources,
		const BarrierPlacement& color, const BarrierPlacement& depth)
	{
		std::vector<VkImageMemoryBarrier> depthBarriers;
		std::vector<VkImageMemoryBarrier> colorBarriers;
		colorBarriers.reserve(resources.size());

		for (auto& resource : resources)
		{
			Texture& texture = m_resourcePool.texture(resource);

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
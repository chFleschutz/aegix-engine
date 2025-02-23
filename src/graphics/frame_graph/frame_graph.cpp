#include "frame_graph.h"

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"


namespace Aegix::Graphics
{
	static auto imageLayoutForUsage(FrameGraphResourceUsage usage) -> VkImageLayout
	{
		switch (usage)
		{
		case FrameGraphResourceUsage::None:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case FrameGraphResourceUsage::Sampled:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case FrameGraphResourceUsage::ColorAttachment:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case FrameGraphResourceUsage::DepthStencilAttachment:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		default:
			assert(false && "Invalid usage");
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}


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
		std::vector<FrameGraphResourceHandle> resourceHandles;
		resourceHandles.reserve(node.inputs.size() + node.outputs.size());
		resourceHandles.insert(resourceHandles.end(), node.inputs.begin(), node.inputs.end());
		resourceHandles.insert(resourceHandles.end(), node.outputs.begin(), node.outputs.end());

		std::vector<VkImageMemoryBarrier> barriers;
		VkPipelineStageFlags srcStage = 0;
		VkPipelineStageFlags dstStage = 0;

		for (const auto& resourceHandle : resourceHandles)
		{
			auto& resource = m_resourcePool.resource(resourceHandle);
			auto& texture = m_resourcePool.texture(resourceHandle);

			VkImageLayout oldLayout = texture.layout();
			VkImageLayout newLayout = imageLayoutForUsage(resource.usage);
			if (oldLayout == newLayout)
				continue;

			VkImageMemoryBarrier barrier = texture.transitionLayoutDeferred(newLayout);
			srcStage |= Tools::srcStage(barrier.srcAccessMask);
			dstStage |= Tools::dstStage(barrier.dstAccessMask);
			barriers.emplace_back(barrier);
		}

		Tools::vk::cmdPipelineBarrier(commandBuffer, srcStage, dstStage, barriers);
	}
}
#include "frame_graph.h"

#include "core/logging.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

#include <cassert>

namespace std
{
	template <>
	struct hash<Aegix::Graphics::FrameGraphNodeHandle>
	{
		std::size_t operator()(const Aegix::Graphics::FrameGraphNodeHandle& v) const
		{
			return std::hash<uint32_t>()(v.id);
		}
	};
}

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
		case FrameGraphResourceUsage::Compute:
			return VK_IMAGE_LAYOUT_GENERAL;
		case FrameGraphResourceUsage::TransferSrc:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case FrameGraphResourceUsage::TransferDst:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case FrameGraphResourceUsage::Present:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		default:
			assert(false && "Undefined FrameGraphResourceUsage");
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}


	// FrameGraph ----------------------------------------------------------------

	void FrameGraph::compile(VulkanDevice& device)
	{
		m_resourcePool.resolveReferences();

		computeEdges();
		sortNodes();

		// TODO: Resource aliasing

		m_resourcePool.createResources(device);
		for (const auto& nodeHandle : m_nodeHandles)
		{
			auto& node = m_resourcePool.node(nodeHandle);
			node.pass->createResources(m_resourcePool);
		}

		// Print frame graph info
		ALOG::info("FrameGraph compiled with {} passes", m_nodeHandles.size());
		for (const auto& nodeHandle : m_nodeHandles)
		{
			ALOG::info("\t- {}", m_resourcePool.node(nodeHandle).name);
		}
	}

	void FrameGraph::execute(const FrameInfo& frameInfo)
	{
		for (const auto& nodeHandle : m_nodeHandles)
		{
			auto& node = m_resourcePool.node(nodeHandle);

			Tools::vk::cmdBeginDebugUtilsLabel(frameInfo.commandBuffer, node.name.c_str());
			placeBarriers(frameInfo.commandBuffer, node);
			node.pass->execute(m_resourcePool, frameInfo);
			Tools::vk::cmdEndDebugUtilsLabel(frameInfo.commandBuffer);
		}
	}

	void FrameGraph::swapChainResized(uint32_t width, uint32_t height)
	{
		m_resourcePool.resizeImages(width, height);

		for (const auto& nodeHandle : m_nodeHandles)
		{
			auto& node = m_resourcePool.node(nodeHandle);
			node.pass->createResources(m_resourcePool);
		}
	}

	void FrameGraph::computeEdges()
	{
		// Cache resource outputs (Possible for one resource to be modified by multiple passes)
		std::unordered_map<std::string, std::vector<FrameGraphNodeHandle>> resourceOutputs;
		for (const auto& nodeHandle : m_nodeHandles)
		{
			const auto& node = m_resourcePool.node(nodeHandle);
			for (const auto& outputHandle : node.outputs)
			{
				const auto& outputResource = m_resourcePool.resource(outputHandle);
				resourceOutputs[outputResource.name].emplace_back(nodeHandle);
			}
		}

		// Create edges
		for (const auto& nodeHandle : m_nodeHandles)
		{
			const auto& node = m_resourcePool.node(nodeHandle);
			for (const auto& inputHandle : node.inputs)
			{
				const auto& inputResource = m_resourcePool.finalResource(inputHandle);
				for (const auto& outputHandle : resourceOutputs[inputResource.name])
				{
					if (outputHandle == nodeHandle) // Avoid self connections
						continue;

					auto& edgeHandles = m_resourcePool.node(outputHandle).edges;
					if (std::find(edgeHandles.begin(), edgeHandles.end(), nodeHandle) == edgeHandles.end())
						edgeHandles.push_back(nodeHandle);
				}
			}
		}
	}

	void FrameGraph::sortNodes()
	{
		// Topological sort

		// Contains the sorted nodes in reverse order at the end
		std::vector<FrameGraphNodeHandle> sortedNodes; 
		sortedNodes.reserve(m_nodeHandles.size());

		// Stack for DFS
		std::vector<FrameGraphNodeHandle> stack; 
		stack.reserve(m_nodeHandles.size());

		// Track nodes to avoid adding duplicates
		constexpr uint8_t VISITED_ONCE = 1;
		constexpr uint8_t ALREADY_ADDED = 2;
		std::vector<uint8_t> visited(m_nodeHandles.size(), 0); 

		// Depth First Search starting at each node 
		for (const auto& nodeHandle : m_nodeHandles)
		{
			stack.emplace_back(nodeHandle);

			while (stack.size() > 0)
			{
				FrameGraphNodeHandle currentHandle = stack.back();

				if (visited[currentHandle.id] == ALREADY_ADDED)
				{
					stack.pop_back();
					continue;
				}

				if (visited[currentHandle.id] == VISITED_ONCE)
				{
					visited[currentHandle.id] = ALREADY_ADDED;
					sortedNodes.emplace_back(currentHandle);
					stack.pop_back();
					continue;
				}

				visited[currentHandle.id] = VISITED_ONCE;

				const auto& currentNode = m_resourcePool.node(currentHandle);
				for (const auto& nextNodeHandle : currentNode.edges)
				{
					if (!visited[nextNodeHandle.id])
						stack.emplace_back(nextNodeHandle);
				}
			}
		}

		assert(sortedNodes.size() == m_nodeHandles.size() && "Failed to sort nodes");
		m_nodeHandles.assign(sortedNodes.rbegin(), sortedNodes.rend());
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

			if (resource.usage == FrameGraphResourceUsage::None)
				continue;

			VkImageLayout oldLayout = texture.image().layout();
			VkImageLayout newLayout = imageLayoutForUsage(resource.usage);
			if (oldLayout == newLayout)
				continue;

			VkImageMemoryBarrier barrier = texture.image().transitionLayoutDeferred(newLayout);
			srcStage |= Tools::srcStage(barrier.srcAccessMask);
			dstStage |= Tools::dstStage(barrier.dstAccessMask);
			barriers.emplace_back(barrier);
		}

		Tools::vk::cmdPipelineBarrier(commandBuffer, srcStage, dstStage, barriers);
	}
}
#include "frame_graph.h"

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

#include <cassert>
#include <iostream>

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
		case FrameGraphResourceUsage::Present:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		default:
			assert(false && "Invalid usage");
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}


	// FrameGraph ----------------------------------------------------------------

	void FrameGraph::compile(VulkanDevice& device)
	{
		m_resourcePool.resolveReferences();

		sortNodes();

		// Aliasing
		// TODO: Replace by proper aliasing
		const auto& presentNode = m_resourcePool.node(m_nodeHandles.back());
		const auto& presentResource = m_resourcePool.finalResource(presentNode.outputs[0]);
		auto& sceneColorResource = m_resourcePool.finalResource(presentNode.inputs[0]);
		sceneColorResource.handle = presentResource.handle;

		m_resourcePool.createResources(device);

		// Print frame graph info
		std::cout << "FrameGraph compiled with " << m_nodeHandles.size() << " passes\n";
		for (const auto& nodeHandle : m_nodeHandles)
		{
			std::cout << "\t- " << m_resourcePool.node(nodeHandle).name << "\n";
		}
	}

	void FrameGraph::execute(const FrameInfo& frameInfo)
	{
		for (const auto& nodeHandle : m_nodeHandles)
		{
			auto& node = m_resourcePool.node(nodeHandle);
			node.pass->prepare(m_resourcePool, frameInfo);
		}

		for (const auto& nodeHandle : m_nodeHandles)
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

	void FrameGraph::sortNodes()
	{
		// Topological sort
		auto edges = computeEdges();

		std::vector<FrameGraphNodeHandle> sortedNodes;
		sortedNodes.reserve(m_nodeHandles.size());

		std::vector<FrameGraphNodeHandle> stack;
		stack.reserve(m_nodeHandles.size());

		std::vector<uint8_t> visited(m_nodeHandles.size(), 0);
		constexpr uint8_t VISITED = 1;
		constexpr uint8_t ADDED = 2;

		// Depth First Search starting at each node 
		for (const auto& nodeHandle : m_nodeHandles)
		{
			stack.emplace_back(nodeHandle);

			while (stack.size() > 0)
			{
				FrameGraphNodeHandle currentHandle = stack.back();

				if (visited[currentHandle.id] == ADDED)
				{
					stack.pop_back();
					continue;
				}

				if (visited[currentHandle.id] == VISITED)
				{
					visited[currentHandle.id] = ADDED;
					sortedNodes.emplace_back(currentHandle);
					stack.pop_back();
					continue;
				}

				visited[currentHandle.id] = VISITED;

				if (!edges.contains(currentHandle))
					continue;

				for (const auto& nextNodeHandle : edges[currentHandle])
				{
					if (!visited[nextNodeHandle.id])
						stack.emplace_back(nextNodeHandle);
				}
			}
		}

		assert(sortedNodes.size() == m_nodeHandles.size() && "Failed to sort nodes");

		m_nodeHandles.assign(sortedNodes.rbegin(), sortedNodes.rend());

		// Print frame graph info
		std::cout << "FrameGraph compiled with " << m_nodeHandles.size() << " passes\n";
		for (const auto& nodeHandle : m_nodeHandles)
		{
			std::cout << "\t- " << m_resourcePool.node(nodeHandle).name << "\n";
		}
	}

	auto FrameGraph::computeEdges() -> std::unordered_map<FrameGraphNodeHandle, std::vector<FrameGraphNodeHandle>>
	{
		// Compute graph edges
		std::unordered_map<FrameGraphNodeHandle, std::vector<FrameGraphNodeHandle>> edges;

		// Cache resource producers (Possible for one resource to be modified by multiple passes)
		std::unordered_map<std::string, std::vector<FrameGraphNodeHandle>> resourceProducers;
		for (const auto& nodeHandle : m_nodeHandles)
		{
			const auto& node = m_resourcePool.node(nodeHandle);
			for (const auto& outputHandle : node.outputs)
			{
				const auto& outputResource = m_resourcePool.resource(outputHandle);
				resourceProducers[outputResource.name].emplace_back(nodeHandle);
			}
		}

		for (const auto& nodeHandle : m_nodeHandles)
		{
			const auto& node = m_resourcePool.node(nodeHandle);
			for (const auto& inputHandle : node.inputs)
			{
				const auto& inputResource = m_resourcePool.finalResource(inputHandle);
				for (const auto& producerHandle : resourceProducers[inputResource.name])
				{
					if (producerHandle == nodeHandle) // Avoid self connections
						continue;

					auto& edgeHandles = edges[producerHandle];
					if (std::find(edgeHandles.begin(), edgeHandles.end(), nodeHandle) == edgeHandles.end())
						edgeHandles.push_back(nodeHandle);
				}
			}
		}

		return edges;
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
#include "frame_graph_resource_pool.h"

#include "graphics/frame_graph/frame_graph_render_pass.h"

#include <cassert>


namespace Aegix::Graphics
{
	auto FrameGraphResourcePool::node(FrameGraphNodeHandle handle) -> FrameGraphNode&
	{
		assert(handle != FrameGraphNode::INVALID_HANDLE && "Invalid node handle");
		assert(handle.id < m_nodes.size() && "Node handle out of range");
		return m_nodes[handle.id];
	}
	
	auto FrameGraphResourcePool::node(FrameGraphNodeHandle handle) const -> const FrameGraphNode&
	{
		assert(handle != FrameGraphNode::INVALID_HANDLE && "Invalid node handle");
		assert(handle.id < m_nodes.size() && "Node handle out of range");
		return m_nodes[handle.id];
	}

	auto FrameGraphResourcePool::resource(FrameGraphResourceHandle handle) -> FrameGraphResource&
	{
		assert(handle != FrameGraphResource::INVALID_HANDLE && "Invalid resource handle");
		assert(handle.id < m_resources.size() && "Resource handle out of range");
		return m_resources[handle.id];
	}
	
	auto FrameGraphResourcePool::resource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&
	{
		assert(handle != FrameGraphResource::INVALID_HANDLE && "Invalid resource handle");
		assert(handle.id < m_resources.size() && "Resource handle out of range");
		return m_resources[handle.id];
	}
	
	auto FrameGraphResourcePool::finalResource(FrameGraphResourceHandle handle) -> FrameGraphResource&
	{
		auto& res = resource(handle);
		if (res.type != FrameGraphResourceType::Reference)
			return res;

		auto& refRes = resource(res.handle);
		assert(refRes.type != FrameGraphResourceType::Reference && "Reference to another reference is not allowed");
		return refRes;
	}
	
	auto FrameGraphResourcePool::finalResource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&
	{
		const auto& res = resource(handle);
		if (res.type != FrameGraphResourceType::Reference)
			return res;

		const auto& refRes = resource(res.handle);
		assert(refRes.type != FrameGraphResourceType::Reference && "Reference to another reference is not allowed");
		return refRes;
	}
	
	auto FrameGraphResourcePool::texture(FrameGraphResourceHandle handle) -> Texture&
	{
		auto& res = finalResource(handle);
		assert(res.type == FrameGraphResourceType::Texture && "Resource is not a texture");
		assert(res.handle != FrameGraphResource::INVALID_HANDLE && "Texture handle is invalid");
		assert(res.handle.id < m_textures.size() && "Texture handle out of range");
		return m_textures[res.handle.id];
	}
	
	auto FrameGraphResourcePool::texture(FrameGraphResourceHandle handle) const -> const Texture&
	{
		const auto& res = finalResource(handle);
		assert(res.type == FrameGraphResourceType::Texture && "Resource is not a texture");
		assert(res.handle != FrameGraphResource::INVALID_HANDLE && "Texture handle is invalid");
		assert(res.handle.id < m_textures.size() && "Texture handle out of range");
		return m_textures[res.handle.id];
	}

	auto FrameGraphResourcePool::addNode(std::unique_ptr<FrameGraphRenderPass> pass) -> FrameGraphNodeHandle
	{
		auto createInfo = pass->createInfo(*this);
		m_nodes.emplace_back(createInfo.name, std::move(pass), createInfo.inputs, createInfo.outputs);
		return FrameGraphNodeHandle{ static_cast<uint32_t>(m_nodes.size() - 1) };
	}

	auto FrameGraphResourcePool::addResource(const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle
	{
		m_resources.emplace_back(createInfo.name, createInfo.type, createInfo.info, FrameGraphResource::INVALID_HANDLE);
		return FrameGraphResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	void FrameGraphResourcePool::resolveReferences()
	{
		for (auto& resource : m_resources)
		{
			if (resource.type != FrameGraphResourceType::Reference)
				continue;

			// Find the resource to reference
			for (uint32_t i = 0; i < m_resources.size(); i++)
			{
				auto& other = m_resources[i];
				if (other.type != FrameGraphResourceType::Reference && other.name == resource.name)
				{
					resource.handle = FrameGraphResourceHandle{ i };
					break;
				}
			}

			if (resource.handle == FrameGraphResource::INVALID_HANDLE)
			{

			}

			assert(resource.handle != FrameGraphResource::INVALID_HANDLE && "Failed to resolve reference");
		}
	}

	void FrameGraphResourcePool::createResources(VulkanDevice& device)
	{
		for (auto& resource : m_resources)
		{
			switch (resource.type)
			{
			case FrameGraphResourceType::Texture:
				createTexture(device, resource);
				break;

			case FrameGraphResourceType::Buffer:
				createBuffer(device, resource);
				break;

			case FrameGraphResourceType::Reference:
				break;
			default:
				[[unlikely]] break;
			}
		}
	}

	void FrameGraphResourcePool::createTexture(VulkanDevice& device, FrameGraphResource& resource)
	{
		auto& info = std::get<FrameGraphResourceTextureInfo>(resource.info);
		if (info.resizePolicy == ResizePolicy::SwapchainRelative)
		{
			info.extent = { DEFAULT_WIDTH, DEFAULT_HEIGHT };
		}

		m_textures.emplace_back(device, Texture::Config{
			.extent = info.extent,
			.format = info.format,
			.usage = info.usage
			});
		resource.handle = FrameGraphResourceHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
	}

	void FrameGraphResourcePool::createBuffer(VulkanDevice& device, FrameGraphResource& resource)
	{
		// TODO: Implement
	}
}

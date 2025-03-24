#include "pch.h"

#include "frame_graph_resource_pool.h"

#include "core/globals.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	static auto imageUsage(FrameGraphResourceUsage usage) -> VkImageUsageFlags
	{
		switch (usage)
		{
		case FrameGraphResourceUsage::None:
			return 0;
		case FrameGraphResourceUsage::Sampled:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case FrameGraphResourceUsage::ColorAttachment:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case FrameGraphResourceUsage::DepthStencilAttachment:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case FrameGraphResourceUsage::Compute:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case FrameGraphResourceUsage::TransferSrc:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case FrameGraphResourceUsage::TransferDst:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case FrameGraphResourceUsage::Present:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		default:
			AGX_ASSERT_X(false, "Undefined FrameGraphResourceUsage");
			return 0;
		}
	}

	auto FrameGraphResourcePool::node(FrameGraphNodeHandle handle) -> FrameGraphNode&
	{
		AGX_ASSERT_X(handle != FrameGraphNode::INVALID_HANDLE, "Invalid node handle");
		AGX_ASSERT_X(handle.id < m_nodes.size(), "Node handle out of range");
		return m_nodes[handle.id];
	}
	
	auto FrameGraphResourcePool::node(FrameGraphNodeHandle handle) const -> const FrameGraphNode&
	{
		AGX_ASSERT_X(handle != FrameGraphNode::INVALID_HANDLE, "Invalid node handle");
		AGX_ASSERT_X(handle.id < m_nodes.size(), "Node handle out of range");
		return m_nodes[handle.id];
	}

	auto FrameGraphResourcePool::resource(FrameGraphResourceHandle handle) -> FrameGraphResource&
	{
		AGX_ASSERT_X(handle != FrameGraphResource::INVALID_HANDLE, "Invalid resource handle");
		AGX_ASSERT_X(handle.id < m_resources.size(), "Resource handle out of range");
		return m_resources[handle.id];
	}
	
	auto FrameGraphResourcePool::resource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&
	{
		AGX_ASSERT_X(handle != FrameGraphResource::INVALID_HANDLE, "Invalid resource handle");
		AGX_ASSERT_X(handle.id < m_resources.size(), "Resource handle out of range");
		return m_resources[handle.id];
	}
	
	auto FrameGraphResourcePool::finalResource(FrameGraphResourceHandle handle) -> FrameGraphResource&
	{
		auto& res = resource(handle);
		if (res.type != FrameGraphResourceType::Reference)
			return res;

		auto& refRes = resource(res.handle);
		AGX_ASSERT_X(refRes.type != FrameGraphResourceType::Reference, "Reference to another reference is not allowed");
		return refRes;
	}
	
	auto FrameGraphResourcePool::finalResource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&
	{
		const auto& res = resource(handle);
		if (res.type != FrameGraphResourceType::Reference)
			return res;

		const auto& refRes = resource(res.handle);
		AGX_ASSERT_X(refRes.type != FrameGraphResourceType::Reference, "Reference to another reference is not allowed");
		return refRes;
	}
	
	auto FrameGraphResourcePool::texture(FrameGraphResourceHandle resourceHandle) -> Texture&
	{
		auto& res = finalResource(resourceHandle);
		AGX_ASSERT_X(res.type == FrameGraphResourceType::Texture, "Resource is not a texture");
		AGX_ASSERT_X(res.handle != FrameGraphResource::INVALID_HANDLE, "Texture handle is invalid");
		AGX_ASSERT_X(res.handle.id < m_textures.size(), "Texture handle out of range");
		return m_textures[res.handle.id];
	}
	
	auto FrameGraphResourcePool::texture(FrameGraphResourceHandle resourceHandle) const -> const Texture&
	{
		const auto& res = finalResource(resourceHandle);
		AGX_ASSERT_X(res.type == FrameGraphResourceType::Texture, "Resource is not a texture");
		AGX_ASSERT_X(res.handle != FrameGraphResource::INVALID_HANDLE, "Texture handle is invalid");
		AGX_ASSERT_X(res.handle.id < m_textures.size(), "Texture handle out of range");
		return m_textures[res.handle.id];
	}

	auto FrameGraphResourcePool::addNode(std::unique_ptr<FrameGraphRenderPass> pass) -> FrameGraphNodeHandle
	{
		FrameGraphNodeHandle handle{ static_cast<uint32_t>(m_nodes.size()) };
		FrameGraphResourceBuilder builder{ *this, handle };
		auto createInfo = pass->createInfo(builder);
		m_nodes.emplace_back(createInfo.name, std::move(pass), createInfo.inputs, createInfo.outputs);
		return handle;
	}

	auto FrameGraphResourcePool::addResource(const FrameGraphResourceCreateInfo& createInfo, FrameGraphNodeHandle producer) -> FrameGraphResourceHandle
	{
		m_resources.emplace_back(createInfo.name, createInfo.type, createInfo.usage, createInfo.info, 
			FrameGraphResource::INVALID_HANDLE, producer);
		return FrameGraphResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	auto FrameGraphResourcePool::addResource(Texture texture, const FrameGraphResourceCreateInfo& createInfo, 
		FrameGraphNodeHandle producer) -> FrameGraphResourceHandle
	{
		auto ResourceHandle = addResource(createInfo, producer);
		m_textures.emplace_back(std::move(texture));
		resource(ResourceHandle).handle = FrameGraphResourceHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
		return ResourceHandle;
	}

	auto FrameGraphResourcePool::addExternalResource(Texture texture, const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle
	{
		auto ResourceHandle = addResource(createInfo, FrameGraphNode::INVALID_HANDLE);
		m_textures.emplace_back(std::move(texture));
		resource(ResourceHandle).handle = FrameGraphResourceHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
		return ResourceHandle;
	}

	void FrameGraphResourcePool::resolveReferences()
	{
		for (auto& resource : m_resources)
		{
			if (resource.type != FrameGraphResourceType::Reference)
				continue;

			// Find the resource for the reference by name
			for (size_t i = 0; i < m_resources.size(); i++)
			{
				auto& other = m_resources[i];
				if (other.type != FrameGraphResourceType::Reference && other.name == resource.name)
				{
					resource.handle = FrameGraphResourceHandle{ static_cast<uint32_t>(i) };
					break;
				}
			}

			if (resource.handle == FrameGraphResource::INVALID_HANDLE)
				ALOG::fatal("Failed to resolve reference '{}'", resource.name);

			AGX_ASSERT(resource.handle != FrameGraphResource::INVALID_HANDLE && "Failed to resolve reference");
		}
	}

	void FrameGraphResourcePool::createResources(VulkanDevice& device)
	{
		// Accumulate usage flags for each resource reference
		for (auto& initialResource : m_resources)
		{
			FrameGraphResourceUsage usage = initialResource.usage;

			FrameGraphResource* actualResource = &initialResource;
			if (initialResource.type == FrameGraphResourceType::Reference)
				actualResource = &resource(initialResource.handle);

			if (actualResource->type == FrameGraphResourceType::Texture)
			{
				auto& info = std::get<FrameGraphResourceTextureInfo>(actualResource->info);
				info.usage |= imageUsage(usage);
			}
		}

		for (auto& resource : m_resources)
		{
			if (resource.handle != FrameGraphResource::INVALID_HANDLE) // Already created
				continue;

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

	void FrameGraphResourcePool::resizeImages(uint32_t width, uint32_t height)
	{
		for (auto& res : m_resources)
		{
			if (res.type != FrameGraphResourceType::Texture)
				continue;

			auto& info = std::get<FrameGraphResourceTextureInfo>(res.info);
			if (info.resizePolicy == ResizePolicy::SwapchainRelative)
			{
				m_textures[res.handle.id].resize({ width, height, 1 }, info.usage);
				info.extent = { width, height };
			}
		}
	}

	void FrameGraphResourcePool::createTexture(VulkanDevice& device, FrameGraphResource& resource)
	{
		auto& info = std::get<FrameGraphResourceTextureInfo>(resource.info);
		if (info.resizePolicy == ResizePolicy::SwapchainRelative)
		{
			info.extent = { Core::DEFAULT_WIDTH, Core::DEFAULT_HEIGHT };
		}

		auto& texture = m_textures.emplace_back(device);
		texture.create2D(info.extent.width, info.extent.height, info.format, info.usage, info.mipLevels);

		resource.handle = FrameGraphResourceHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
	}

	void FrameGraphResourcePool::createBuffer(VulkanDevice& device, FrameGraphResource& resource)
	{
		// TODO: Implement
	}
}

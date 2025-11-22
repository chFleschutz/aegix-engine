#include "pch.h"
#include "frame_graph_resource_pool.h"

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	auto FGResourcePool::node(FGNodeHandle handle) -> FGNode&
	{
		assert(handle.isValid());
		assert(handle.handle < m_nodes.size());
		return m_nodes[handle.handle];
	}


	auto FGResourcePool::resource(FGResourceHandle handle) -> FGResource&
	{
		assert(handle.isValid());
		assert(handle.handle < m_resources.size());
		return m_resources[handle.handle];
	}

	auto FGResourcePool::actualResource(FGResourceHandle handle) -> FGResource&
	{
		return m_resources[actualHandle(handle).handle];
	}

	auto FGResourcePool::actualHandle(FGResourceHandle handle) -> FGResourceHandle
	{
		assert(handle.isValid());
		const auto& res = resource(handle);
		if (auto info = std::get_if<FGReferenceInfo>(&res.info))
			return info->handle;
		return handle;
	}

	auto FGResourcePool::buffer(FGBufferHandle handle) -> Buffer&
	{
		assert(handle.isValid());
		assert(handle.handle < m_buffers.size());
		return m_buffers[handle.handle];
	}

	auto FGResourcePool::buffer(FGResourceHandle handle) -> Buffer&
	{
		auto& res = resource(actualHandle(handle));
		AGX_ASSERT_X(std::holds_alternative<FGBufferInfo>(res.info), "Resource is not a buffer");
		return buffer(std::get<FGBufferInfo>(res.info).handle);
	}

	auto FGResourcePool::texture(FGTextureHandle handle) -> Texture&
	{
		assert(handle.isValid());
		assert(handle.handle < m_textures.size());
		return m_textures[handle.handle];
	}

	auto FGResourcePool::texture(FGResourceHandle handle) -> Texture&
	{
		auto& res = resource(actualHandle(handle));
		AGX_ASSERT_X(std::holds_alternative<FGTextureInfo>(res.info), "Resource is not a texture");
		return texture(std::get<FGTextureInfo>(res.info).handle);
	}

	auto FGResourcePool::addNode(std::unique_ptr<FGRenderPass> pass) -> FGNodeHandle
	{
		auto nodeInfo = pass->info();
		m_nodes.emplace_back(nodeInfo, std::move(pass));
		return FGNodeHandle{ static_cast<uint32_t>(m_nodes.size() - 1) };
	}

	auto FGResourcePool::addBuffer(const std::string& name, FGResourceUsage usage, const FGBufferInfo& info) -> FGResourceHandle
	{
		m_resources.emplace_back(name, usage, info);
		return FGResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	auto FGResourcePool::addImage(const std::string& name, FGResourceUsage usage, const FGTextureInfo& info) -> FGResourceHandle
	{
		m_resources.emplace_back(name, usage, info);
		return FGResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	auto FGResourcePool::addReference(const std::string& name, FGResourceUsage usage) -> FGResourceHandle
	{
		m_resources.emplace_back(name, usage, FGReferenceInfo{});
		return FGResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
	}

	void FGResourcePool::resolveReferences()
	{
		for (auto& resource : m_resources)
		{
			if (!std::holds_alternative<FGReferenceInfo>(resource.info))
				continue;

			// Find the referenced resource by name
			auto& info = std::get<FGReferenceInfo>(resource.info);
			for (size_t i = 0; i < m_resources.size(); ++i)
			{
				const auto& other = m_resources[i];
				if (std::holds_alternative<FGReferenceInfo>(other.info))
					continue;

				if (other.name == resource.name)
				{
					info.handle = FGResourceHandle{ static_cast<uint32_t>(i) };
					break;
				}
			}

			if (!info.handle.isValid())
			{
				ALOG::fatal("Error: Unable to resolve reference for resource '{}'", resource.name);
			}
		}
	}

	void FGResourcePool::createResources()
	{
		// Accumulate usage flags
		for (const auto& res : m_resources)
		{
			if (!std::holds_alternative<FGReferenceInfo>(res.info))
				continue;

			auto& info = std::get<FGReferenceInfo>(res.info);
			auto& actualResource = resource(info.handle);

			if (std::holds_alternative<FGTextureInfo>(actualResource.info))
			{
				auto& textureInfo = std::get<FGTextureInfo>(actualResource.info);
				textureInfo.usage |= toImageUsage(res.usage);
			}
			else if (std::holds_alternative<FGBufferInfo>(actualResource.info))
			{
				auto& bufferInfo = std::get<FGBufferInfo>(actualResource.info);
				bufferInfo.usage |= toBufferUsage(res.usage);
			}
		}

		// Create actual resources
		for (auto& resource : m_resources)
		{
			if (std::holds_alternative<FGReferenceInfo>(resource.info))
				continue;
			if (std::holds_alternative<FGTextureInfo>(resource.info))
			{
				auto& textureInfo = std::get<FGTextureInfo>(resource.info);
				textureInfo.handle = createImage(textureInfo);
			}
			else if (std::holds_alternative<FGBufferInfo>(resource.info))
			{
				auto& bufferInfo = std::get<FGBufferInfo>(resource.info);
				bufferInfo.handle = createBuffer(bufferInfo);
			}
		}
	}

	auto FGResourcePool::createBuffer(const FGBufferInfo& info) -> FGBufferHandle
	{
		m_buffers.emplace_back(); // TODO: Create with info
		return FGBufferHandle{ static_cast<uint32_t>(m_buffers.size() - 1) };
	}

	auto FGResourcePool::createImage(const FGTextureInfo& info) -> FGTextureHandle
	{
		m_textures.emplace_back(); // TODO: Create with info
		return FGTextureHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
	}

	void FGResourcePool::resizeImages(uint32_t width, uint32_t height)
	{
		for (auto& res : m_resources)
		{
			if (!std::holds_alternative<FGTextureInfo>(res.info))
				continue;

			auto& info = std::get<FGTextureInfo>(res.info);
			if (info.resizeMode == FGResizeMode::SwapChainRelative)
			{
				m_textures[info.handle.handle].resize({ width, height, 1 }, info.usage);
				info.extent = { width, height };
			}
		}
	}
}

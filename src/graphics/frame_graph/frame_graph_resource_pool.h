#pragma once

#include "graphics/descriptors.h"
#include "graphics/systems/render_system.h"
#include "graphics/texture.h"
#include "graphics/uniform_buffer.h"

#include <array>
#include <cassert>
#include <limits>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace Aegix::Graphics
{
	class FrameGraphRenderPass;

	struct FrameGraphResourceHandle
	{
		uint32_t id;

		constexpr auto operator<=>(const FrameGraphResourceHandle&) const = default;
	};

	struct FrameGraphNodeHandle
	{
		uint32_t id;

		constexpr auto operator<=>(const FrameGraphNodeHandle&) const = default;
	};

	enum class FrameGraphResourceType
	{
		Buffer,
		Texture,
		Reference
	};

	enum class ResizePolicy
	{
		Fixed,
		SwapchainRelative
	};

	struct FrameGraphResourceBufferInfo
	{
		// TODO: Add buffer info
	};

	struct FrameGraphResourceTextureInfo
	{
		VkExtent2D extent;
		VkFormat format;
		ResizePolicy resizePolicy = ResizePolicy::Fixed;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	};

	using FrameGraphResourceInfo = std::variant<FrameGraphResourceBufferInfo, FrameGraphResourceTextureInfo>;

	struct FrameGraphResource
	{
		static constexpr FrameGraphResourceHandle INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };

		std::string name;
		FrameGraphResourceType type;
		FrameGraphResourceInfo info;
		FrameGraphResourceHandle handle;
	};

	struct FrameGraphNode
	{
		static constexpr FrameGraphNodeHandle INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };

		std::string name;
		std::unique_ptr<FrameGraphRenderPass> pass;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};

	struct FrameGraphResourceCreateInfo
	{
		std::string name;
		FrameGraphResourceType type;
		FrameGraphResourceInfo info;
	};

	struct FrameGraphNodeCreateInfo
	{
		std::string name;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};


	struct RenderStage
	{
		enum class Type
		{
			Geometry,
			Count
		};

		std::vector<std::unique_ptr<RenderSystem>> renderSystems;
		std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
		std::unique_ptr<DescriptorSet> descriptorSet;
		std::unique_ptr<UniformBuffer> ubo;
	};

	class FrameGraphResourcePool
	{
	public:
		FrameGraphResourcePool() = default;
		FrameGraphResourcePool(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool(FrameGraphResourcePool&&) = delete;
		~FrameGraphResourcePool() = default;

		FrameGraphResourcePool& operator=(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool& operator=(FrameGraphResourcePool&&) = delete;

		[[nodiscard]] auto node(FrameGraphNodeHandle handle) -> FrameGraphNode& 
		{ 
			assert(handle != FrameGraphNode::INVALID_HANDLE && "Invalid node handle");
			assert(handle.id < m_nodes.size() && "Node handle out of range");
			return m_nodes[handle.id]; 
		}
		[[nodiscard]] auto node(FrameGraphNodeHandle handle) const -> const FrameGraphNode& 
		{
			assert(handle != FrameGraphNode::INVALID_HANDLE && "Invalid node handle");
			assert(handle.id < m_nodes.size() && "Node handle out of range");
			return m_nodes[handle.id];
		}

		[[nodiscard]] auto resource(FrameGraphResourceHandle handle) -> FrameGraphResource& 
		{ 
			assert(handle != FrameGraphResource::INVALID_HANDLE && "Invalid resource handle");
			assert(handle.id < m_resources.size() && "Resource handle out of range");
			return m_resources[handle.id]; 
		}
		[[nodiscard]] auto resource(FrameGraphResourceHandle handle) const -> const FrameGraphResource& 
		{ 
			assert(handle != FrameGraphResource::INVALID_HANDLE && "Invalid resource handle");
			assert(handle.id < m_resources.size() && "Resource handle out of range");
			return m_resources[handle.id]; 
		}
		[[nodiscard]] auto finalResource(FrameGraphResourceHandle handle) -> FrameGraphResource&
		{
			auto& res = resource(handle);
			if (res.type != FrameGraphResourceType::Reference)
				return res;

			auto& refRes = resource(res.handle);
			assert(refRes.type != FrameGraphResourceType::Reference && "Reference to another reference is not allowed");
			return refRes;
		}
		[[nodiscard]] auto finalResource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&
		{
			const auto& res = resource(handle);
			if (res.type != FrameGraphResourceType::Reference)
				return res;

			const auto& refRes = resource(res.handle);
			assert(refRes.type != FrameGraphResourceType::Reference && "Reference to another reference is not allowed");
			return refRes;
		}
		[[nodiscard]] auto texture(FrameGraphResourceHandle handle) -> Texture& 
		{ 
			auto& res = finalResource(handle);
			assert(res.type == FrameGraphResourceType::Texture && "Resource is not a texture");
			assert(res.handle != FrameGraphResource::INVALID_HANDLE && "Texture handle not created");
			assert(res.handle.id < m_textures.size() && "Texture handle out of range");
			return m_textures[res.handle.id];
		}
		[[nodiscard]] auto texture(FrameGraphResourceHandle handle) const -> const Texture& 
		{ 
			const auto& res = finalResource(handle);
			assert(res.type == FrameGraphResourceType::Texture && "Resource is not a texture");
			assert(res.handle != FrameGraphResource::INVALID_HANDLE && "Texture handle not created");
			assert(res.handle.id < m_textures.size() && "Texture handle out of range");
			return m_textures[res.handle.id];
		}


		template<typename T, typename... Args>
			requires std::is_base_of_v<FrameGraphRenderPass, T>&& std::is_constructible_v<T, Args...>
		auto addNode(Args&&... args) -> FrameGraphNodeHandle
		{
			auto pass = std::make_unique<T>(std::forward<Args>(args)...);
			auto createInfo = pass->createInfo(*this);
			m_nodes.emplace_back(createInfo.name, std::move(pass), createInfo.inputs, createInfo.outputs);
			return FrameGraphNodeHandle{ static_cast<uint32_t>(m_nodes.size() - 1) };
		}

		auto addResource(const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle
		{
			m_resources.emplace_back(createInfo.name, createInfo.type, createInfo.info, FrameGraphResource::INVALID_HANDLE);
			return FrameGraphResourceHandle{ static_cast<uint32_t>(m_resources.size() - 1) };
		}

		template<typename T>
			requires std::is_base_of_v<RenderSystem, T>
		RenderSystem& addRenderSystem(VulkanDevice& device, RenderStage::Type stageType)
		{
			auto& stage = renderStage(stageType);

			// Check if rendersystem type already exists
			for (auto& system : stage.renderSystems)
			{
				if (dynamic_cast<T*>(system.get()))
					return *system;
			}

			stage.renderSystems.emplace_back(std::make_unique<T>(device, *stage.descriptorSetLayout));
			return *stage.renderSystems.back();
		}

		auto renderStage(RenderStage::Type type) -> RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }
		auto renderStage(RenderStage::Type type) const -> const RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }

		void resolveReferences()
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

		void createResources(VulkanDevice& device)
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

	private:
		void createTexture(VulkanDevice& device, FrameGraphResource& resource)
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

		void createBuffer(VulkanDevice& device, FrameGraphResource& resource)
		{
			// TODO: Implement
		}

		std::vector<FrameGraphNode> m_nodes;
		std::vector<FrameGraphResource> m_resources;
		std::vector<Texture> m_textures;
		std::vector<Buffer> m_buffers;

		std::array<RenderStage, static_cast<size_t>(RenderStage::Type::Count)> m_renderStages;
	};
}
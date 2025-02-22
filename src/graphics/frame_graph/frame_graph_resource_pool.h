#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/systems/render_system.h"
#include "graphics/texture.h"

#include <array>
#include <limits>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace Aegix::Graphics
{
	class FrameGraphRenderPass;


	// FrameGraphResource --------------------------------------------------------

	struct FrameGraphResourceHandle
	{
		uint32_t id;

		constexpr auto operator<=>(const FrameGraphResourceHandle&) const = default;
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

	struct FrameGraphResourceCreateInfo
	{
		std::string name;
		FrameGraphResourceType type;
		FrameGraphResourceInfo info;
	};

	struct FrameGraphResource
	{
		static constexpr FrameGraphResourceHandle INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };

		std::string name;
		FrameGraphResourceType type;
		FrameGraphResourceInfo info;
		FrameGraphResourceHandle handle;
	};


	// FrameGraphNode ------------------------------------------------------------

	struct FrameGraphNodeHandle
	{
		uint32_t id;

		constexpr auto operator<=>(const FrameGraphNodeHandle&) const = default;
	};

	struct FrameGraphNodeCreateInfo
	{
		std::string name;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};

	struct FrameGraphNode
	{
		static constexpr FrameGraphNodeHandle INVALID_HANDLE{ std::numeric_limits<uint32_t>::max() };

		std::string name;
		std::unique_ptr<FrameGraphRenderPass> pass;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};


	// RenderStage ---------------------------------------------------------------

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


	// FrameGraphResourcePool ----------------------------------------------------

	/// @brief Holds all resources and nodes for a frame graph
	class FrameGraphResourcePool
	{
	public:
		FrameGraphResourcePool() = default;
		FrameGraphResourcePool(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool(FrameGraphResourcePool&&) = delete;
		~FrameGraphResourcePool() = default;

		FrameGraphResourcePool& operator=(const FrameGraphResourcePool&) = delete;
		FrameGraphResourcePool& operator=(FrameGraphResourcePool&&) = delete;

		/// @brief Returns the node for the given handle
		[[nodiscard]] auto node(FrameGraphNodeHandle handle) -> FrameGraphNode&;
		[[nodiscard]] auto node(FrameGraphNodeHandle handle) const -> const FrameGraphNode&;

		/// @brief Returns the resource for the given handle
		[[nodiscard]] auto resource(FrameGraphResourceHandle handle) -> FrameGraphResource&;
		[[nodiscard]] auto resource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&;

		/// @brief Returns the resource for the given handle and resolves any references
		[[nodiscard]] auto finalResource(FrameGraphResourceHandle handle) -> FrameGraphResource&;
		[[nodiscard]] auto finalResource(FrameGraphResourceHandle handle) const -> const FrameGraphResource&;

		/// @brief Returns the texture for the given handle (must be a texture resource)
		[[nodiscard]] auto texture(FrameGraphResourceHandle handle) -> Texture&;
		[[nodiscard]] auto texture(FrameGraphResourceHandle handle) const -> const Texture&;

		auto addNode(std::unique_ptr<FrameGraphRenderPass> pass) -> FrameGraphNodeHandle;
		auto addResource(const FrameGraphResourceCreateInfo& createInfo) -> FrameGraphResourceHandle;

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

		/// @brief For all reference resources, resolve the handle to the actual resource
		void resolveReferences();
		void createResources(VulkanDevice& device);

	private:
		void createTexture(VulkanDevice& device, FrameGraphResource& resource);
		void createBuffer(VulkanDevice& device, FrameGraphResource& resource);

		std::vector<FrameGraphNode> m_nodes;
		std::vector<FrameGraphResource> m_resources;
		std::vector<Texture> m_textures;
		std::vector<Buffer> m_buffers;

		std::array<RenderStage, static_cast<size_t>(RenderStage::Type::Count)> m_renderStages;
	};
}
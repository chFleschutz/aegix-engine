#pragma once

#include "graphics/descriptors.h"
#include "graphics/systems/render_system.h"
#include "graphics/texture.h"
#include "graphics/uniform_buffer.h"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Aegix::Graphics
{
	class FrameGraphRenderPass;

	struct FrameGraphResourceHandle
	{
		uint32_t id;
	};

	struct FrameGraphNodeHandle
	{
		uint32_t id;
	};

	struct FrameGraphNodeCreateInfo
	{
		std::string name;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};

	struct FrameGraphNode
	{
		std::string name;
		std::unique_ptr<FrameGraphRenderPass> pass;
		std::vector<FrameGraphResourceHandle> inputs;
		std::vector<FrameGraphResourceHandle> outputs;
	};



	enum class ResizePolicy
	{
		Fixed,
		SwapchainRelative
	};

	struct FrameGraphTexture
	{
		std::string name;
		Texture texture;
		VkImageUsageFlags usage;
		ResizePolicy resizePolicy;
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

		[[nodiscard]] auto node(FrameGraphNodeHandle handle) -> FrameGraphNode& { return m_nodes[handle.id]; }
		[[nodiscard]] auto node(FrameGraphNodeHandle handle) const -> const FrameGraphNode& { return m_nodes[handle.id]; }
		[[nodiscard]] auto texture(FrameGraphResourceHandle handle) -> FrameGraphTexture& { return m_textures[handle.id]; }
		[[nodiscard]] auto texture(FrameGraphResourceHandle handle) const -> const FrameGraphTexture& { return m_textures[handle.id]; }


		template<typename T, typename... Args>
			requires std::is_base_of_v<FrameGraphRenderPass, T> && std::is_constructible_v<T, Args...>
		auto addNode(Args&&... args) -> FrameGraphNodeHandle
		{
			auto pass = std::make_unique<T>(std::forward<Args>(args)...);
			auto createInfo = pass->createInfo();
			m_nodes.emplace_back(createInfo.name, std::move(pass), createInfo.inputs, createInfo.outputs);
			return FrameGraphNodeHandle{ static_cast<uint32_t>(m_nodes.size() - 1) };
		}

		FrameGraphResourceHandle addTexture(VulkanDevice& device, const std::string& name, uint32_t width, uint32_t height,
			VkFormat format, VkImageUsageFlags usage, ResizePolicy resizePolicy)
		{
			m_textures.emplace_back(name, Texture{ device, width, height, format, usage }, usage, resizePolicy);
			return FrameGraphResourceHandle{ static_cast<uint32_t>(m_textures.size() - 1) };
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

		auto textures() -> std::vector<FrameGraphTexture>& { return m_textures; }

		auto renderStage(RenderStage::Type type) -> RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }
		auto renderStage(RenderStage::Type type) const -> const RenderStage& { return m_renderStages[static_cast<size_t>(type)]; }

	private:
		std::vector<FrameGraphNode> m_nodes;

		std::vector<FrameGraphTexture> m_textures;
		std::array<RenderStage, static_cast<size_t>(RenderStage::Type::Count)> m_renderStages;
	};
}
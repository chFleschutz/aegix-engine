#pragma once

#include "ui/layer.h"

#include "vulkan/vulkan.h"

#include <memory>
#include <vector>

namespace Aegix::Graphics
{
	class Window;
	class Renderer;

	/// @brief Manages all GUI Layers for displaying ImGui elements
	/// @note This class is a wrapper around ImGui
	class UI
	{
	public:
		UI(const Window& window, Renderer& renderer);
		UI(const UI&) = delete;
		UI(UI&&) = delete;
		~UI();

		UI& operator=(const UI&) = delete;
		UI& operator=(UI&&) = delete;

		/// @brief Updates all GUI layers
		void update(float deltaTime);

		/// @brief Renders all GUI elements
		void render(VkCommandBuffer commandBuffer);

		/// @brief Pushes a layer to the stack
		void pushLayer(std::shared_ptr<Layer> layer);

		/// @brief Pops a layer from the stack
		void popLayer(std::shared_ptr<Layer> layer);

		/// @brief Pushes a layer of type T to the stack
		template<typename T, typename... Args>
		std::shared_ptr<T> pushLayer(Args&&... args)
		{
			auto layer = std::make_shared<T>(std::forward<Args>(args)...);
			pushLayer(layer);
			return layer;
		}

		/// @brief Pushes a layer of type T if it does not exist in the stack
		template<typename T, typename... Args>
		std::shared_ptr<T> pushLayerIfNotExist(Args&&... args)
		{
			for (auto& layer : m_layers)
			{
				if (std::dynamic_pointer_cast<T>(layer))
					return std::dynamic_pointer_cast<T>(layer);
			}

			return pushLayer<T>(std::forward<Args>(args)...);
		}

		/// @brief Removes the last pushed layer of type T from the stack
		template<typename T>
		void popLayer()
		{
			for (auto it = m_layers.rbegin(); it != m_layers.rend(); it++)
			{
				if (std::dynamic_pointer_cast<T>(*it))
				{
					popLayer(*it);
					return;
				}
			}
		}

	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
	};
}
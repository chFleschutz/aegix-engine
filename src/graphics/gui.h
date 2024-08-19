#pragma once

#include "graphics/renderer.h"
#include "graphics/window.h"
#include "graphics/layers/layer.h"

#include "imgui.h"

namespace Aegix::Graphics
{
	class GUI
	{
	public:
		GUI(Window& window, Renderer& renderer);
		~GUI();

		/// @brief Updates all layers
		void update(float deltaTime);

		/// @brief Renders all GUI elements
		void renderGui(VkCommandBuffer commandBuffer);

		/// @brief Pushes a layer to the stack
		void push(std::shared_ptr<Layer> layer);

		/// @brief Pops a layer from the stack
		void pop(std::shared_ptr<Layer> layer);

		/// @brief Pushes a layer of type T to the stack
		template<typename T, typename... Args>
		std::shared_ptr<T> push(Args&&... args)
		{
			auto layer = std::make_shared<T>(std::forward<Args>(args)...);
			push(layer);
			return layer;
		}

		/// @brief Pushes a layer of type T if it does not exist in the stack
		template<typename T, typename... Args>
		std::shared_ptr<T> pushIfNotExist(Args&&... args)
		{
			for (auto& layer : m_layers)
			{
				if (std::dynamic_pointer_cast<T>(layer))
					return std::dynamic_pointer_cast<T>(layer);
			}

			return push<T>(std::forward<Args>(args)...);
		}

		/// @brief Removes the last pushed layer of type T from the stack
		template<typename T>
		void pop()
		{
			for (auto it = m_layers.rbegin(); it != m_layers.rend(); it++)
			{
				if (std::dynamic_pointer_cast<T>(*it))
				{
					pop(*it);
					return;
				}
			}
		}

	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
	};
}
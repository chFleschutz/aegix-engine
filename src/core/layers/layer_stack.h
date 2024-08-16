#pragma once

#include <memory>
#include <vector>

namespace Aegix
{
	class Layer;

	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		template<typename T, typename... Args>
		std::shared_ptr<T> push(Args&&... args)
		{
			auto layer = std::make_shared<T>(std::forward<Args>(args)...);
			push(layer);
			return layer;
		}

		void push(std::shared_ptr<Layer> layer);

		void pop(std::shared_ptr<Layer> layer);
		
		void update(float deltaTime);
		
		void renderGui();
		
	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
	};
}
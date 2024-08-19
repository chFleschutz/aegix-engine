#pragma once

namespace Aegix::Graphics
{
	/// @brief Base class for all GUI layers
	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		/// @brief Called once when the layer is pushed to the stack
		virtual void onAttach() {};

		/// @brief Called once when the layer is popped from the stack
		virtual void onDetach() {};

		/// @brief Called once per frame before rendering started
		virtual void onUpdate(float deltaTime) {};

		/// @brief Called once per frame after scene rendering is done
		virtual void onGuiRender() {};
	};
}
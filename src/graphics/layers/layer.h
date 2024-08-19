#pragma once

namespace Aegix
{
	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void onAttach() {};
		virtual void onDetach() {};

		virtual void onUpdate(float deltaTime) {};
		virtual void onGuiRender() {};
	};
}
#pragma once

#include "graphics/device.h"
#include "graphics/globals.h"
#include "graphics/renderer.h"

namespace Aegix::Graphics
{
	class Graphics
	{
	public:
		Graphics(Core::Window& window)
			: m_device{ window }, m_renderer{ window, m_device } 
		{
		}
		Graphics(const Graphics&) = delete;
		Graphics(Graphics&&) = delete;
		~Graphics() = default;

		auto operator=(const Graphics&) -> Graphics& = delete;
		auto operator=(Graphics&&) -> Graphics& = delete;

		[[nodiscard]] auto device() -> VulkanDevice& { return m_device; }
		[[nodiscard]] auto renderer() -> Renderer& { return m_renderer; }

	private:
		VulkanDevice m_device;
		Renderer m_renderer;
	};
}

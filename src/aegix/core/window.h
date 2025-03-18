#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Aegix::Core
{
	class Window
	{
	public:
		Window(int width, int height, std::string title);
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		~Window();

		auto operator=(const Window&) -> Window& = delete;
		auto operator=(Window&&) -> Window& = delete;

		[[nodiscard]] auto glfwWindow() const -> GLFWwindow* { return m_window; }
		[[nodiscard]] auto shouldClose() const -> bool { return glfwWindowShouldClose(m_window); }
		[[nodiscard]] auto extent() const -> VkExtent2D { return { m_width, m_height }; }
		[[nodiscard]] auto wasResized() const -> bool { return m_windowResized; }

		void resetResizedFlag() { m_windowResized = false; }
		void createSurface(VkInstance instance, VkSurfaceKHR& surface) const;

	private:
		static void onWindowResize(GLFWwindow* glfwWindow, int newWidth, int newHeight);

		GLFWwindow* m_window = nullptr;
		std::string m_windowTitle;
		uint32_t m_width;
		uint32_t m_height;
		bool m_windowResized = false;
	};
}

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vre 
{
	class Window
	{
	public:
		Window(int width, int height, std::string title);
		~Window();

		Window(const Window&) = delete;
		Window operator=(const Window&) = delete;

		bool shouldClose() const { return glfwWindowShouldClose(m_window); }
		VkExtent2D extend() const { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }
		bool wasWindowResized() const { return m_frameBufferResized; }
		void resetWindowResizedFlag() { m_frameBufferResized = false; }
		GLFWwindow* glfwWindow() const { return m_window; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		void initWindow();

		int m_width;
		int m_height;
		bool m_frameBufferResized = false;

		std::string m_windowTitle;
		GLFWwindow* m_window = nullptr;
	};

} // namespace vre
#include "pch.h"
#include "window.h"

#include "graphics/vulkan/vulkan_tools.h"

namespace Aegis::Core
{
	Window::Window(int width, int height, std::string title) 
		: m_width(width), m_height(height), m_windowTitle(std::move(title))
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(m_width, m_height, m_windowTitle.c_str(), nullptr, nullptr);
		AGX_ASSERT_X(m_window, "Failed to create GLFW window");

		glfwSetWindowUserPointer(m_window, this);
		glfwSetWindowSizeCallback(m_window, onWindowResize);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Window::createSurface(VkInstance instance, VkSurfaceKHR& surface) const
	{
		VK_CHECK(glfwCreateWindowSurface(instance, m_window, nullptr, &surface));
	}

	void Window::onWindowResize(GLFWwindow* glfwWindow, int newWidth, int newHeight)
	{
		auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->m_windowResized = true;
		window->m_width = static_cast<uint32_t>(newWidth);
		window->m_height = static_cast<uint32_t>(newHeight);
	}
}

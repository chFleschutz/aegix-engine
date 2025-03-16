#include "pch.h"

#include "window.h"

#include "graphics/vulkan_tools.h"

namespace Aegix::Core
{
	Window::Window(int width, int height, std::string title) : m_width(width), m_height(height), m_windowTitle(std::move(title))
	{
		initWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		VK_CHECK(glfwCreateWindowSurface(instance, m_window, nullptr, surface))
	}

	void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height)
	{
		auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->m_frameBufferResized = true;
		window->m_width = width;
		window->m_height = height;
	}

	void Window::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(m_width, m_height, m_windowTitle.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetWindowSizeCallback(m_window, framebufferResizeCallback);
	}
}

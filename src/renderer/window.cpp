#include "window.h"

#include <stdexcept>

namespace vre
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
		if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface");
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto vreWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		vreWindow->m_frameBufferResized = true;
		vreWindow->m_width = width;
		vreWindow->m_height = height;
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

} // namespace vre
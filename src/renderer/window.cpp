#include "window.h"

#include <stdexcept>

namespace vre
{
	Window::Window(int width, int height, std::string title) : mWidth(width), mHeight(height), mWindowTitle(std::move(title))
	{
		initWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, mWindow, nullptr, surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface");
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto vreWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		vreWindow->mFrameBufferResized = true;
		vreWindow->mWidth = width;
		vreWindow->mHeight = height;
	}

	void Window::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindow = glfwCreateWindow(mWidth, mHeight, mWindowTitle.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(mWindow, this);
		glfwSetWindowSizeCallback(mWindow, framebufferResizeCallback);
	}

} // namespace vre
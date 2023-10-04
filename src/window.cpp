#include "window.h"

#include <stdexcept>

namespace vre
{
	VreWindow::VreWindow(int width, int height, std::string title) : mWidth(width), mHeight(height), mWindowTitle(std::move(title))
	{
		initWindow();
	}

	VreWindow::~VreWindow()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	void VreWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, mWindow, nullptr, surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface");
	}

	void VreWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto vreWindow = reinterpret_cast<VreWindow*>(glfwGetWindowUserPointer(window));
		vreWindow->mFrameBufferResized = true;
		vreWindow->mWidth = width;
		vreWindow->mHeight = height;
	}

	void VreWindow::initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindow = glfwCreateWindow(mWidth, mHeight, mWindowTitle.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(mWindow, this);
		glfwSetWindowSizeCallback(mWindow, framebufferResizeCallback);
	}

} // namespace vre
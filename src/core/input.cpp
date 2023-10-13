#include "input.h"

namespace vre
{
	Input& Input::instance()
	{
		static Input instance;
		return instance;
	}

	void Input::initialize(GLFWwindow* window)
	{
		mWindow = window;

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	bool Input::keyPressed(int key)
	{
		return glfwGetKey(mWindow, key) == GLFW_PRESS;
	}

	bool Input::mouseButtonPressed(int button)
	{
		return glfwGetMouseButton(mWindow, button) == GLFW_PRESS;
	}

	glm::vec2 Input::cursorPosition()
	{
		double xPos, yPos;
		glfwGetCursorPos(mWindow, &xPos, &yPos);
		return { xPos, yPos };
	}

	void Input::setInputMode(int mode, int value)
	{
		glfwSetInputMode(mWindow, mode, value);
	}

} // namespace vre

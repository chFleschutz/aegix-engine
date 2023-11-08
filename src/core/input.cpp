#include "input.h"


Input& Input::instance()
{
	static Input instance;
	return instance;
}

void Input::initialize(GLFWwindow* window)
{
	m_window = window;

	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

bool Input::keyPressed(int key)
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::mouseButtonPressed(int button)
{
	return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

Vector2 Input::cursorPosition()
{
	double xPos, yPos;
	glfwGetCursorPos(m_window, &xPos, &yPos);
	return { xPos, yPos };
}

void Input::setInputMode(int mode, int value)
{
	glfwSetInputMode(m_window, mode, value);
}

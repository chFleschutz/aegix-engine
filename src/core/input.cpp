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

	// Set up key callback
	glfwSetKeyCallback(m_window, [](GLFWwindow*, int key, int scancode, int action, int mods)
		{
			instance().glfwKeyCallback(key, scancode, static_cast<KeyEvent>(action), static_cast<Modifier>(mods));
		});
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

void Input::glfwKeyCallback(int key, int scancode, KeyEvent action, Modifier mods)
{
	for (const auto& binding : m_keyBindings[key])
	{
		if (binding.event == action and binding.mods == mods)
			binding.function();
	}
}
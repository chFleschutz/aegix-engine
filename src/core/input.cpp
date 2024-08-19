#include "input.h"

#include "graphics/window.h"


#include <cassert>

namespace Aegix
{
	Input* Input::s_instance = nullptr;

	Input::Input(const Graphics::Window& window)
		: m_window{ window.glfwWindow() }
	{
		assert(s_instance == nullptr && "Cannot create Input: Only one instance of Input is allowed");
		s_instance = this;

		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		// Set up key callback
		glfwSetKeyCallback(m_window, [](GLFWwindow*, int key, int scancode, int action, int mods)
			{
				Input::instance().glfwKeyCallback(key, scancode, static_cast<KeyEvent>(action), static_cast<Modifier>(mods));
			});
	}

	Input::~Input()
	{
		s_instance = nullptr;
	}

	Input& Input::instance()
	{
		assert(s_instance != nullptr && "Input instance is not created");
		return *s_instance;
	}

	bool Input::keyPressed(Key key)
	{
		return glfwGetKey(m_window, key) == GLFW_PRESS;
	}

	bool Input::mouseButtonPressed(MouseButton button)
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
}

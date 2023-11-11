#pragma once

#include "graphics/window.h"
#include "utils/math_utils.h"

#include <functional>
#include <unordered_map>

class Input
{
public:
	/// @brief Key events
	enum KeyEvent
	{
		Press = GLFW_PRESS,
		Release = GLFW_RELEASE,
		Repeat = GLFW_REPEAT
	};

	/// @brief Modifier keys 
	/// @note Can be combined with bitwise OR
	enum Modifier
	{
		None = 0,
		Shift = GLFW_MOD_SHIFT,
		Control = GLFW_MOD_CONTROL,
		Alt = GLFW_MOD_ALT,
		Super = GLFW_MOD_SUPER,
		CapsLock = GLFW_MOD_CAPS_LOCK,	
		NumLock = GLFW_MOD_NUM_LOCK
	};

	Input(const Input&) = delete;
	void operator=(const Input&) = delete;

	/// @brief Acces to the instance of Input
	/// @return Returns a reference to the instance of Input
	/// @note Make sure Input was initialized 
	static Input& instance();

	/// @brief Initializes Input
	/// @param window Pointer to the GLFW window
	void initialize(GLFWwindow* window);

	/// @brief Returns the state of key
	/// @param key Keycode of the key
	/// @return Returns true if the key is pressed otherwise false
	bool keyPressed(int key);

	/// @brief Returns the state of button
	/// @param button Buttoncode of the mousebutton
	/// @return Returns true if the button is pressed otherwise false
	bool mouseButtonPressed(int button);

	/// @brief Retrieves the mouse cursor position
	/// @return Returns a vec2 with the cursor position
	Vector2 cursorPosition();

	/// @brief Sets the input mode
	/// @param mode The mode to change
	/// @param value The new value for the mode
	void setInputMode(int mode, int value);

	/// @brief Binds a function to a key
	/// @tparam T Type of the class that contains the function
	/// @param instance Pointer to the instance of the class
	/// @param func Function to bind
	/// @param key to bind the function to (A, B, C, etc)
	/// @param event which triggers the function (press, release, etc)
	/// @param mod Any modifier keys that should be pressed (shift, control, etc)
	/// @note The function will be executed in glfwPollEvents
	template<typename T>
	void bind(T* instance, void (T::* func)(), int key, KeyEvent event = Press, Modifier mod = None)
	{
		m_keyBindings[key].push_back({ std::bind(func, instance), event, mod });
	}

private:
	struct Binding
	{
		std::function<void()> function;
		KeyEvent event;
		Modifier mods;
	};

	Input() = default;

	/// @brief Calls the bound functions for the key
	void glfwKeyCallback(int key, int scancode, KeyEvent action, Modifier mods);

	GLFWwindow* m_window = nullptr;

	std::unordered_map<int, std::vector<Binding>> m_keyBindings;
};

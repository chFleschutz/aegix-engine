#pragma once

#include "window.h"

#include <glm/glm.hpp>

namespace vre
{
	class Input
	{
	public:
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
		glm::vec2 cursorPosition();

		/// @brief Sets the input mode
		/// @param mode The mode to change
		/// @param value The new value for the mode
		void setInputMode(int mode, int value);

	private:
		Input() {}

		GLFWwindow* mWindow = nullptr;
	};

} // namespace vre


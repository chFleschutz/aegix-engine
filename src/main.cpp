#include "vulkanite_engine.h"
#include "camera.h"

#include <iostream>
#include <stdexcept>

auto main() -> int 
{
	vre::VulkaniteEngine app{};

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
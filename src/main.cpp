#include "core/vulkanite_engine.h"
#include "scene/default_scene.h"

#include <iostream>
#include <stdexcept>

auto main() -> int 
{
	try
	{
		vre::VulkaniteEngine engine{};
		engine.loadScene<DefaultScene>();
		engine.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
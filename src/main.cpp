#include "core/vulkanite_engine.h"
#include "ai/ai_scene.h"
#include "ai/swarm_example/swarm_scene.h"

#include <iostream>
#include <stdexcept>

auto main() -> int 
{
	try
	{
		Vulkanite::Engine engine{};
		engine.loadScene<AIScene>();
		engine.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
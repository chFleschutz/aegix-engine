#include "core/vulkanite_engine.h"

#include "ai/ai_scene.h"
#include "ai/swarm_example/swarm_scene.h"
#include "ai/decision_tree_example/decision_scene.h"

#include <iostream>
#include <stdexcept>

int main()
{
	try
	{
		Vulkanite::Engine engine{};
		engine.loadScene<DecisionTreeScene>();
		engine.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#include "core/engine.h"

#include "template_scene.h"

#include <iostream>
#include <stdexcept>

int main()
{
	try
	{
		Aegix::Engine engine{};
		engine.loadScene<TemplateScene>();
		engine.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
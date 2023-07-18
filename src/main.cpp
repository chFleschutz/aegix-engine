#include "vre_application.h"

#include <iostream>
#include <stdexcept>
#include "vre_camera.h"

auto main() -> int 
{
	vre::VreApplication app{};

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
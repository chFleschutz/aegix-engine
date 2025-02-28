#include "core/engine.h"

#include "simple_scene.h"

auto main() -> int
{
	Aegix::Engine engine{};
	engine.loadScene<SimpleScene>();
	engine.run();
}
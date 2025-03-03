#include "core/engine.h"
#include "helmet_scene.h"

auto main() -> int
{
	Aegix::Engine engine;
	engine.loadScene<HelmetScene>();
	engine.run();
}
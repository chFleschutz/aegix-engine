#include "core/engine.h"

#include "pbs_scene.h"

auto main() -> int
{
	Aegix::Engine engine{};
	engine.loadScene<PBSScene>();
	engine.run();
}
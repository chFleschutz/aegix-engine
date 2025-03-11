#include "core/engine.h"

#include "simple_scene.h"

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<SimpleScene>();
	engine.run();
}

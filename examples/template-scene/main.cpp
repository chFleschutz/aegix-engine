#include "core/engine.h"

#include "template_scene.h"

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<TemplateScene>();
	engine.run();
}

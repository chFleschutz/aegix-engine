#include "core/engine.h"

#include "template_scene.h"

auto main() -> int
{
	Aegix::Engine engine{};
	engine.loadScene<TemplateScene>();
	engine.run();
}
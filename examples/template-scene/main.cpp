#include "core/engine.h"
#include "scene/description.h"

class TemplateScene : public Aegix::Scene::Description
{
public:
	/// @brief All objects in a scene are created here
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		// Create your scene here 

	}
};

auto main() -> int
{
	auto& engine = Aegix::Engine::instance();
	engine.loadScene<TemplateScene>();
	engine.run();
}

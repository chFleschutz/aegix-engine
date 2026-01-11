#include "aegix/engine.h"
#include "aegix/scene/description.h"

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
	Aegix::Engine engine;
	engine.loadScene<TemplateScene>();
	engine.run();
}

#include "ai_component.h"

#include "ai/knowledge.h"
#include "ai/options/steering_behaviour/steering_behaviour_seek.h"
#include "core/input.h"

namespace VEAI
{
    AIComponent::AIComponent(VEScene::Entity m_player)
		: m_player(m_player)
	{
        auto& input = Input::instance();
        input.bind(this, &AIComponent::seekPlayer, GLFW_KEY_1);
        input.bind(this, &AIComponent::startPauseOption, GLFW_KEY_SPACE);
        input.bind(this, &AIComponent::stopOption, GLFW_KEY_ESCAPE);
	}

    void AIComponent::update(float deltaSeconds)
    {
        m_optionManager.update(deltaSeconds);
    }

    void AIComponent::startPauseOption()
    {
        auto activeOption = m_optionManager.activeOption();
        if (!activeOption)
            return;

        if (activeOption->isActive())
        {
            activeOption->pause();
        }
        else
        {
            activeOption->start();
        }
    }

    void AIComponent::stopOption()
    {
        m_optionManager.cancelActive();
    }

    void AIComponent::seekPlayer()
    {
        m_optionManager.cancelActive();
        auto& seekOption = m_optionManager.emplacePrioritized<SteeringBehaviourSeek>(this);
        seekOption.setTarget(EntityKnowledge{ m_player });
    }

} // namespace VEAI

#include "ai_component.h"

#include "ai/knowledge.h"
#include "ai/options/steering_behaviour/steering_behaviour_flee.h"
#include "ai/options/steering_behaviour/steering_behaviour_seek.h"
#include "core/input.h"

namespace VEAI
{
    AIComponent::AIComponent(VEScene::Entity m_player)
		: m_player(m_player)
	{
        auto& input = Input::instance();
        input.bind(this, &AIComponent::seekPlayer, Input::One);
        input.bind(this, &AIComponent::fleeFromPlayer, Input::Two);
        input.bind(this, &AIComponent::startPauseOption, Input::Space);
        input.bind(this, &AIComponent::stopOption, Input::Escape);
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

    void AIComponent::fleeFromPlayer()
    {
        m_optionManager.cancelActive();
		auto& fleeOption = m_optionManager.emplacePrioritized<SteeringBehaviourFlee>(this);
		fleeOption.setTarget(EntityKnowledge{ m_player });
    }

} // namespace VEAI

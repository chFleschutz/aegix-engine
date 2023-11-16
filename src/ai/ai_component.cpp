#include "ai_component.h"

#include "ai/knowledge.h"
#include "ai/options/steering_behaviour/steering_behaviour_arrive.h"
#include "ai/options/steering_behaviour/steering_behaviour_flee.h"
#include "ai/options/steering_behaviour/steering_behaviour_seek.h"
#include "ai/options/steering_behaviour/steering_behaviour_wander.h"
#include "ai/options/steering_behaviour/steering_behaviour_separation.h"
#include "ai/options/steering_behaviour/steering_behaviour_cohesion.h"
#include "ai/options/steering_behaviour/steering_behaviour_velocity_matching.h"
#include "ai/options/steering_behaviour/steering_behaviour_flocking.h"
#include "core/input.h"

namespace VEAI
{
    AIComponent::AIComponent(VEScene::Entity player, std::vector<VEScene::Entity> npcs)
		: m_player(player),
        m_npcs(npcs)
	{
        auto& input = Input::instance();
        input.bind(this, &AIComponent::seekPlayer, Input::One);
        input.bind(this, &AIComponent::fleeFromPlayer, Input::Two);
        input.bind(this, &AIComponent::arriveAtPlayer, Input::Three);
        input.bind(this, &AIComponent::flockingWander, Input::Four);
        input.bind(this, &AIComponent::flockingSeek, Input::Five);
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
            std::cout << getComponent<VEComponent::Name>().name << ": Paused option" << std::endl;
        }
        else
        {
            activeOption->start();
            std::cout << getComponent<VEComponent::Name>().name << ": Started option" << std::endl;
        }
    }

    void AIComponent::stopOption()
    {
        m_optionManager.cancelActive();

        std::cout << getComponent<VEComponent::Name>().name << ": Stopping option" << std::endl;
    }

    void AIComponent::seekPlayer()
    {
        m_optionManager.cancelActive();
        auto& seekOption = m_optionManager.emplacePrioritized<SteeringBehaviourSeek>(this, EntityKnowledge{ m_player });

        std::cout << getComponent<VEComponent::Name>().name << ": Seeking player" << std::endl;
    }

    void AIComponent::fleeFromPlayer()
    {
        m_optionManager.cancelActive();
		auto& fleeOption = m_optionManager.emplacePrioritized<SteeringBehaviourFlee>(this);
		fleeOption.setTarget(EntityKnowledge{ m_player });

        std::cout << getComponent<VEComponent::Name>().name << ": Fleeing from player" << std::endl;
    }

    void AIComponent::arriveAtPlayer()
    {
        m_optionManager.cancelActive();
		auto& arriveOption = m_optionManager.emplacePrioritized<SteeringBehaviourArrive>(this);
        arriveOption.setTarget(EntityKnowledge{ m_player });

        std::cout << getComponent<VEComponent::Name>().name << ": Arriving at player" << std::endl;
    }

    void AIComponent::flockingWander()
    {
        m_optionManager.cancelActive();

        auto& blendOption = m_optionManager.emplacePrioritized<SteeringBehaviourBlend>(this);
        blendOption.add<SteeringBehaviourWander>(1.0f, this);
        blendOption.add<SteeringBehaviourFlocking>(1.0f, this, EntityGroupKnowledge{ m_npcs });

        std::cout << getComponent<VEComponent::Name>().name << ": Flocking wander" << std::endl;
    }

    void AIComponent::flockingSeek()
    {
        m_optionManager.cancelActive();
        auto& blendOption = m_optionManager.emplacePrioritized<SteeringBehaviourBlend>(this);
        blendOption.add<SteeringBehaviourSeek>(1.0f, this, EntityKnowledge{ m_player });
        blendOption.add<SteeringBehaviourFlocking>(1.0f, this, EntityGroupKnowledge{ m_npcs });

        std::cout << getComponent<VEComponent::Name>().name << ": Flocking seek" << std::endl;
    }

} // namespace VEAI

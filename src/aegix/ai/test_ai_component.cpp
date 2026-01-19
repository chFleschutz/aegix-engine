#include "test_ai_component.h"

#include "ai/knowledge.h"
#include "ai/options/steering_behaviour/steering_behaviour_arrive.h"
#include "ai/options/steering_behaviour/steering_behaviour_cohesion.h"
#include "ai/options/steering_behaviour/steering_behaviour_flee.h"
#include "ai/options/steering_behaviour/steering_behaviour_flocking.h"
#include "ai/options/steering_behaviour/steering_behaviour_grappling_hooks.h"
#include "ai/options/steering_behaviour/steering_behaviour_seek.h"
#include "ai/options/steering_behaviour/steering_behaviour_separation.h"
#include "ai/options/steering_behaviour/steering_behaviour_velocity_matching.h"
#include "ai/options/steering_behaviour/steering_behaviour_wander.h"
#include "core/input.h"

namespace Aegis::AI
{
    TestAIComponent::TestAIComponent(Blackboard& blackboard) : AIComponent(blackboard)
    {
        auto& input = Aegis::Input::instance();
        input.bind(this, &TestAIComponent::seekPlayer, Input::One);
        input.bind(this, &TestAIComponent::fleeFromPlayer, Input::Two);
        input.bind(this, &TestAIComponent::arriveAtPlayer, Input::Three);
        input.bind(this, &TestAIComponent::flockingWander, Input::Four);
        input.bind(this, &TestAIComponent::flockingSeek, Input::Five);
        input.bind(this, &TestAIComponent::followPath, Input::Six);
        input.bind(this, &TestAIComponent::startPauseOption, Input::Space);
        input.bind(this, &TestAIComponent::stopOption, Input::Escape);
    }

    void TestAIComponent::begin()
    {
        m_player = m_blackboard.get<EntityKnowledge>("Player");
        m_npcs = m_blackboard.get<EntityGroupKnowledge>("NPCs");

        assert(m_player && "Player is not set correctly (check blackboard)");
        assert(m_npcs && "NPCs are not set correctly (check blackboard)");
    }

    void TestAIComponent::startPauseOption()
    {
        auto activeOption = m_optionManager.activeOption();
        if (!activeOption)
            return;

        if (activeOption->isActive())
        {
            activeOption->pause();
            std::cout << getComponent<Aegis::Component::Name>().name << ": Paused option" << std::endl;
        }
        else
        {
            activeOption->start();
            std::cout << getComponent<Aegis::Component::Name>().name << ": Started option" << std::endl;
        }
    }

    void TestAIComponent::stopOption()
    {
        m_optionManager.cancelActive();

        std::cout << getComponent<Aegis::Component::Name>().name << ": Stopping option" << std::endl;
    }

    void TestAIComponent::seekPlayer()
    {
        m_optionManager.cancelActive();
        auto& seekOption = m_optionManager.emplacePrioritized<SteeringBehaviourSeek>(this, EntityKnowledge{ *m_player });

        std::cout << getComponent<Aegis::Component::Name>().name << ": Seeking player" << std::endl;
    }

    void TestAIComponent::fleeFromPlayer()
    {
        m_optionManager.cancelActive();
        auto& fleeOption = m_optionManager.emplacePrioritized<SteeringBehaviourFlee>(this);
        fleeOption.setTarget(EntityKnowledge{ *m_player });

        std::cout << getComponent<Aegis::Component::Name>().name << ": Fleeing from player" << std::endl;
    }

    void TestAIComponent::arriveAtPlayer()
    {
        m_optionManager.cancelActive();
        auto& arriveOption = m_optionManager.emplacePrioritized<SteeringBehaviourArrive>(this);
        arriveOption.setTarget(EntityKnowledge{ *m_player });

        std::cout << getComponent<Aegis::Component::Name>().name << ": Arriving at player" << std::endl;
    }

    void TestAIComponent::flockingWander()
    {
        m_optionManager.cancelActive();

        auto& blendOption = m_optionManager.emplacePrioritized<SteeringBehaviourBlend>(this);
        blendOption.add<SteeringBehaviourWander>(1.0f, this);
		blendOption.add<SteeringBehaviourFlocking>(1.0f, this, *m_npcs);

        std::cout << getComponent<Aegis::Component::Name>().name << ": Flocking wander" << std::endl;
    }

    void TestAIComponent::flockingSeek()
    {
        m_optionManager.cancelActive();
        auto& blendOption = m_optionManager.emplacePrioritized<SteeringBehaviourBlend>(this);
        blendOption.add<SteeringBehaviourSeek>(1.0f, this, *m_player);
        blendOption.add<SteeringBehaviourFlocking>(1.0f, this, *m_npcs);

        std::cout << getComponent<Aegis::Component::Name>().name << ": Flocking seek" << std::endl;
    }

    void TestAIComponent::followPath()
    {
        m_optionManager.cancelActive();

        PathKnowledge path{};
        path.path.emplace_back(glm::vec3{ 10.0f, 0.0f, 10.0f });
        path.path.emplace_back(glm::vec3{ -10.0f, 0.0f, 10.0f });
        path.path.emplace_back(glm::vec3{ -10.0f, 0.0f, -10.0f });
        path.path.emplace_back(glm::vec3{ 10.0f, 0.0f, -10.0f });
        m_optionManager.emplacePrioritized<SteeringBehaviourGrapplingHooks>(this, path);

        std::cout << getComponent<Aegis::Component::Name>().name << ": Flollow path" << std::endl;
    }
}

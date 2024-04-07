#pragma once

#include "ai/knowledge.h"
#include "ai/ai_component.h"
#include "ai/option_manager.h"
#include "scene/entity.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class TestAIComponent : public AIComponent
    {
    public:
        TestAIComponent(Blackboard& blackboardBloard);

        void begin() override;

        // Key callbacks
        void startPauseOption();
        void stopOption();
        void seekPlayer();
        void fleeFromPlayer();
        void arriveAtPlayer();
        void flockingWander();
        void flockingSeek();
        void followPath();

    private:
        EntityKnowledge* m_player = nullptr;
        EntityGroupKnowledge* m_npcs = nullptr; 
    };

} // namespace VEAI
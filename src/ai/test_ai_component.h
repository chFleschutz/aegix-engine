#pragma once

#include "ai/ai_component.h"
#include "ai/option_manager.h"
#include "scene/entity.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class TestAIComponent : public AIComponent
    {
    public:
        TestAIComponent(VEScene::Entity player, std::vector<VEScene::Entity> npcs);

        // Key callbacks
        void startPauseOption();
        void stopOption();
        void seekPlayer();
        void fleeFromPlayer();
        void arriveAtPlayer();
        void flockingWander();
        void flockingSeek();

    private:
        VEScene::Entity m_player;
        std::vector<VEScene::Entity> m_npcs; // Referenced by EntityGroupKnowledge
    };

} // namespace VEAI
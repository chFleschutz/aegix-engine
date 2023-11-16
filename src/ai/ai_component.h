#pragma once

#include "ai/option_manager.h"
#include "scene/entity.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class AIComponent : public VEScripting::ScriptBase
    {
    public:
        AIComponent(VEScene::Entity player, std::vector<VEScene::Entity> npcs);
        ~AIComponent() = default;

        void update(float deltaSeconds) override;

        // Key callbacks
        void startPauseOption();
        void stopOption();
        void seekPlayer();
        void fleeFromPlayer();
        void arriveAtPlayer();
        void flockingWander();
        void flockingSeek();

    private:
        OptionManager m_optionManager;

        VEScene::Entity m_player;
        std::vector<VEScene::Entity> m_npcs; // Referenced by EntityGroupKnowledge
    };

} // namespace VEAI
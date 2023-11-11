#pragma once

#include "ai/option_manager.h"
#include "scene/entity.h"
#include "scripting/script_base.h"

namespace VEAI
{
    class AIComponent : public VEScripting::ScriptBase
    {
    public:
        AIComponent(VEScene::Entity m_player);
        ~AIComponent() = default;

        void update(float deltaSeconds) override;

        // Key callbacks
        void startPauseOption();
        void stopOption();
        void seekPlayer();
        void fleeFromPlayer();

    private:
        OptionManager m_optionManager;

        VEScene::Entity m_player;
    };

} // namespace VEAI
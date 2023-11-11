#include "ai_component.h"

#include "ai/options/option_text_output.h"
#include "core/input.h"

namespace VEAI
{
    AIComponent::AIComponent()
    {
        m_optionManager.emplaceQueued<OptionTextOutput>(this, 5.0f, 2.0f);
    }

    void AIComponent::update(float deltaSeconds)
    {
        handleInput();

        m_optionManager.update(deltaSeconds);
    }

    void AIComponent::handleInput()
    {
        // Check for key press (compare to lastFrame to trigger only once)
        static bool lastFramePressed = false;
        bool keyPressed = Input::instance().keyPressed(GLFW_KEY_SPACE);
        
        if (keyPressed and !lastFramePressed)
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

        lastFramePressed = keyPressed;
    }

} // namespace VEAI

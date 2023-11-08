#include "ai_component.h"

#include "ai/options/option_text_output.h"
#include "core/input.h"

namespace VEAI
{
    AIComponent::AIComponent()
    {
        m_option = std::make_unique<OptionTextOutput>();
        m_option->setDuration(3.0f);
        m_option->setCooldown(1.0f);
    }

    void AIComponent::update(float deltaSeconds)
    {
        // Check for key press (compare to lastFrame to trigger only once)
        static bool lastFramePressed = false;
        bool keyPressed = Input::instance().keyPressed(GLFW_KEY_SPACE);
        if (keyPressed and !lastFramePressed)
        {
            if (m_option->isActive())
            {
                m_option->pause();
            }
            else
            {
                m_option->start();
            }
        }
        lastFramePressed = keyPressed;

        m_option->update(deltaSeconds);
    }

} // namespace VEAI

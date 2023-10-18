#include "ai-component.h"

#include "ai/option.h"
#include "core/input.h"

namespace vai
{
    AIComponent::AIComponent()
    {
        m_option.setDuration(1.0f);
        m_option.setCooldown(0.2f);
    }

    void AIComponent::update(float deltaSeconds)
    {
        // Check for key press (compare to lastFrame to trigger only once)
        static bool lastFramePressed = false;
        bool keyPressed = vre::Input::instance().keyPressed(GLFW_KEY_SPACE);
        if (keyPressed and !lastFramePressed)
        {
            if (m_option.isActive())
            {
                m_option.pause();
            }
            else
            {
                m_option.start();
            }
        }
        lastFramePressed = keyPressed;

        m_option.update(deltaSeconds);
    }

} // namespace vai

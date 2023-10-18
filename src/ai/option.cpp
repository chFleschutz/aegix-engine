#include "option.h"

namespace vai
{
    void Option::setDuration(float seconds)
    {
        m_duration = seconds;
    }

    void Option::setCooldown(float seconds)
    {
        m_cooldown = seconds;
    }

    void Option::start()
    {
        if (m_onCooldown)
        {
            std::cout << "Option on cooldown" << std::endl;
            return;
        }

        m_isActive = true;
        std::cout << "Option started" << std::endl;
    }

    void Option::pause()
    {
        m_isActive = false;
        m_onCooldown = true;
        std::cout << "Option paused" << std::endl;
    }

    void Option::stop()
    {
        m_isActive = false;
        m_elapsedTime = 0.0f;
        std::cout << "Option stopped" << std::endl;
    }   

    void Option::update(float deltaSeconds)
    {
        // Update cooldown
        if (m_onCooldown)
        {
            m_cooldownTime += deltaSeconds;
            if (m_cooldownTime > m_cooldown)
            {
                m_onCooldown = false;
                m_cooldownTime = 0.0f;
            }
        }

        // Update option 
        if (m_isActive)
        {
            m_elapsedTime += deltaSeconds;
            updateOption(deltaSeconds);

            if (m_elapsedTime > m_duration)
                stop();
        }
    }

} // namespace vai
#pragma once

#include <iostream>

namespace vai
{
    class Option
    {
    public:
        Option() = default;
        virtual ~Option() = default;    

        void setDuration(float seconds);
        void setCooldown(float seconds);

        /// @brief Returns if the option if currently active
        /// @return True if the option is active otherwise false
        bool isActive() const { return m_isActive; }

        /// @brief Start the option
        void start();
        /// @brief Deactivates the option
        void pause();
        /// @brief Deactivates the option and resets it
        void stop();

        /// @brief Updates the option
        void update(float deltaSeconds);
        
        /// @brief Function is called once per frame if the option is active
        /// @param deltaSeconds Seconds between frames
        /// @note Override this function to implement custom behaviour
        virtual void updateOption(float deltaSeconds) {
            std::cout << "Option is active since " << m_elapsedTime << std::endl;
        }

    private:
        bool m_isActive = false;
        float m_duration = 0.0f;
        float m_elapsedTime = 0.0f;

        bool m_onCooldown = false;
        float m_cooldown = 0.0f;
        float m_cooldownTime = 0.0f;
    };
    
} // namespace vai
#pragma once

#include "ai/options/option.h"

#include <iostream>

namespace VEAI
{
	/// @brief Debug option that prints to the console
	class OptionTextOutput : public Option
	{
	public:
		void start() override
		{
			Option::start();

			if (onCooldown())
			{
				std::cout << "Option on Cooldown" << std::endl;
				return;
			}

			std::cout << "Option started" << std::endl;
		}

		void pause() override
		{
			Option::pause();
			std::cout << "Option paused" << std::endl;
		}

		void stop() override
		{
			Option::stop();
			std::cout << "Option stopped" << std::endl;
		}

		void updateOption(float deltaSeconds) override
		{
			std::cout << "Option active for " << elapsedTime() << " seconds" << std::endl;
		}
	};

} // namespace VEAI
#pragma once

#include "ai/options/option.h"

namespace Aegix::AI
{
    class SleepOption : public Option
    {
    public:
        SleepOption(AIComponent* ai) : Option(ai) {}
        void start() override
        {
            Option::start();
            std::cout << "Sleeping... zZz ZzZ zZz" << std::endl;
        }
    };

    class PatrolOption : public Option
    {
    public:
        PatrolOption(AIComponent* ai) : Option(ai) {}
        void start() override
        {
            Option::start();
            std::cout << "Patrolling..." << std::endl;
        }
    };

    class GreetOption : public Option
    {
    public:
        GreetOption(AIComponent* ai) : Option(ai) {}
        void start() override
        {
			Option::start();
			std::cout << "Greetings my friend" << std::endl;
        }
    };

    class MurderOption : public Option
    {
    public:
        MurderOption(AIComponent* ai) : Option(ai) {}
        void start() override
        {
            Option::start();
            std::cout << "Murdering... Muhahahaaa" << std::endl;
        }
    };
} 

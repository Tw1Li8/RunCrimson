#pragma once

#include <chrono>

namespace Engine
{
    class Time
    {
    public:
        void Reset();
        float Tick();
        float DeltaTime() const { return deltaTime; }

    private:
        std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();
        float deltaTime = 0.0f;
    };
}

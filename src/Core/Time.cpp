#include "Core/Time.h"

namespace Engine
{
    void Time::Reset()
    {
        previousTime = std::chrono::steady_clock::now();
        deltaTime = 0.0f;
    }

    float Time::Tick()
    {
        const auto currentTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
        previousTime = currentTime;
        return deltaTime;
    }
}

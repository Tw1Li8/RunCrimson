#pragma once

#include <array>
#include <windows.h>

namespace Engine
{
    class Input
    {
    public:
        static void Update();
        static bool IsKeyDown(int virtualKey);
        static bool WasKeyPressed(int virtualKey);
        static bool WasKeyReleased(int virtualKey);

    private:
        static std::array<bool, 256> current;
        static std::array<bool, 256> previous;
    };
}

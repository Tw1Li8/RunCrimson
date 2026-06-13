#include "Input/Input.h"

namespace Engine
{
    std::array<bool, 256> Input::current = {};
    std::array<bool, 256> Input::previous = {};

    void Input::Update()
    {
        previous = current;
        for (int key = 0; key < 256; ++key)
        {
            current[key] = (GetAsyncKeyState(key) & 0x8000) != 0;
        }
    }

    bool Input::IsKeyDown(int virtualKey)
    {
        return virtualKey >= 0 && virtualKey < 256 && current[virtualKey];
    }

    bool Input::WasKeyPressed(int virtualKey)
    {
        return virtualKey >= 0 && virtualKey < 256 && current[virtualKey] && !previous[virtualKey];
    }

    bool Input::WasKeyReleased(int virtualKey)
    {
        return virtualKey >= 0 && virtualKey < 256 && !current[virtualKey] && previous[virtualKey];
    }
}

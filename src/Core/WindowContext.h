#pragma once

#include <windows.h>

namespace Engine
{
    class WindowContext
    {
    public:
        bool Initialize(HINSTANCE instance, int width, int height, const wchar_t* title, WNDPROC windowProc);
        void Shutdown();

        HWND Handle() const { return hwnd; }
        int Width() const { return width; }
        int Height() const { return height; }
        void SetSize(int newWidth, int newHeight);

    private:
        HWND hwnd = nullptr;
        HINSTANCE instance = nullptr;
        int width = 1280;
        int height = 720;
        const wchar_t* className = L"RunnerEngineWindow";
    };
}

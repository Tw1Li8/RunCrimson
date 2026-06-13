#include "Core/WindowContext.h"

namespace Engine
{
    bool WindowContext::Initialize(HINSTANCE appInstance, int requestedWidth, int requestedHeight, const wchar_t* title, WNDPROC windowProc)
    {
        instance = appInstance;
        width = requestedWidth;
        height = requestedHeight;

        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = windowProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = className;

        RegisterClassEx(&wc);

        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        hwnd = CreateWindow(
            className,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr,
            nullptr,
            instance,
            nullptr);

        if (!hwnd)
        {
            return false;
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
        return true;
    }

    void WindowContext::SetSize(int newWidth, int newHeight)
    {
        width = newWidth;
        height = newHeight;
    }

    void WindowContext::Shutdown()
    {
        if (hwnd)
        {
            DestroyWindow(hwnd);
            hwnd = nullptr;
        }
        UnregisterClass(className, instance);
    }
}

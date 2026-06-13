#pragma once

#include <memory>
#include <windows.h>
#include "Core/Time.h"
#include "Core/WindowContext.h"
#include "Graphics/GraphicsContext.h"

namespace Engine
{
    class Scene;

    class Application
    {
    public:
        bool Initialize(HINSTANCE instance, std::unique_ptr<Scene> startScene);
        int Run();
        void RequestQuit() { isRunning = false; }
        void OnResize(int width, int height);

        static Application* Instance() { return instance; }
        GraphicsContext& Graphics() { return graphics; }

    private:
        static Application* instance;
        bool isRunning = true;
        WindowContext window;
        GraphicsContext graphics;
        Time time;
        std::unique_ptr<Scene> scene;
    };
}

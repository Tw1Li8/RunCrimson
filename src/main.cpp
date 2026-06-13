#include <memory>
#include <windows.h>
#include "Core/Application.h"
#include "Game/RunnerGameScene.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int)
{
    Engine::Application app;
    auto scene = std::make_unique<Engine::RunnerGameScene>();

    if (!app.Initialize(instance, std::move(scene)))
    {
        return -1;
    }

    return app.Run();
}
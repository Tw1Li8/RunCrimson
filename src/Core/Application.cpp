#include "Core/Application.h"
#include "Input/Input.h"
#include "Scene/Scene.h"

namespace Engine
{
    Application* Application::instance = nullptr;

    static LRESULT CALLBACK AppWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_DESTROY)
        {
            PostQuitMessage(0);
            return 0;
        }

        if (message == WM_SIZE && Application::Instance())
        {
            const int width = LOWORD(lParam);
            const int height = HIWORD(lParam);
            if (width > 0 && height > 0)
            {
                Application::Instance()->OnResize(width, height);
            }
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    bool Application::Initialize(HINSTANCE appInstance, std::unique_ptr<Scene> startScene)
    {
        instance = this;
        scene = std::move(startScene);

        if (!window.Initialize(appInstance, 1280, 720, L"Runner Engine - Basic DX11 Runner", AppWindowProc))
        {
            return false;
        }

        if (!graphics.Initialize(window.Handle(), window.Width(), window.Height()))
        {
            return false;
        }

        scene->Initialize(graphics);
        time.Reset();
        return true;
    }

    int Application::Run()
    {
        MSG message = {};
        while (isRunning)
        {
            if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
            {
                if (message.message == WM_QUIT)
                {
                    isRunning = false;
                }
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            else
            {
                Input::Update();
                if (Input::IsKeyDown(VK_ESCAPE))
                {
                    RequestQuit();
                }

                const float dt = time.Tick();
                scene->Update(dt);

                graphics.BeginFrame({ 0.08f, 0.10f, 0.14f, 1.0f });
                scene->Render(graphics);
                graphics.EndFrame();
            }
        }

        scene.reset();
        graphics.Shutdown();
        window.Shutdown();
        instance = nullptr;
        return 0;
    }

    void Application::OnResize(int width, int height)
    {
        window.SetSize(width, height);
        graphics.Resize(width, height);
    }
}

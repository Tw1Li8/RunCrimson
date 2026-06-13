#pragma once

#include <d3d11.h>
#include <string>
#include <vector>
#include <windows.h>
#include "Core/EngineTypes.h"

namespace Engine
{
    class GraphicsContext
    {
    public:
        bool Initialize(HWND hwnd, int width, int height);
        void Shutdown();
        void Resize(int width, int height);

        void BeginFrame(const Color& clearColor);
        void EndFrame();
        void QueueText(const std::wstring& text, int x, int y, int size, const Color& color);
        void DrawDebugRect(const Vec2& center, float rectWidth, float rectHeight, const Color& color);

        ID3D11Device* Device()  const { return device; }
        ID3D11DeviceContext* Context() const { return context; }

    private:
        struct DebugLineVertex
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            Color color;
        };

        struct TextCommand
        {
            std::wstring text;
            int   x = 0;
            int   y = 0;
            int   size = 24;
            Color color;
        };

        bool CreateRenderTarget();
        bool CreateDebugLineResources();
        bool CreateBlendState();          // 알파 블렌딩 스테이트 생성
        void DrawDebugLines();
        void DrawQueuedText();

        HWND                     hwnd = nullptr;
        ID3D11Device* device = nullptr;
        ID3D11DeviceContext* context = nullptr;
        IDXGISwapChain* swapChain = nullptr;
        ID3D11RenderTargetView* renderTarget = nullptr;
        ID3D11BlendState* blendState = nullptr; // 알파 블렌딩용
        ID3D11VertexShader* debugLineVertexShader = nullptr;
        ID3D11PixelShader* debugLinePixelShader = nullptr;
        ID3D11InputLayout* debugLineInputLayout = nullptr;
        ID3D11Buffer* debugLineVertexBuffer = nullptr;
        std::vector<TextCommand>      queuedText;
        std::vector<DebugLineVertex>  debugLineVertices;
        int width = 0;
        int height = 0;
    };
}
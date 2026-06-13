#include "Graphics/GraphicsContext.h"
#include <cstring>
#include <d3dcompiler.h>

namespace Engine
{
    bool GraphicsContext::Initialize(HWND hwnd, int backBufferWidth, int backBufferHeight)
    {
        this->hwnd = hwnd;
        width = backBufferWidth;
        height = backBufferHeight;

        DXGI_SWAP_CHAIN_DESC desc = {};
        desc.BufferCount = 1;
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;
        desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = hwnd;
        desc.SampleDesc.Count = 1;
        desc.Windowed = TRUE;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE;

        const HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &desc,
            &swapChain,
            &device,
            nullptr,
            &context);

        if (FAILED(hr))
        {
            return false;
        }

        if (!CreateRenderTarget())
        {
            return false;
        }
        if (!CreateBlendState())
        {
            return false;
        }
        return CreateDebugLineResources();
    }

    bool GraphicsContext::CreateRenderTarget()
    {
        ID3D11Texture2D* backBuffer = nullptr;
        HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr))
        {
            return false;
        }

        hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTarget);
        backBuffer->Release();
        return SUCCEEDED(hr);
    }

    void GraphicsContext::Resize(int newWidth, int newHeight)
    {
        if (!swapChain || newWidth == 0 || newHeight == 0)
        {
            return;
        }

        width = newWidth;
        height = newHeight;

        context->OMSetRenderTargets(0, nullptr, nullptr);
        if (renderTarget)
        {
            renderTarget->Release();
            renderTarget = nullptr;
        }

        swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        CreateRenderTarget();
    }

    void GraphicsContext::BeginFrame(const Color& clearColor)
    {
        debugLineVertices.clear();

        const float color[] = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
        context->ClearRenderTargetView(renderTarget, color);
        context->OMSetRenderTargets(1, &renderTarget, nullptr);

        // 알파 블렌딩 활성화 (스프라이트 투명 처리)
        if (blendState)
        {
            const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            context->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);
        }

        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context->RSSetViewports(1, &viewport);
    }

    void GraphicsContext::EndFrame()
    {
        DrawDebugLines();
        DrawQueuedText();
        swapChain->Present(1, 0);
    }

    void GraphicsContext::QueueText(const std::wstring& text, int x, int y, int size, const Color& color)
    {
        queuedText.push_back({ text, x, y, size, color });
    }

    void GraphicsContext::DrawDebugRect(const Vec2& center, float rectWidth, float rectHeight, const Color& color)
    {
        const float left = center.x - rectWidth * 0.5f;
        const float right = center.x + rectWidth * 0.5f;
        const float top = center.y + rectHeight * 0.5f;
        const float bottom = center.y - rectHeight * 0.5f;

        debugLineVertices.push_back({ left, top, 0.0f, color });
        debugLineVertices.push_back({ right, top, 0.0f, color });
        debugLineVertices.push_back({ right, top, 0.0f, color });
        debugLineVertices.push_back({ right, bottom, 0.0f, color });
        debugLineVertices.push_back({ right, bottom, 0.0f, color });
        debugLineVertices.push_back({ left, bottom, 0.0f, color });
        debugLineVertices.push_back({ left, bottom, 0.0f, color });
        debugLineVertices.push_back({ left, top, 0.0f, color });
    }

    bool GraphicsContext::CreateBlendState()
    {
        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        return SUCCEEDED(device->CreateBlendState(&blendDesc, &blendState));
    }

    bool GraphicsContext::CreateDebugLineResources()
    {
        const char* shaderSource =
            "struct VS_INPUT { float3 position : POSITION; float4 color : COLOR; };"
            "struct PS_INPUT { float4 position : SV_POSITION; float4 color : COLOR; };"
            "PS_INPUT VS(VS_INPUT input) {"
            "    PS_INPUT output;"
            "    output.position = float4(input.position, 1.0f);"
            "    output.color = input.color;"
            "    return output;"
            "}"
            "float4 PS(PS_INPUT input) : SV_Target { return input.color; }";

        ID3DBlob* vertexBlob = nullptr;
        ID3DBlob* pixelBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        HRESULT hr = D3DCompile(shaderSource, strlen(shaderSource), nullptr, nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vertexBlob, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob) { errorBlob->Release(); }
            return false;
        }

        hr = D3DCompile(shaderSource, strlen(shaderSource), nullptr, nullptr, nullptr, "PS", "ps_5_0", 0, 0, &pixelBlob, &errorBlob);
        if (FAILED(hr))
        {
            vertexBlob->Release();
            if (errorBlob) { errorBlob->Release(); }
            return false;
        }

        device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &debugLineVertexShader);
        device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &debugLinePixelShader);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        device->CreateInputLayout(layout, 2, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &debugLineInputLayout);

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(DebugLineVertex) * 512;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device->CreateBuffer(&bufferDesc, nullptr, &debugLineVertexBuffer);

        vertexBlob->Release();
        pixelBlob->Release();
        return debugLineVertexShader && debugLinePixelShader && debugLineInputLayout && debugLineVertexBuffer;
    }

    void GraphicsContext::DrawDebugLines()
    {
        if (debugLineVertices.empty() || !debugLineVertexBuffer || !debugLineInputLayout)
        {
            return;
        }

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        if (FAILED(context->Map(debugLineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
        {
            return;
        }

        const size_t maxVertexCount = 512;
        const size_t vertexCount = debugLineVertices.size() > maxVertexCount ? maxVertexCount : debugLineVertices.size();
        memcpy(mapped.pData, debugLineVertices.data(), sizeof(DebugLineVertex) * vertexCount);
        context->Unmap(debugLineVertexBuffer, 0);

        UINT stride = sizeof(DebugLineVertex);
        UINT offset = 0;
        context->IASetInputLayout(debugLineInputLayout);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->IASetVertexBuffers(0, 1, &debugLineVertexBuffer, &stride, &offset);
        context->VSSetShader(debugLineVertexShader, nullptr, 0);
        context->PSSetShader(debugLinePixelShader, nullptr, 0);
        context->Draw(static_cast<UINT>(vertexCount), 0);
    }

    void GraphicsContext::DrawQueuedText()
    {
        if (queuedText.empty() || !swapChain)
        {
            return;
        }

        context->OMSetRenderTargets(0, nullptr, nullptr);

        IDXGISurface1* backBufferSurface = nullptr;
        HRESULT hr = swapChain->GetBuffer(0, __uuidof(IDXGISurface1), reinterpret_cast<void**>(&backBufferSurface));
        if (FAILED(hr))
        {
            queuedText.clear();
            return;
        }

        HDC dc = nullptr;
        hr = backBufferSurface->GetDC(FALSE, &dc);
        if (FAILED(hr))
        {
            backBufferSurface->Release();
            queuedText.clear();
            return;
        }

        SetBkMode(dc, TRANSPARENT);

        for (const TextCommand& command : queuedText)
        {
            HFONT font = CreateFontW(
                command.size,
                0,
                0,
                0,
                FW_BOLD,
                FALSE,
                FALSE,
                FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                FIXED_PITCH | FF_DONTCARE,
                L"Consolas");

            HFONT oldFont = static_cast<HFONT>(SelectObject(dc, font));
            SetTextColor(dc, RGB(
                static_cast<int>(command.color.r * 255.0f),
                static_cast<int>(command.color.g * 255.0f),
                static_cast<int>(command.color.b * 255.0f)));

            TextOutW(dc, command.x, command.y, command.text.c_str(), static_cast<int>(command.text.size()));

            SelectObject(dc, oldFont);
            DeleteObject(font);
        }

        backBufferSurface->ReleaseDC(nullptr);
        backBufferSurface->Release();
        queuedText.clear();
    }

    void GraphicsContext::Shutdown()
    {
        if (debugLineVertexBuffer) { debugLineVertexBuffer->Release(); debugLineVertexBuffer = nullptr; }
        if (debugLineInputLayout) { debugLineInputLayout->Release();  debugLineInputLayout = nullptr; }
        if (debugLinePixelShader) { debugLinePixelShader->Release();  debugLinePixelShader = nullptr; }
        if (debugLineVertexShader) { debugLineVertexShader->Release(); debugLineVertexShader = nullptr; }
        if (blendState) { blendState->Release();            blendState = nullptr; }
        if (renderTarget) { renderTarget->Release();          renderTarget = nullptr; }
        if (swapChain) { swapChain->Release();             swapChain = nullptr; }
        if (context) { context->Release();               context = nullptr; }
        if (device) { device->Release();                device = nullptr; }
        hwnd = nullptr;
    }
}
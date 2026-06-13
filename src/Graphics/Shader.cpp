#include "Graphics/Shader.h"
#include <d3dcompiler.h>

namespace Engine
{
    bool Shader::LoadFromFile(ID3D11Device* device, const std::wstring& path)
    {
        ID3DBlob* vertexBlob = nullptr;
        ID3DBlob* pixelBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        HRESULT hr = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "VS", "vs_5_0", 0, 0, &vertexBlob, &errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob) { errorBlob->Release(); }
            return false;
        }

        hr = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "PS", "ps_5_0", 0, 0, &pixelBlob, &errorBlob);
        if (FAILED(hr))
        {
            vertexBlob->Release();
            if (errorBlob) { errorBlob->Release(); }
            return false;
        }

        device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &vertexShader);
        device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &pixelShader);

        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        device->CreateInputLayout(layout, 3, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &inputLayout);

        vertexBlob->Release();
        pixelBlob->Release();
        return vertexShader && pixelShader && inputLayout;
    }

    void Shader::Bind(ID3D11DeviceContext* context) const
    {
        context->IASetInputLayout(inputLayout);
        context->VSSetShader(vertexShader, nullptr, 0);
        context->PSSetShader(pixelShader, nullptr, 0);
    }

    void Shader::Release()
    {
        if (inputLayout) { inputLayout->Release(); inputLayout = nullptr; }
        if (vertexShader) { vertexShader->Release(); vertexShader = nullptr; }
        if (pixelShader) { pixelShader->Release(); pixelShader = nullptr; }
    }
}

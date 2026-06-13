#pragma once

#include <d3d11.h>
#include <string>

namespace Engine
{
    class Shader
    {
    public:
        bool LoadFromFile(ID3D11Device* device, const std::wstring& path);
        void Bind(ID3D11DeviceContext* context) const;
        void Release();

    private:
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11PixelShader* pixelShader = nullptr;
        ID3D11InputLayout* inputLayout = nullptr;
    };
}

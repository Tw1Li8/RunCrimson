#pragma once

#include <d3d11.h>
#include <vector>
#include "Core/EngineTypes.h"

namespace Engine
{
    struct Vertex
    {
        float x, y, z;
        float r, g, b, a;
        float u, v;
    };

    class Mesh
    {
    public:
        bool Create(ID3D11Device* device, const std::vector<Vertex>& vertices);
        void Bind(ID3D11DeviceContext* context) const;
        void Draw(ID3D11DeviceContext* context) const;
        void Release();

    private:
        ID3D11Buffer* vertexBuffer = nullptr;
        unsigned int vertexCount = 0;
    };
}

#include "Graphics/Mesh.h"

namespace Engine
{
    bool Mesh::Create(ID3D11Device* device, const std::vector<Vertex>& vertices)
    {
        vertexCount = static_cast<unsigned int>(vertices.size());

        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(Vertex) * vertexCount;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = vertices.data();

        return SUCCEEDED(device->CreateBuffer(&desc, &initData, &vertexBuffer));
    }

    void Mesh::Bind(ID3D11DeviceContext* context) const
    {
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    void Mesh::Draw(ID3D11DeviceContext* context) const
    {
        context->Draw(vertexCount, 0);
    }

    void Mesh::Release()
    {
        if (vertexBuffer)
        {
            vertexBuffer->Release();
            vertexBuffer = nullptr;
        }
    }
}

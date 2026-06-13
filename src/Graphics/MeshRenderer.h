#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "Object/Component.h"

namespace Engine
{
    class Material;
    class Mesh;

    class MeshRenderer : public Component
    {
    public:
        MeshRenderer(Mesh* mesh, Material* material);
        ~MeshRenderer() override;

        void SetMaterial(Material* newMaterial) { material = newMaterial; }
        void Start() override;
        void Render(GraphicsContext& graphics) override;

    private:
        struct TransformBuffer
        {
            DirectX::XMMATRIX world;
        };

        Mesh* mesh = nullptr;
        Material* material = nullptr;
        ID3D11Buffer* transformBuffer = nullptr;
    };
}

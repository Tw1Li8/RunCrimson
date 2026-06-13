#include "Graphics/MeshRenderer.h"
#include "Core/Application.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Object/GameObject.h"

using namespace DirectX;

namespace Engine
{
    MeshRenderer::MeshRenderer(Mesh* meshRef, Material* materialRef)
        : mesh(meshRef), material(materialRef)
    {
    }

    MeshRenderer::~MeshRenderer()
    {
        if (transformBuffer)
        {
            transformBuffer->Release();
            transformBuffer = nullptr;
        }
    }

    void MeshRenderer::Start()
    {
        GraphicsContext& graphics = Engine::Application::Instance()->Graphics();
        D3D11_BUFFER_DESC desc = {};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(TransformBuffer);
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        graphics.Device()->CreateBuffer(&desc, nullptr, &transformBuffer);
    }

    void MeshRenderer::Render(GraphicsContext& graphics)
    {
        if (!mesh || !material || !transformBuffer)
        {
            return;
        }

        const Transform& transform = Owner().GetTransform();
        const XMMATRIX world =
            XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f) *
            XMMatrixRotationZ(transform.rotation) *
            XMMatrixTranslation(transform.position.x, transform.position.y, 0.0f);

        TransformBuffer data = { XMMatrixTranspose(world) };
        graphics.Context()->UpdateSubresource(transformBuffer, 0, nullptr, &data, 0, 0);
        graphics.Context()->VSSetConstantBuffers(0, 1, &transformBuffer);

        material->Bind(graphics.Context());
        mesh->Bind(graphics.Context());
        mesh->Draw(graphics.Context());
    }
}

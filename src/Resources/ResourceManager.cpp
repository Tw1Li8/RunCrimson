#include "Resources/ResourceManager.h"
#include "Graphics/GraphicsContext.h"

namespace Engine
{
    bool ResourceManager::Initialize(GraphicsContext& graphicsContext)
    {
        graphics = &graphicsContext;
        basicShader = std::make_unique<Shader>();
        if (!basicShader->LoadFromFile(graphics->Device(), L"Shaders/BasicColor.hlsl"))
        {
            return false;
        }

        std::vector<Vertex> quad =
        {
            { -0.5f,  0.5f, 0.0f, 1, 1, 1, 1, 0, 0 },
            {  0.5f,  0.5f, 0.0f, 1, 1, 1, 1, 1, 0 },
            {  0.5f, -0.5f, 0.0f, 1, 1, 1, 1, 1, 1 },
            { -0.5f,  0.5f, 0.0f, 1, 1, 1, 1, 0, 0 },
            {  0.5f, -0.5f, 0.0f, 1, 1, 1, 1, 1, 1 },
            { -0.5f, -0.5f, 0.0f, 1, 1, 1, 1, 0, 1 },
        };

        auto quadMesh = std::make_unique<Mesh>();
        quadMesh->Create(graphics->Device(), quad);
        meshes["Quad"] = std::move(quadMesh);
        return true;
    }

    Mesh* ResourceManager::GetMesh(const std::string& name)
    {
        auto found = meshes.find(name);
        return found != meshes.end() ? found->second.get() : nullptr;
    }

    Material* ResourceManager::CreateMaterial(const std::string& name, const Color& color)
    {
        auto material = std::make_unique<Material>();
        material->Create(graphics->Device(), basicShader.get(), color);
        Material* ref = material.get();
        materials[name] = std::move(material);
        return ref;
    }

    Material* ResourceManager::CreateTexturedMaterial(const std::string& name, const Color& tint, const unsigned int* pixels, int width, int height)
    {
        auto material = std::make_unique<Material>();
        material->Create(graphics->Device(), basicShader.get(), tint);
        material->CreateTexture(graphics->Device(), pixels, width, height);
        Material* ref = material.get();
        materials[name] = std::move(material);
        return ref;
    }

    Material* ResourceManager::LoadMaterialFromFile(const std::string& name, const Color& tint, const std::wstring& filePath)
    {
        auto material = std::make_unique<Material>();
        material->Create(graphics->Device(), basicShader.get(), tint);
        if (!material->LoadTextureFromFile(graphics->Device(), filePath))
        {
            return nullptr;
        }
        Material* ref = material.get();
        materials[name] = std::move(material);
        return ref;
    }

    Material* ResourceManager::GetMaterial(const std::string& name)
    {
        auto found = materials.find(name);
        return found != materials.end() ? found->second.get() : nullptr;
    }

    void ResourceManager::Shutdown()
    {
        for (auto& item : materials)
        {
            item.second->Release();
        }
        materials.clear();

        for (auto& item : meshes)
        {
            item.second->Release();
        }
        meshes.clear();

        if (basicShader)
        {
            basicShader->Release();
            basicShader.reset();
        }
    }
}
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Shader.h"

namespace Engine
{
    class GraphicsContext;

    class ResourceManager
    {
    public:
        bool Initialize(GraphicsContext& graphics);
        void Shutdown();

        Mesh* GetMesh(const std::string& name);
        Material* CreateMaterial(const std::string& name, const Color& color);
        Material* CreateTexturedMaterial(const std::string& name, const Color& tint, const unsigned int* pixels, int width, int height);
        Material* LoadMaterialFromFile(const std::string& name, const Color& tint, const std::wstring& filePath);
        Material* GetMaterial(const std::string& name);

    private:
        GraphicsContext* graphics = nullptr;
        std::unique_ptr<Shader> basicShader;
        std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
        std::unordered_map<std::string, std::unique_ptr<Material>> materials;
    };
}
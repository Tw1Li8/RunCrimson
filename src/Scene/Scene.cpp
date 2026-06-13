#include "Scene/Scene.h"
#include "Object/GameObject.h"

namespace Engine
{
    Scene::~Scene() = default;

    GameObject& Scene::CreateObject(const char* name)
    {
        auto object = std::make_unique<GameObject>(name);
        GameObject& ref = *object;
        objects.push_back(std::move(object));
        return ref;
    }

    void Scene::Update(float dt)
    {
        for (auto& object : objects)
        {
            object->Update(dt);
        }
        physics.Step(dt);
    }

    void Scene::Render(GraphicsContext& graphics)
    {
        for (auto& object : objects)
        {
            object->Render(graphics);
        }
    }
}

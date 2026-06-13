#include "Object/GameObject.h"
#include "Graphics/GraphicsContext.h"

namespace Engine
{
    GameObject::GameObject(std::string objectName)
        : name(std::move(objectName))
    {
    }

    void GameObject::Update(float dt)
    {
        for (auto& component : components)
        {
            if (!component->Started())
            {
                component->Start();
                component->MarkStarted();
            }
            component->Update(dt);
        }
    }

    void GameObject::Render(GraphicsContext& graphics)
    {
        for (auto& component : components)
        {
            component->Render(graphics);
        }
    }
}

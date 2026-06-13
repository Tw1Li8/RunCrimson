#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include "Object/Component.h"
#include "Object/Transform.h"

namespace Engine
{
    class GameObject
    {
    public:
        explicit GameObject(std::string objectName);

        const std::string& Name() const { return name; }
        Transform& GetTransform() { return transform; }
        const Transform& GetTransform() const { return transform; }

        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component.");
            std::unique_ptr<T> component = std::make_unique<T>(std::forward<Args>(args)...);
            component->owner = this;
            T& ref = *component;
            components.push_back(std::move(component));
            return ref;
        }

        template <typename T>
        T* GetComponent()
        {
            for (auto& component : components)
            {
                if (T* casted = dynamic_cast<T*>(component.get()))
                {
                    return casted;
                }
            }
            return nullptr;
        }

        void Update(float dt);
        void Render(GraphicsContext& graphics);

    private:
        std::string name;
        Transform transform;
        std::vector<std::unique_ptr<Component>> components;
    };
}

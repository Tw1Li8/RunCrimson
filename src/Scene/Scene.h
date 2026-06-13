#pragma once

#include <memory>
#include <vector>
#include "Object/GameObject.h"
#include "Physics/PhysicsWorld2D.h"

namespace Engine
{
    class GraphicsContext;

    class Scene
    {
    public:
        virtual ~Scene();
        virtual void Initialize(GraphicsContext& graphics) {}
        virtual void Update(float dt);
        virtual void Render(GraphicsContext& graphics);

        GameObject& CreateObject(const char* name);
        PhysicsWorld2D& Physics() { return physics; }

    protected:
        std::vector<std::unique_ptr<GameObject>> objects;
        PhysicsWorld2D physics;
    };
}

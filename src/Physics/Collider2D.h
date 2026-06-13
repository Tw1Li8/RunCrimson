#pragma once

#include "Core/EngineTypes.h"
#include "Object/Component.h"

namespace Engine
{
    class Collider2D : public Component
    {
    public:
        explicit Collider2D(Vec2 halfSize, bool staticCollider = false)
            : halfExtents(halfSize), isStatic(staticCollider)
        {
        }

        Vec2 halfExtents;
        bool isStatic = false;
    };
}

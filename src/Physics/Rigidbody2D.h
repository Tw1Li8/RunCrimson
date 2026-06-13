#pragma once

#include "Core/EngineTypes.h"
#include "Object/Component.h"

namespace Engine
{
    class Rigidbody2D : public Component
    {
    public:
        Vec2 velocity = { 0.0f, 0.0f };
        float gravityScale = 1.0f;
        bool useGravity = true;
        bool isStatic = false;
        bool isGrounded = false;

        void AddForce(const Vec2& force)
        {
            velocity += force;
        }
    };
}

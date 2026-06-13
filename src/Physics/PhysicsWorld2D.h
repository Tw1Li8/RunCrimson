#pragma once

#include <vector>
#include "Core/EngineTypes.h"

namespace Engine
{
    class Collider2D;
    class Rigidbody2D;

    class PhysicsWorld2D
    {
    public:
        void Register(Rigidbody2D* body);
        void Register(Collider2D* collider);
        void Step(float dt);

    private:
        bool Intersects(Collider2D& a, Collider2D& b, Vec2& penetration);
        std::vector<Rigidbody2D*> bodies;
        std::vector<Collider2D*> colliders;
        Vec2 gravity = { 0.0f, -3.6f };
    };
}

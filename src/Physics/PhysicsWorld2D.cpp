#include "Physics/PhysicsWorld2D.h"
#include <cmath>
#include "Object/GameObject.h"
#include "Physics/Collider2D.h"
#include "Physics/Rigidbody2D.h"

namespace Engine
{
    void PhysicsWorld2D::Register(Rigidbody2D* body)
    {
        bodies.push_back(body);
    }

    void PhysicsWorld2D::Register(Collider2D* collider)
    {
        colliders.push_back(collider);
    }

    void PhysicsWorld2D::Step(float dt)
    {
        for (Rigidbody2D* body : bodies)
        {
            if (!body || body->isStatic)
            {
                continue;
            }

            body->isGrounded = false;
            if (body->useGravity)
            {
                body->velocity += gravity * body->gravityScale * dt;
            }

            Transform& transform = body->Owner().GetTransform();
            transform.position += body->velocity * dt;
        }

        for (Collider2D* dynamicCollider : colliders)
        {
            if (!dynamicCollider || dynamicCollider->isStatic)
            {
                continue;
            }

            Rigidbody2D* body = dynamicCollider->Owner().GetComponent<Rigidbody2D>();
            if (!body || body->isStatic)
            {
                continue;
            }

            for (Collider2D* staticCollider : colliders)
            {
                if (!staticCollider || !staticCollider->isStatic)
                {
                    continue;
                }

                Vec2 penetration;
                if (!Intersects(*dynamicCollider, *staticCollider, penetration))
                {
                    continue;
                }

                Transform& transform = dynamicCollider->Owner().GetTransform();
                if (penetration.y < penetration.x)
                {
                    const float direction = transform.position.y > staticCollider->Owner().GetTransform().position.y ? 1.0f : -1.0f;
                    transform.position.y += penetration.y * direction;
                    if (direction > 0.0f)
                    {
                        body->isGrounded = true;
                    }
                    body->velocity.y = 0.0f;
                }
                else
                {
                    const float direction = transform.position.x > staticCollider->Owner().GetTransform().position.x ? 1.0f : -1.0f;
                    transform.position.x += penetration.x * direction;
                    body->velocity.x = 0.0f;
                }
            }
        }
    }

    bool PhysicsWorld2D::Intersects(Collider2D& a, Collider2D& b, Vec2& penetration)
    {
        const Vec2 aPos = a.Owner().GetTransform().position;
        const Vec2 bPos = b.Owner().GetTransform().position;

        const float overlapX = (a.halfExtents.x + b.halfExtents.x) - std::fabs(aPos.x - bPos.x);
        const float overlapY = (a.halfExtents.y + b.halfExtents.y) - std::fabs(aPos.y - bPos.y);

        if (overlapX <= 0.0f || overlapY <= 0.0f)
        {
            return false;
        }

        penetration = { overlapX, overlapY };
        return true;
    }
}

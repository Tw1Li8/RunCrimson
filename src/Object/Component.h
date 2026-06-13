#pragma once

namespace Engine
{
    class GameObject;
    class GraphicsContext;

    class Component
    {
    public:
        virtual ~Component() = default;
        virtual void Start() {}
        virtual void Update(float dt) {}
        virtual void Render(GraphicsContext& graphics) {}

        GameObject& Owner() const { return *owner; }
        bool Started() const { return started; }
        void MarkStarted() { started = true; }

    private:
        friend class GameObject;
        GameObject* owner = nullptr;
        bool started = false;
    };
}

#pragma once

namespace Engine
{
    struct Vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        Vec2() = default;
        Vec2(float xValue, float yValue) : x(xValue), y(yValue) {}

        Vec2 operator+(const Vec2& other) const { return { x + other.x, y + other.y }; }
        Vec2 operator-(const Vec2& other) const { return { x - other.x, y - other.y }; }
        Vec2 operator*(float value) const { return { x * value, y * value }; }
        Vec2& operator+=(const Vec2& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }
    };

    struct Color
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
    };

    inline float Clamp(float value, float minValue, float maxValue)
    {
        if (value < minValue)
        {
            return minValue;
        }
        if (value > maxValue)
        {
            return maxValue;
        }
        return value;
    }
}

#pragma once

#include <cmath>

struct Vec2
{
    // 2D vector components
    float x = 0.0f;
    float y = 0.0f;

    // Constructors
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    // Operators
    Vec2 operator+(const Vec2 &rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2 &rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator*(float scalar) const
    {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/(float scalar) const
    {
        return Vec2(x / scalar, y / scalar);
    }

    void operator+=(const Vec2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }

    void operator-=(const Vec2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }

    void operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    void operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    // Methods
    void set(float newX, float newY)
    {
        x = newX;
        y = newY;
    }

    float length() const
    {
        return std::sqrt(x * x + y * y);
    }

    float angle() const
    {
        return std::atan2(y, x);
    }
};
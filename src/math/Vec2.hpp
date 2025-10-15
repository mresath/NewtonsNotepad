#pragma once

#include <cmath>
#include <string>

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

    float lengthSquared() const
    {
        return x * x + y * y;
    }

    float length() const
    {
        return std::sqrt(lengthSquared());
    }

    float angle() const
    {
        return std::atan2(y, x);
    }

    Vec2 normalized() const
    {
        float len = length();
        if (len > 0.0f)
            return Vec2(x / len, y / len);
        return Vec2(0, 0);
    }

    // Conversions
    std::string toString() const
    {
        return "Vec2(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

inline float dot(const Vec2 &a, const Vec2 &b)
{
    return a.x * b.x + a.y * b.y;
}
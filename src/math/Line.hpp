#pragma once

#include <cmath>
#include "Vec2.hpp"

struct Line
{
    Vec2 origin = Vec2(0, 0);    // A point on the line
    Vec2 direction = Vec2(1, 1); // Direction vector of the line (should be normalized)

    Line() = default;
    Line(Vec2 origin, float dx, float dy) : origin(origin), direction(dx, dy) {}

    float scalarProjection(const Vec2 &point) const
    {
        return dot(direction, point - origin);
    }
};
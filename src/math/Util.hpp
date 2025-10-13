#pragma once

#include "math/Vec2.hpp"

// Conversion functions

/// Length
float pixelsPerMeter = 25.0f; // Conversion factor between pixels and meters
float pixelsToMeters(float pixels) { return pixels / pixelsPerMeter; }
float metersToPixels(float meters) { return meters * pixelsPerMeter; }
Vec2 pixelsToMeters(const Vec2 &pixels) { return Vec2(pixelsToMeters(pixels.x), pixelsToMeters(pixels.y)); }
Vec2 metersToPixels(const Vec2 &meters) { return Vec2(metersToPixels(meters.x), metersToPixels(meters.y)); }
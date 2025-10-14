#pragma once

// Conversion functions

/// Length
inline float pixelsPerMeter = 50.f; // Conversion factor between pixels and meters
inline float pixelsToMeters(float pixels) { return pixels / pixelsPerMeter; }
inline float metersToPixels(float meters) { return meters * pixelsPerMeter; }
inline Vec2 * pixelsToMeters(const Vec2 * pixels) { return new Vec2(pixelsToMeters(pixels->x), pixelsToMeters(pixels->y)); }
inline Vec2 * metersToPixels(const Vec2 * meters) { return new Vec2(metersToPixels(meters->x), metersToPixels(meters->y)); }
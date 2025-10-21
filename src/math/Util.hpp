#pragma once

#define PIXELS_PER_METER 50.f

// Conversion functions

/// Length
inline float pixelsPerMeter = PIXELS_PER_METER; // Conversion factor between pixels and meters
inline float pixelsToMeters(float pixels) { return pixels / pixelsPerMeter; }
inline float metersToPixels(float meters) { return meters * pixelsPerMeter; }
inline Vec2 * pixelsToMeters(const Vec2 * pixels) { return new Vec2(pixelsToMeters(pixels->x), pixelsToMeters(pixels->y)); }
inline Vec2 * metersToPixels(const Vec2 * meters) { return new Vec2(metersToPixels(meters->x), metersToPixels(meters->y)); }
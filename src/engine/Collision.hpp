#pragma once

#include <iostream>
#include <fmt/format.h>
#include "math/Vec2.hpp"
#include "math/Line.hpp"
#include "objects/Object.hpp"

// TODO: Update collision detection to support rotated rectangles

// Helper structures for collision information
struct CollisionInfo
{
    bool isColliding;
    Vec2 normal;       // Direction to push objects apart
    float penetration; // How deep the overlap is
    Vec2 contactPoint; // Point of contact
};

// Circle-Circle collision detection
CollisionInfo checkCircleCircleCollision(Object *objA, Object *objB)
{
    CollisionInfo info;
    info.isColliding = false;

    Vec2 diff = objA->body->position - objB->body->position;
    float distance = diff.length();
    float radiusSum = objA->dimensions.x + objB->dimensions.x;

    if (distance < radiusSum)
    {
        info.isColliding = true;
        info.penetration = radiusSum - distance;
        info.normal = (distance > 0.0f) ? diff.normalized() : Vec2(1.0f, 0.0f);
        info.contactPoint = objA->body->position - info.normal * (objA->dimensions.x - info.penetration * 0.5f);
    }

    return info;
}

// Helper functions for rectangles

Line* getAxes(const Object *rect) {
    Vec2 RX = Vec2(1, 0).rotated(rect->body->rotation);
    Vec2 RY = Vec2(0, 1).rotated(rect->body->rotation);

    Line axisX(rect->body->position, RX.x, RX.y);
    Line axisY(rect->body->position, RY.x, RY.y);

    return new Line[2]{axisX, axisY};
}

Vec2* getCorners(const Object *rect) {
    Line* axes = getAxes(rect);
    Vec2 RX = axes[0].direction * (rect->dimensions.x * 0.5f);
    Vec2 RY = axes[1].direction * (rect->dimensions.y * 0.5f);
    delete[] axes;

    return new Vec2[4]{
        rect->body->position + RX + RY, // Top Right
        rect->body->position + RX - RY, // Bottom Right
        rect->body->position - RX - RY, // Bottom Left
        rect->body->position - RX + RY, // Top Left
    };
}

// Helper function to project corners onto an axis using Line.projectPoint
void projectCornersOntoAxis(Vec2* corners, const Line& axis, float& min, float& max)
{
    // Use Line's projectPoint to get scalar projections
    min = max = axis.scalarProjection(corners[0]);
    for (int i = 1; i < 4; i++)
    {
        float projection = axis.scalarProjection(corners[i]);
        if (projection < min) min = projection;
        if (projection > max) max = projection;
    }
}

/*
// Rectangle-Rectangle collision detection using SAT
CollisionInfo checkRectRectCollision(Object *objA, Object *objB)
{
    CollisionInfo info;
    info.isColliding = true;
    info.penetration = INFINITY;

    // Get corners for both rectangles
    Vec2* cornersA = getCorners(objA);
    Vec2* cornersB = getCorners(objB);

    // Get axes to test (normals of edges)
    Line* axesA = getAxes(objA);
    Line* axesB = getAxes(objB);

    // Test all 4 axes (2 from each rectangle)
    Line allAxes[4] = {axesA[0], axesA[1], axesB[0], axesB[1]};
    
    for (int i = 0; i < 4; i++)
    {
        const Line& axis = allAxes[i];
        
        // Project both rectangles onto the axis using Line's projection method
        float minA, maxA, minB, maxB;
        projectCornersOntoAxis(cornersA, axis, minA, maxA);
        projectCornersOntoAxis(cornersB, axis, minB, maxB);
        
        // Check for overlap
        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        
        if (overlap <= 0)
        {
            // Found separating axis - no collision
            info.isColliding = false;
            delete[] cornersA;
            delete[] cornersB;
            delete[] axesA;
            delete[] axesB;
            return info;
        }
        
        // Track minimum overlap for penetration depth
        if (overlap < info.penetration)
        {
            info.penetration = overlap;
            info.normal = axis.direction;
            
            // Ensure normal points from B to A
            Vec2 diff = objA->body->position - objB->body->position;
            if (dot(diff, info.normal) < 0)
            {
                info.normal = info.normal * -1.0f;
            }
        }
    }

    // Calculate contact point - find the center of the overlapping region
    
    // Get perpendicular to the collision normal
    Vec2 tangent = info.normal.perpendicular();
    
    // Project all corners onto both collision normal and tangent
    float projNormalA[4], projNormalB[4];
    float projTangentA[4], projTangentB[4];
    
    for (int i = 0; i < 4; i++)
    {
        projNormalA[i] = dot(cornersA[i], info.normal);
        projNormalB[i] = dot(cornersB[i], info.normal);
        projTangentA[i] = dot(cornersA[i], tangent);
        projTangentB[i] = dot(cornersB[i], tangent);
    }

    // Find overlap range along normal
    float minNormalA = *std::min_element(projNormalA, projNormalA + 4);
    float maxNormalA = *std::max_element(projNormalA, projNormalA + 4);
    float minNormalB = *std::min_element(projNormalB, projNormalB + 4);
    float maxNormalB = *std::max_element(projNormalB, projNormalB + 4);
    
    // Find overlap range along tangent
    float minTangentA = *std::min_element(projTangentA, projTangentA + 4);
    float maxTangentA = *std::max_element(projTangentA, projTangentA + 4);
    float minTangentB = *std::min_element(projTangentB, projTangentB + 4);
    float maxTangentB = *std::max_element(projTangentB, projTangentB + 4);

    // Contact point is at the center of the overlapping region
    float contactNormal = (std::max(minNormalA, minNormalB) + std::min(maxNormalA, maxNormalB)) * 0.5f;
    float contactTangent = (std::max(minTangentA, minTangentB) + std::min(maxTangentA, maxTangentB)) * 0.5f;
    
    info.contactPoint = info.normal * contactNormal + tangent * contactTangent;
    
    delete[] cornersA;
    delete[] cornersB;
    delete[] axesA;
    delete[] axesB;
    
    return info;
}
*/

// Circle-Rectangle collision detection (supports rotated rectangles)
CollisionInfo checkCircleRectCollision(Object *circle, Object *rect)
{
    CollisionInfo info;
    info.isColliding = false;

    // Get rectangle's local axes
    Line* axes = getAxes(rect);
    Vec2 halfSize = rect->dimensions * 0.5f;
    
    // Transform circle center to rectangle's local space using projections
    Vec2 diff = circle->body->position - rect->body->position;
    float localX = axes[0].scalarProjection(circle->body->position);
    float localY = axes[1].scalarProjection(circle->body->position);
    
    // Clamp to rectangle extents in local space
    float clampedX = std::max(-halfSize.x, std::min(localX, halfSize.x));
    float clampedY = std::max(-halfSize.y, std::min(localY, halfSize.y));
    
    // Transform closest point back to world space
    Vec2 closestPoint = rect->body->position + 
                        axes[0].direction * clampedX + 
                        axes[1].direction * clampedY;
    
    // Calculate vector from closest point to circle center
    Vec2 delta = circle->body->position - closestPoint;
    float distSquared = delta.lengthSquared();
    float radius = circle->dimensions.x;

    if (distSquared < radius * radius)
    {
        info.isColliding = true;
        float dist = std::sqrt(distSquared);

        if (dist > 0.0f)
        {
            info.penetration = radius - dist;
            info.normal = delta.normalized();
            info.contactPoint = closestPoint;
        }
        else
        {
            // Circle center is inside rectangle - push out on closest axis in local space
            float distToEdgeX = halfSize.x - std::abs(localX);
            float distToEdgeY = halfSize.y - std::abs(localY);
            
            if (distToEdgeX < distToEdgeY)
            {
                info.penetration = radius + distToEdgeX;
                info.normal = axes[0].direction * (localX > 0 ? 1.0f : -1.0f);
            }
            else
            {
                info.penetration = radius + distToEdgeY;
                info.normal = axes[1].direction * (localY > 0 ? 1.0f : -1.0f);
            }
            info.contactPoint = closestPoint;
        }
    }

    delete[] axes;
    return info;
}

// General Collision detection
CollisionInfo checkCollision(Object *objA, Object *objB)
{
    if (objA->isGrabbed || objB->isGrabbed)
    {
        return CollisionInfo{false, Vec2(0, 0), 0.0f};
    }

    if (objA->shapeType == CIRCLE && objB->shapeType == CIRCLE)
    {
        return checkCircleCircleCollision(objA, objB);
    }
    else if (objA->shapeType == RECTANGLE && objB->shapeType == RECTANGLE)
    {
        // return checkRectRectCollision(objA, objB);
    }
    else if (objA->shapeType == CIRCLE && objB->shapeType == RECTANGLE)
    {
        return checkCircleRectCollision(objA, objB);
    }
    else if (objA->shapeType == RECTANGLE && objB->shapeType == CIRCLE)
    {
        CollisionInfo info = checkCircleRectCollision(objB, objA);
        // Invert normal for correct direction
        info.normal *= -1.0f;
        return info;
    }
    return CollisionInfo{false, Vec2(0, 0), 0.0f};
}

void resolveCollision(Object *objA, Object *objB, const CollisionInfo &info, float dt)
{
    // Don't resolve collision if both objects are static
    if (objA->isStatic && objB->isStatic)
    {
        return;
    }

    Body *bodyA = objA->body;
    Body *bodyB = objB->body;

    Vec2 rA = info.contactPoint - bodyA->position;
    Vec2 rB = info.contactPoint - bodyB->position;

    // Calculate inverse masses (0 for static objects)
    float invMassA = objA->isStatic ? 0.0f : bodyA->invMass;
    float invMassB = objB->isStatic ? 0.0f : bodyB->invMass;

    // Calculate inverse moments of inertia (0 for static objects)
    float invInertiaA = objA->isStatic ? 0.0f : bodyA->invMomentOfInertia;
    float invInertiaB = objB->isStatic ? 0.0f : bodyB->invMomentOfInertia;

    // Calculate relative velocity at contact point
    Vec2 velA = bodyA->velocity + Vec2(-rA.y * bodyA->angularVelocity, 
                                        rA.x * bodyA->angularVelocity);
    Vec2 velB = bodyB->velocity + Vec2(-rB.y * bodyB->angularVelocity, 
                                        rB.x * bodyB->angularVelocity);
    Vec2 relativeVelocity = velA - velB;

    float vNormalMag = dot(relativeVelocity, info.normal);
    if (vNormalMag > 0)
        return; // Objects are separating
    if (std::abs(vNormalMag) < 0.01f)
        return; // Negligible collision

    // Positional correction to avoid sinking
    const float percent = 0.8f; // Penetration percentage to correct
    const float slop = 0.01f;   // Penetration allowance
    float invMassSum = invMassA + invMassB;
    Vec2 correction = info.normal * (std::max(info.penetration - slop, 0.0f) / invMassSum) * percent;
    if (!objA->isStatic)
        bodyA->position += correction * invMassA;
    if (!objB->isStatic)
        bodyB->position -= correction * invMassB;

    // Cross products for angular component
    float rACrossN = cross(rA, info.normal);
    float rBCrossN = cross(rB, info.normal);

    // Effective mass including rotation
    float effectiveMass = invMassA + invMassB + 
                          rACrossN * rACrossN * invInertiaA + 
                          rBCrossN * rBCrossN * invInertiaB;

    float restitution = std::min(bodyA->restitution, bodyB->restitution);

    // Calculate impulse scalar
    float impulseMagnitude = -(1.0f + restitution) * vNormalMag / effectiveMass;
    Vec2 impulse = info.normal * impulseMagnitude;

    // Apply impulse to linear and angular velocities
    bodyA->velocity += impulse * invMassA;
    bodyB->velocity -= impulse * invMassB;

    bodyA->angularVelocity += rACrossN * impulseMagnitude * invInertiaA;
    bodyB->angularVelocity -= rBCrossN * impulseMagnitude * invInertiaB;

    // --- Apply Friction ---
    // Recalculate relative velocity after normal impulse
    velA = bodyA->velocity + Vec2(-rA.y * bodyA->angularVelocity, 
                                   rA.x * bodyA->angularVelocity);
    velB = bodyB->velocity + Vec2(-rB.y * bodyB->angularVelocity, 
                                   rB.x * bodyB->angularVelocity);
    relativeVelocity = velA - velB;

    // Calculate tangent vector (perpendicular to normal)
    Vec2 tangent = relativeVelocity - info.normal * dot(relativeVelocity, info.normal);
    float tangentMag = tangent.length();
    
    if (tangentMag > 0.001f) // Only apply friction if there's tangential velocity
    {
        tangent = tangent / tangentMag; // Normalize
        
        // Cross products for tangential direction
        float rACrossT = cross(rA, tangent);
        float rBCrossT = cross(rB, tangent);
        
        // Effective mass for tangential direction
        float effectiveMassTangent = invMassA + invMassB + 
                                      rACrossT * rACrossT * invInertiaA + 
                                      rBCrossT * rBCrossT * invInertiaB;
        
        // Calculate friction impulse
        float friction = std::max(bodyA->frictionCoefficient, bodyB->frictionCoefficient);
        float frictionImpulseMag = -dot(relativeVelocity, tangent) / effectiveMassTangent;
        
        // Coulomb friction: clamp to friction cone
        // F_friction <= μ * F_normal
        frictionImpulseMag = std::max(-std::abs(impulseMagnitude) * friction, 
                                      std::min(frictionImpulseMag, 
                                               std::abs(impulseMagnitude) * friction));
        
        Vec2 frictionImpulse = tangent * frictionImpulseMag;
        
        // Apply friction impulse to linear and angular velocities
        bodyA->velocity += frictionImpulse * invMassA;
        bodyB->velocity -= frictionImpulse * invMassB;
        
        bodyA->angularVelocity += rACrossT * frictionImpulseMag * invInertiaA;
        bodyB->angularVelocity -= rBCrossT * frictionImpulseMag * invInertiaB;
    }
}

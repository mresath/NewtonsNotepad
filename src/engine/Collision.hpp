#pragma once

#include <iostream>
#include "math/Vec2.hpp"
#include "objects/Object.hpp"

// Helper structures for collision information
struct CollisionInfo
{
    bool isColliding;
    Vec2 normal;       // Direction to push objects apart
    float penetration; // How deep the overlap is
};

// Circle-Circle collision detection
CollisionInfo checkCircleCircleCollision(Object *objA, Object *objB)
{
    CollisionInfo info;
    info.isColliding = false;

    Vec2 diff = objB->body->position - objA->body->position;
    float distance = diff.length();
    float radiusSum = objA->dimensions.x + objB->dimensions.x;

    if (distance < radiusSum)
    {
        info.isColliding = true;
        info.penetration = radiusSum - distance;
        info.normal = (distance > 0.0f) ? diff.normalized() : Vec2(1.0f, 0.0f);
    }

    return info;
}

// Rectangle-Rectangle collision detection
CollisionInfo checkRectRectCollision(Object *objA, Object *objB)
{
    CollisionInfo info;
    info.isColliding = false;

    Vec2 halfSizeA = objA->dimensions * 0.5f;
    Vec2 halfSizeB = objB->dimensions * 0.5f;

    Vec2 diff = objB->body->position - objA->body->position;

    // Calculate overlap on each axis
    float overlapX = (halfSizeA.x + halfSizeB.x) - std::abs(diff.x);
    float overlapY = (halfSizeA.y + halfSizeB.y) - std::abs(diff.y);

    if (overlapX > 0 && overlapY > 0)
    {
        info.isColliding = true;

        // Choose the axis with smallest overlap (minimum penetration)
        if (overlapX < overlapY)
        {
            info.penetration = overlapX;
            info.normal = Vec2(diff.x > 0 ? 1.0f : -1.0f, 0.0f);
        }
        else
        {
            info.penetration = overlapY;
            info.normal = Vec2(0.0f, diff.y > 0 ? 1.0f : -1.0f);
        }
    }

    return info;
}

// Circle-Rectangle collision detection
CollisionInfo checkCircleRectCollision(Object *circle, Object *rect)
{
    CollisionInfo info;
    info.isColliding = false;

    Vec2 halfSize = rect->dimensions * 0.5f;
    Vec2 diff = circle->body->position - rect->body->position;

    // Find closest point on rectangle to circle center
    Vec2 closest;
    closest.x = std::max(-halfSize.x, std::min(diff.x, halfSize.x));
    closest.y = std::max(-halfSize.y, std::min(diff.y, halfSize.y));

    // Calculate vector from closest point to circle center
    Vec2 delta = diff - closest;
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
        }
        else
        {
            // Circle center is inside rectangle - push out on closest axis
            Vec2 absClosest = Vec2(std::abs(closest.x), std::abs(closest.y));
            if (halfSize.x - absClosest.x < halfSize.y - absClosest.y)
            {
                info.penetration = radius + (halfSize.x - absClosest.x);
                info.normal = Vec2(diff.x > 0 ? 1.0f : -1.0f, 0.0f);
            }
            else
            {
                info.penetration = radius + (halfSize.y - absClosest.y);
                info.normal = Vec2(0.0f, diff.y > 0 ? 1.0f : -1.0f);
            }
        }
    }

    return info;
}

// General Collision detection
CollisionInfo checkCollision(Object *objA, Object *objB)
{
    if (objA->shapeType == CIRCLE && objB->shapeType == CIRCLE)
    {
        return checkCircleCircleCollision(objA, objB);
    }
    else if (objA->shapeType == RECTANGLE && objB->shapeType == RECTANGLE)
    {
        return checkRectRectCollision(objA, objB);
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

// Calculate and apply normal and friction forces based on collision info
void resolveCollision(Object *objA, Object *objB, const CollisionInfo &info, float dt)
{
    if (objA->isStatic && objB->isStatic) {
        return;
    }

    Body* bodyA = objA->body;
    Body* bodyB = objB->body;

    float invMassA = objA->isStatic ? 0.0f : bodyA->invMass;
    float invMassB = objB->isStatic ? 0.0f : bodyB->invMass;
    float invMassSum = invMassA + invMassB;

    // Calculate and apply impulse
    Vec2 relativeVelocity = bodyB->velocity - bodyA->velocity;
    float restitution = std::min(bodyA->restitution, bodyB->restitution);
    
    float vNormalMag = dot(relativeVelocity, info.normal);
    // if (std::abs(vNormalMag) > 0) return; // Objects are separating
    // if (std::abs(vNormalMag) < 0.1f) restitution = 0.0f; // Prevent tiny bounces
    Vec2 vNormal = info.normal * vNormalMag * -(1 + restitution);

    Vec2 impulse = vNormal / invMassSum;

    bodyA->velocity -= impulse * invMassA;
    bodyB->velocity += impulse * invMassB;

    // Calculate and apply normal force
    Vec2 fNormal = info.normal * dot(bodyA->netForce - bodyB->netForce, info.normal) * -1;
    bodyA->applyForce(fNormal);
    bodyB->applyForce(fNormal * -1);
}

#pragma once

#include "math/Vec2.hpp"

struct Body
{
    // Motion Vectors
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    Vec2 netForce;

    // Physical Properties
    float mass;
    float invMass; // Inverse of mass to avoid unnecessary divisions

    // Other Properties
    float dragCoefficient = 0.47f;
    float staticFriction = 0.5f;
    float kineticFriction = 0.3f;
    float restitution = 0.7f;

    // Constructors
    Body(const Vec2 &position, float mass) : position(position), mass(mass)
    {
        this->invMass = (mass == 0.0f) ? 0.0f : 1.0f / mass;
    }

    // Methods
    void applyForce(const Vec2 &force) {
        netForce += force;
    };
};
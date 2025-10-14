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
    float area = 1.0f;

    // Constructors
    Body(const Vec2 &position, float mass);

    // Methods
    void applyForce(const Vec2 &force); // Apply a force to the body
    void update(float dt);              // Update the body's state based on forces and time step
};
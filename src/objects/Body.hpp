#pragma once

#include "math/Vec2.hpp"

struct Body
{
    // Motion Vectors
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    Vec2 netForce;

    // Rotational Motion - Counterclockwise positive
    float rotation = 0.0f;       // In radians
    float angularVelocity = 0.0f; // In radians per second
    float angularAcceleration = 0.0f;
    float netTorque = 0.0f;

    // Energies and Momenta
    Vec2 momentum = Vec2(0.0f, 0.0f);
    float angularMomentum = 0.0f;

    float kineticEnergy = 0.0f;
    float rotationalKineticEnergy = 0.0f;
    float gravitationalPotential = 0.0f;
    float totalEnergy = 0.0f;

    // Physical Properties
    float mass;
    float invMass; // Inverse of mass to avoid unnecessary divisions
    float momentOfInertia = 0.0f;
    float invMomentOfInertia = 0.0f;

    // Other Properties
    float dragCoefficient = 0.25f;
    float liftCoefficient = 0.00f;
    float frictionCoefficient = 0.5f;
    float restitution = 0.7f;

    // Constructors
    Body(const Vec2 &position, float mass, float momentOfInertia) : position(position), mass(mass), momentOfInertia(momentOfInertia)
    {
        this->invMass = (mass == 0.0f) ? 0.0f : 1.0f / mass;
        this->invMomentOfInertia = (momentOfInertia == 0.0f) ? 0.0f : 1.0f / momentOfInertia;
    }
};
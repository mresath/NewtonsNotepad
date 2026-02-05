#pragma once

#include "math/Vec2.hpp"
#include <nlohmann/json.hpp>

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
    float springPotential = 0.0f;
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

    nlohmann::json to_json() const
    {
        nlohmann::json j;

        nlohmann::json properties;
        properties["mass"] = mass;
        properties["momentOfInertia"] = momentOfInertia;

        nlohmann::json coefficients;
        coefficients["drag"] = dragCoefficient;
        coefficients["lift"] = liftCoefficient;
        coefficients["friction"] = frictionCoefficient;
        coefficients["restitution"] = restitution;

        nlohmann::json state;

        nlohmann::json linear;
        linear["position"] = { position.x, position.y };
        linear["velocity"] = { velocity.x, velocity.y };
        linear["momentum"] = { momentum.x, momentum.y };

        nlohmann::json rotational;
        rotational["rotation"] = rotation;
        rotational["angularVelocity"] = angularVelocity;
        rotational["angularMomentum"] = angularMomentum;

        nlohmann::json energies;
        energies["kinetic"] = kineticEnergy;
        energies["rotationalKinetic"] = rotationalKineticEnergy;
        energies["gravitationalPotential"] = gravitationalPotential;
        energies["springPotential"] = springPotential;
        energies["totalEnergy"] = totalEnergy;

        state["linear"] = linear;
        state["rotational"] = rotational;
        state["energies"] = energies;

        j["properties"] = properties;
        j["coefficients"] = coefficients;
        j["state"] = state;

        return j;
    }
};

enum BodyKeys {
    MASS,
    MOMENT_OF_INERTIA,
    DRAG_COEFFICIENT,
    LIFT_COEFFICIENT,
    FRICTION_COEFFICIENT,
    RESTITUTION,
    POSITION,
    VELOCITY,
    MOMENTUM,
    ROTATION,
    ANGULAR_VELOCITY,
    ANGULAR_MOMENTUM,
    KINETIC_ENERGY,
    ROTATIONAL_KINETIC_ENERGY,
    GRAVITATIONAL_POTENTIAL,
    SPRING_POTENTIAL,
    TOTAL_ENERGY
};

inline std::string BodyKeyValue(BodyKeys key) {
    switch (key) {
        case MASS: return "mass";
        case MOMENT_OF_INERTIA: return "momentOfInertia";
        case DRAG_COEFFICIENT: return "dragCoefficient";
        case LIFT_COEFFICIENT: return "liftCoefficient";
        case FRICTION_COEFFICIENT: return "frictionCoefficient";
        case RESTITUTION: return "restitution";
        case POSITION: return "position";
        case VELOCITY: return "velocity";
        case MOMENTUM: return "momentum";
        case ROTATION: return "rotation";
        case ANGULAR_VELOCITY: return "angularVelocity";
        case ANGULAR_MOMENTUM: return "angularMomentum";
        case KINETIC_ENERGY: return "kineticEnergy";
        case ROTATIONAL_KINETIC_ENERGY: return "rotationalKineticEnergy";
        case GRAVITATIONAL_POTENTIAL: return "gravitationalPotential";
        case SPRING_POTENTIAL: return "springPotential";
        case TOTAL_ENERGY: return "totalEnergy";
        default: return "unknown";
    }
}

inline std::string BodyKeyName(BodyKeys key) {
    switch (key) {
        case MASS: return "Mass";
        case MOMENT_OF_INERTIA: return "Moment of Inertia";
        case DRAG_COEFFICIENT: return "Drag Coefficient";
        case LIFT_COEFFICIENT: return "Lift Coefficient";
        case FRICTION_COEFFICIENT: return "Friction Coefficient";
        case RESTITUTION: return "Restitution";
        case POSITION: return "Position";
        case VELOCITY: return "Velocity";
        case MOMENTUM: return "Momentum";
        case ROTATION: return "Rotation";
        case ANGULAR_VELOCITY: return "Angular Velocity";
        case ANGULAR_MOMENTUM: return "Angular Momentum";
        case KINETIC_ENERGY: return "Kinetic Energy";
        case ROTATIONAL_KINETIC_ENERGY: return "Rotational Kinetic Energy";
        case GRAVITATIONAL_POTENTIAL: return "Gravitational Potential";
        case SPRING_POTENTIAL: return "Spring Potential";
        case TOTAL_ENERGY: return "Total Energy";
        default: return "Unknown";
    }
}
#pragma once

#include "math/Vec2.hpp"
#include "math/Util.hpp"
#include <nlohmann/json.hpp>

enum BodyKeys
{
    MASS,
    MOMENT_OF_INERTIA,
    DRAG_COEFFICIENT,
    FRICTION_COEFFICIENT,
    RESTITUTION,
    POSITION,
    POSITION_X,
    POSITION_Y,
    VELOCITY,
    VELOCITY_X,
    VELOCITY_Y,
    MOMENTUM,
    MOMENTUM_X,
    MOMENTUM_Y,
    ROTATION,
    ANGULAR_VELOCITY,
    ANGULAR_MOMENTUM,
    KINETIC_ENERGY,
    ROTATIONAL_KINETIC_ENERGY,
    GRAVITATIONAL_POTENTIAL,
    SPRING_POTENTIAL,
    TOTAL_ENERGY
};

struct Body
{
    // Motion Vectors
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    Vec2 netForce;

    // Rotational Motion - Counterclockwise positive
    float rotation = 0.0f;        // In radians
    float angularVelocity = 0.0f; // In radians per second
    float angularAcceleration = 0.0f;
    float netTorque = 0.0f;

    // Energies and Momenta
    Vec2 momentum;
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
    float frictionCoefficient = 0.5f;
    float restitution = 0.7f;

    // Constructors
    Body()
    {
        mass = 1.0f;
        invMass = 1.0f;
        momentOfInertia = 1.0f;
        invMomentOfInertia = 1.0f;
    }

    Body(const Vec2 &position, float mass, float momentOfInertia) : position(position), mass(mass), momentOfInertia(momentOfInertia)
    {
        this->invMass = (mass == 0.0f) ? 0.0f : 1.0f / mass;
        this->invMomentOfInertia = (momentOfInertia == 0.0f) ? 0.0f : 1.0f / momentOfInertia;
    }

    Body(const nlohmann::json &j)
    {
        *this = from_json(j);
    }

    static Body from_json(const nlohmann::json &j)
    {
        Body body;

        body.mass = j["properties"]["mass"].get<float>();
        body.momentOfInertia = j["properties"]["momentOfInertia"].get<float>();

        body.dragCoefficient = j["coefficients"]["drag"].get<float>();
        body.frictionCoefficient = j["coefficients"]["friction"].get<float>();
        body.restitution = j["coefficients"]["restitution"].get<float>();

        body.position.set(j["state"]["linear"]["position"][0].get<float>(),
                          j["state"]["linear"]["position"][1].get<float>());
        body.position = destandardizePosition(body.position);
        body.velocity.set(j["state"]["linear"]["velocity"][0].get<float>(),
                          j["state"]["linear"]["velocity"][1].get<float>());
        body.momentum.set(j["state"]["linear"]["momentum"][0].get<float>(),
                          j["state"]["linear"]["momentum"][1].get<float>());

        body.rotation = j["state"]["rotational"]["rotation"].get<float>();
        body.angularVelocity = j["state"]["rotational"]["angularVelocity"].get<float>();
        body.angularMomentum = j["state"]["rotational"]["angularMomentum"].get<float>();

        body.kineticEnergy = j["state"]["energies"]["kinetic"].get<float>();
        body.rotationalKineticEnergy = j["state"]["energies"]["rotationalKinetic"].get<float>();
        body.gravitationalPotential = j["state"]["energies"]["gravitationalPotential"].get<float>();
        body.springPotential = j["state"]["energies"]["springPotential"].get<float>();
        body.totalEnergy = j["state"]["energies"]["totalEnergy"].get<float>();

        return body;
    }

    nlohmann::json to_json() const
    {
        nlohmann::json j;

        nlohmann::json properties;
        properties["mass"] = mass;
        properties["momentOfInertia"] = momentOfInertia;

        nlohmann::json coefficients;
        coefficients["drag"] = dragCoefficient;
        coefficients["friction"] = frictionCoefficient;
        coefficients["restitution"] = restitution;

        nlohmann::json state;

        nlohmann::json linear;
        Vec2 sPos = standardizePosition(position);
        linear["position"] = {sPos.x, sPos.y};
        linear["velocity"] = {velocity.x, velocity.y};
        linear["momentum"] = {momentum.x, momentum.y};

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

    float getProperty(BodyKeys key) const
    {
        Vec2 sPos = standardizePosition(position);
        switch (key)
        {
        case MASS:
            return mass;
        case MOMENT_OF_INERTIA:
            return momentOfInertia;
        case DRAG_COEFFICIENT:
            return dragCoefficient;
        case FRICTION_COEFFICIENT:
            return frictionCoefficient;
        case RESTITUTION:
            return restitution;
        case POSITION:
            return sPos.length(); // Return magnitude of position vector
        case POSITION_X:
            return sPos.x;
        case POSITION_Y:
            return sPos.y;
        case VELOCITY:
            return velocity.length(); // Return speed
        case VELOCITY_X:
            return velocity.x;
        case VELOCITY_Y:
            return velocity.y;
        case MOMENTUM:
            return momentum.length(); // Return magnitude of momentum vector
        case MOMENTUM_X:
            return momentum.x;
        case MOMENTUM_Y:    
            return momentum.y;
        case ROTATION:
            return rotation;
        case ANGULAR_VELOCITY:
            return angularVelocity;
        case ANGULAR_MOMENTUM:
            return angularMomentum;
        case KINETIC_ENERGY:
            return kineticEnergy;
        case ROTATIONAL_KINETIC_ENERGY:
            return rotationalKineticEnergy;
        case GRAVITATIONAL_POTENTIAL:
            return gravitationalPotential;
        case SPRING_POTENTIAL:
            return springPotential;
        case TOTAL_ENERGY:
            return totalEnergy;
        default:
            throw std::invalid_argument("Invalid BodyKey");
        }
    }
};

inline std::string BodyKeyValue(BodyKeys key)
{
    switch (key)
    {
    case MASS:
        return "mass";
    case MOMENT_OF_INERTIA:
        return "momentOfInertia";
    case DRAG_COEFFICIENT:
        return "dragCoefficient";
    case FRICTION_COEFFICIENT:
        return "frictionCoefficient";
    case RESTITUTION:
        return "restitution";
    case POSITION:
        return "position";
    case POSITION_X:
        return "positionX";
    case POSITION_Y:
        return "positionY";
    case VELOCITY:
        return "velocity";
    case VELOCITY_X:
        return "velocityX";
    case VELOCITY_Y:
        return "velocityY";
    case MOMENTUM:
        return "momentum";
    case MOMENTUM_X:
        return "momentumX";
    case MOMENTUM_Y:
        return "momentumY";
    case ROTATION:
        return "rotation";
    case ANGULAR_VELOCITY:
        return "angularVelocity";
    case ANGULAR_MOMENTUM:
        return "angularMomentum";
    case KINETIC_ENERGY:
        return "kineticEnergy";
    case ROTATIONAL_KINETIC_ENERGY:
        return "rotationalKineticEnergy";
    case GRAVITATIONAL_POTENTIAL:
        return "gravitationalPotential";
    case SPRING_POTENTIAL:
        return "springPotential";
    case TOTAL_ENERGY:
        return "totalEnergy";
    default:
        return "unknown";
    }
}

inline std::string BodyKeyName(BodyKeys key)
{
    switch (key)
    {
    case MASS:
        return "Mass";
    case MOMENT_OF_INERTIA:
        return "Moment of Inertia";
    case DRAG_COEFFICIENT:
        return "Drag Coefficient";
    case FRICTION_COEFFICIENT:
        return "Friction Coefficient";
    case RESTITUTION:
        return "Restitution";
    case POSITION:
        return "Position";
    case POSITION_X:
        return "Position.X";
    case POSITION_Y:
        return "Position.Y";
    case VELOCITY:
        return "Velocity";
    case VELOCITY_X:
        return "Velocity.X";
    case VELOCITY_Y:
        return "Velocity.Y";
    case MOMENTUM:
        return "Momentum";
    case MOMENTUM_X:
        return "Momentum.X";
    case MOMENTUM_Y:
        return "Momentum.Y";
    case ROTATION:
        return "Rotation";
    case ANGULAR_VELOCITY:
        return "Angular Velocity";
    case ANGULAR_MOMENTUM:
        return "Angular Momentum";
    case KINETIC_ENERGY:
        return "Kinetic Energy";
    case ROTATIONAL_KINETIC_ENERGY:
        return "Rotational Kinetic Energy";
    case GRAVITATIONAL_POTENTIAL:
        return "Gravitational Potential";
    case SPRING_POTENTIAL:
        return "Spring Potential";
    case TOTAL_ENERGY:
        return "Total Energy";
    default:
        return "Unknown";
    }
}

inline std::string BodyKeyUnit(BodyKeys key)
{
    switch (key)
    {
    case MASS:
        return "kg";
    case MOMENT_OF_INERTIA:
        return "kg·m²";
    case DRAG_COEFFICIENT:
        return "";
    case FRICTION_COEFFICIENT:
        return "";
    case RESTITUTION:
        return "";
    case POSITION:
        return "m";
    case POSITION_X:
        return "m";
    case POSITION_Y:
        return "m";
    case VELOCITY:
        return "m/s";
    case VELOCITY_X:
        return "m/s";
    case VELOCITY_Y:
        return "m/s";
    case MOMENTUM:
        return "kg·m/s";
    case MOMENTUM_X:
        return "kg·m/s";
    case MOMENTUM_Y:
        return "kg·m/s";
    case ROTATION:
        return "rad";
    case ANGULAR_VELOCITY:
        return "rad/s";
    case ANGULAR_MOMENTUM:
        return "kg·m²/s";
    case KINETIC_ENERGY:
        return "J";
    case ROTATIONAL_KINETIC_ENERGY:
        return "J";
    case GRAVITATIONAL_POTENTIAL:
        return "J";
    case SPRING_POTENTIAL:
        return "J";
    case TOTAL_ENERGY:
        return "J";
    default:
        return "";
    }
}
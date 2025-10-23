#pragma once

#include "objects/Body.hpp"

enum SolverType
{
    EULER,
    RK2,
    RK4,
};

inline std::string solverTypeToString(SolverType type)
{
    switch (type)
    {
    case EULER:
        return "Euler";
    case RK2:
        return "RK2";
    case RK4:
        return "RK4";
    default:
        return "Unknown";
    }
}

class ODESolver {
protected:
    Body *body;
public:
    ODESolver(Body *initialState) : body(initialState) {}
    virtual void step(float dt) = 0;
    virtual Body simulate(float dt) = 0;
    virtual ~ODESolver() = default;
};

class EulerSolver : public ODESolver {
public:
    EulerSolver(Body *initialState) : ODESolver(initialState) {}
    void step(float dt) override {
        body->acceleration = body->netForce * body->invMass;
        body->velocity += body->acceleration * dt;
        body->position += body->velocity * dt;
    }
    Body simulate(float dt) override {
        Body tempBody = *body;
        tempBody.acceleration = tempBody.netForce * tempBody.invMass;
        tempBody.velocity += tempBody.acceleration * dt;
        tempBody.position += tempBody.velocity * dt;
        return tempBody;
    }
};

class RK2Solver : public ODESolver {
public:
    RK2Solver(Body *initialState) : ODESolver(initialState) {}
    void step(float dt) override {
        Vec2 initialPosition = body->position;
        Vec2 initialVelocity = body->velocity;
        body->acceleration = body->netForce * body->invMass;

        Vec2 k1_v = body->acceleration * dt;
        Vec2 k1_x = initialVelocity * dt;

        Vec2 k2_v = (body->netForce * body->invMass) * dt;
        Vec2 k2_x = (initialVelocity + k1_v * 0.5f) * dt;

        body->velocity += k2_v;
        body->position += k2_x;
    }
    Body simulate(float dt) override {
        Body tempBody = *body;
        Vec2 initialPosition = tempBody.position;
        Vec2 initialVelocity = tempBody.velocity;
        tempBody.acceleration = tempBody.netForce * tempBody.invMass;

        Vec2 k1_v = tempBody.acceleration * dt;
        Vec2 k1_x = initialVelocity * dt;

        Vec2 k2_v = (tempBody.netForce * tempBody.invMass) * dt;
        Vec2 k2_x = (initialVelocity + k1_v * 0.5f) * dt;

        tempBody.velocity += k2_v;
        tempBody.position += k2_x;

        return tempBody;
    }
};

class RK4Solver : public ODESolver {
public:
    RK4Solver(Body *initialState) : ODESolver(initialState) {}
    void step(float dt) override {
        Vec2 initialPosition = body->position;
        Vec2 initialVelocity = body->velocity;
        body->acceleration = body->netForce * body->invMass;

        Vec2 k1_v = body->acceleration * dt;
        Vec2 k1_x = initialVelocity * dt;

        Vec2 k2_v = (body->netForce * body->invMass) * dt;
        Vec2 k2_x = (initialVelocity + k1_v * 0.5f) * dt;

        Vec2 k3_v = (body->netForce * body->invMass) * dt;
        Vec2 k3_x = (initialVelocity + k2_v * 0.5f) * dt;

        Vec2 k4_v = (body->netForce * body->invMass) * dt;
        Vec2 k4_x = (initialVelocity + k3_v) * dt;

        body->velocity += (k1_v + k2_v * 2.0f + k3_v * 2.0f + k4_v) / 6.0f;
        body->position += (k1_x + k2_x * 2.0f + k3_x * 2.0f + k4_x) / 6.0f;
    }
    Body simulate(float dt) override {
        Body tempBody = *body;
        Vec2 initialPosition = tempBody.position;
        Vec2 initialVelocity = tempBody.velocity;
        tempBody.acceleration = tempBody.netForce * tempBody.invMass;

        Vec2 k1_v = tempBody.acceleration * dt;
        Vec2 k1_x = initialVelocity * dt;

        Vec2 k2_v = (tempBody.netForce * tempBody.invMass) * dt;
        Vec2 k2_x = (initialVelocity + k1_v * 0.5f) * dt;

        Vec2 k3_v = (tempBody.netForce * tempBody.invMass) * dt;
        Vec2 k3_x = (initialVelocity + k2_v * 0.5f) * dt;

        Vec2 k4_v = (tempBody.netForce * tempBody.invMass) * dt;
        Vec2 k4_x = (initialVelocity + k3_v) * dt;

        tempBody.velocity += (k1_v + k2_v * 2.0f + k3_v * 2.0f + k4_v) / 6.0f;
        tempBody.position += (k1_x + k2_x * 2.0f + k3_x * 2.0f + k4_x) / 6.0f;

        return tempBody;
    }
};
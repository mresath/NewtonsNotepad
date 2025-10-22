#pragma once

#include "objects/Body.hpp"

enum SolverType
{
    EULER,
    RK4,
};

class ODESolver {
protected:
    Body *body;
public:
    ODESolver(Body *initialState) : body(initialState) {}
    virtual void step(float dt) = 0;
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
};
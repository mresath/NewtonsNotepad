#include "ODE.hpp"

// Helper structure to hold derivatives for a single RK stage
struct RKDerivatives {
    Vec2 velocity;
    Vec2 acceleration;
    float angularVelocity;
    float angularAcceleration;
};

// Helper function to evaluate derivatives at a given state
static RKDerivatives evaluateDerivatives(const Body& currentBody, Object* object, float dt, float factor, const RKDerivatives& prevDeriv, bool isInitialStage) {
    RKDerivatives deriv;
    
    if (isInitialStage) {
        // K1: Use current state directly
        std::tuple<Force, float> ft = object->getNetForce();
        Vec2 force = std::get<0>(ft).force;
        float torque = std::get<1>(ft);
        
        deriv.velocity = currentBody.velocity;
        deriv.acceleration = force * currentBody.invMass;
        deriv.angularVelocity = currentBody.angularVelocity;
        deriv.angularAcceleration = torque * currentBody.invMomentOfInertia;
    } else {
        // K2/K3/K4: Create intermediate body state
        Body intermediateBody = currentBody;
        intermediateBody.position = currentBody.position + prevDeriv.velocity * (dt * factor);
        intermediateBody.velocity = currentBody.velocity + prevDeriv.acceleration * (dt * factor);
        intermediateBody.rotation = currentBody.rotation + prevDeriv.angularVelocity * (dt * factor);
        intermediateBody.angularVelocity = currentBody.angularVelocity + prevDeriv.angularAcceleration * (dt * factor);
        intermediateBody.netForce = Vec2(0.0f, 0.0f);
        intermediateBody.netTorque = 0.0f;
        
        deriv.velocity = intermediateBody.velocity;
        deriv.acceleration = Vec2(0.0f, 0.0f);
        deriv.angularVelocity = intermediateBody.angularVelocity;
        deriv.angularAcceleration = 0.0f;
        
        // Accumulate forces from all sources
        for (auto forceSource : object->getForces()) {
            std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody);
            deriv.acceleration += std::get<0>(ft).force * currentBody.invMass;
            deriv.angularAcceleration += std::get<1>(ft) * currentBody.invMomentOfInertia;
        }
    }
    
    return deriv;
}

// Euler Solver Implementation
void EulerSolver::step(float dt)
{
    Body *body = object->body;

    std::tuple<Force, float> netForceTorque = object->getNetForce();

    body->netForce = std::get<0>(netForceTorque).force;
    body->acceleration = body->netForce * body->invMass;
    body->velocity += body->acceleration * dt;
    body->position += body->velocity * dt;

    body->netTorque = std::get<1>(netForceTorque);
    body->angularAcceleration = body->netTorque * body->invMomentOfInertia;
    body->angularVelocity += body->angularAcceleration * dt;
    body->rotation += body->angularVelocity * dt;
}
Body EulerSolver::simulate(float dt)
{
    Body tempBody = *object->body;

    std::tuple<Force, float> netForceTorque = object->getNetForce();

    tempBody.netForce = std::get<0>(netForceTorque).force;
    tempBody.acceleration = tempBody.netForce * tempBody.invMass;
    tempBody.velocity += tempBody.acceleration * dt;
    tempBody.position += tempBody.velocity * dt;

    tempBody.netTorque = std::get<1>(netForceTorque);
    tempBody.angularAcceleration = tempBody.netTorque * tempBody.invMomentOfInertia;
    tempBody.angularVelocity += tempBody.angularAcceleration * dt;
    tempBody.rotation += tempBody.angularVelocity * dt;

    return tempBody;
}

// RK2 Solver Implementation
void RK2Solver::step(float dt)
{
    Body *body = object->body;

    // K1: Evaluate at the current state
    RKDerivatives k1 = evaluateDerivatives(*body, object, dt, 0.0f, {}, true);
    body->netForce = k1.acceleration * body->mass;
    body->netTorque = k1.angularAcceleration * body->momentOfInertia;

    // K2: Evaluate at the midpoint using K1
    RKDerivatives k2 = evaluateDerivatives(*body, object, dt, 0.5f, k1, false);

    // Update using K2 (the midpoint slope)
    body->velocity += k2.acceleration * dt;
    body->position += k2.velocity * dt;
    body->acceleration = k2.acceleration;

    body->angularVelocity += k2.angularAcceleration * dt;
    body->rotation += k2.angularVelocity * dt;
    body->angularAcceleration = k2.angularAcceleration;
}
Body RK2Solver::simulate(float dt)
{
    Body tempBody = *object->body;

    // K1: Evaluate at the current state
    RKDerivatives k1 = evaluateDerivatives(tempBody, object, dt, 0.0f, {}, true);
    tempBody.netForce = k1.acceleration * tempBody.mass;
    tempBody.netTorque = k1.angularAcceleration * tempBody.momentOfInertia;

    // K2: Evaluate at the midpoint using K1
    RKDerivatives k2 = evaluateDerivatives(tempBody, object, dt, 0.5f, k1, false);

    // Update using K2 (the midpoint slope)
    tempBody.velocity += k2.acceleration * dt;
    tempBody.position += k2.velocity * dt;
    tempBody.acceleration = k2.acceleration;

    tempBody.angularVelocity += k2.angularAcceleration * dt;
    tempBody.rotation += k2.angularVelocity * dt;
    tempBody.angularAcceleration = k2.angularAcceleration;

    return tempBody;
}

// RK4 Solver Implementation
void RK4Solver::step(float dt)
{
    Body *body = object->body;

    // K1: Evaluate at the current state
    RKDerivatives k1 = evaluateDerivatives(*body, object, dt, 0.0f, {}, true);
    body->netForce = k1.acceleration * body->mass;
    body->netTorque = k1.angularAcceleration * body->momentOfInertia;

    // K2: Evaluate at the midpoint using K1
    RKDerivatives k2 = evaluateDerivatives(*body, object, dt, 0.5f, k1, false);

    // K3: Evaluate at the midpoint using K2
    RKDerivatives k3 = evaluateDerivatives(*body, object, dt, 0.5f, k2, false);

    // K4: Evaluate at the endpoint using K3
    RKDerivatives k4 = evaluateDerivatives(*body, object, dt, 1.0f, k3, false);

    // Weighted average: (k1 + 2*k2 + 2*k3 + k4) / 6
    body->velocity += (k1.acceleration + k2.acceleration * 2.0f + k3.acceleration * 2.0f + k4.acceleration) * (dt / 6.0f);
    body->position += (k1.velocity + k2.velocity * 2.0f + k3.velocity * 2.0f + k4.velocity) * (dt / 6.0f);
    body->acceleration = (k1.acceleration + k2.acceleration * 2.0f + k3.acceleration * 2.0f + k4.acceleration) / 6.0f;

    body->angularVelocity += (k1.angularAcceleration + k2.angularAcceleration * 2.0f + k3.angularAcceleration * 2.0f + k4.angularAcceleration) * (dt / 6.0f);
    body->rotation += (k1.angularVelocity + k2.angularVelocity * 2.0f + k3.angularVelocity * 2.0f + k4.angularVelocity) * (dt / 6.0f);
    body->angularAcceleration = (k1.angularAcceleration + k2.angularAcceleration * 2.0f + k3.angularAcceleration * 2.0f + k4.angularAcceleration) / 6.0f;
}
Body RK4Solver::simulate(float dt) {
    Body tempBody = *object->body;

    // K1: Evaluate at the current state
    RKDerivatives k1 = evaluateDerivatives(tempBody, object, dt, 0.0f, {}, true);
    tempBody.netForce = k1.acceleration * tempBody.mass;
    tempBody.netTorque = k1.angularAcceleration * tempBody.momentOfInertia;

    // K2: Evaluate at the midpoint using K1
    RKDerivatives k2 = evaluateDerivatives(tempBody, object, dt, 0.5f, k1, false);

    // K3: Evaluate at the midpoint using K2
    RKDerivatives k3 = evaluateDerivatives(tempBody, object, dt, 0.5f, k2, false);

    // K4: Evaluate at the endpoint using K3
    RKDerivatives k4 = evaluateDerivatives(tempBody, object, dt, 1.0f, k3, false);

    // Weighted average: (k1 + 2*k2 + 2*k3 + k4) / 6
    tempBody.velocity += (k1.acceleration + k2.acceleration * 2.0f + k3.acceleration * 2.0f + k4.acceleration) * (dt / 6.0f);
    tempBody.position += (k1.velocity + k2.velocity * 2.0f + k3.velocity * 2.0f + k4.velocity) * (dt / 6.0f);
    tempBody.acceleration = (k1.acceleration + k2.acceleration * 2.0f + k3.acceleration * 2.0f + k4.acceleration) / 6.0f;

    tempBody.angularVelocity += (k1.angularAcceleration + k2.angularAcceleration * 2.0f + k3.angularAcceleration * 2.0f + k4.angularAcceleration) * (dt / 6.0f);
    tempBody.rotation += (k1.angularVelocity + k2.angularVelocity * 2.0f + k3.angularVelocity * 2.0f + k4.angularVelocity) * (dt / 6.0f);
    tempBody.angularAcceleration = (k1.angularAcceleration + k2.angularAcceleration * 2.0f + k3.angularAcceleration * 2.0f + k4.angularAcceleration) / 6.0f;

    return tempBody;
}
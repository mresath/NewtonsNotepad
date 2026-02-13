#include "ODE.hpp"

// Helper structure to hold derivatives for a single RK stage
struct RKDerivatives
{
    Vec2 velocity;
    Vec2 acceleration;
    float angularVelocity;
    float angularAcceleration;
};

// Helper function to evaluate derivatives at a given state
static RKDerivatives evaluateDerivatives(const Body &currentBody, Object *object, float dt, float factor, const RKDerivatives &prevDeriv, bool isInitialStage)
{
    RKDerivatives deriv;

    if (isInitialStage)
    {
        // K1: Use current state directly
        std::tuple<Force, float> ft = object->getNetForce();
        Vec2 force = std::get<0>(ft).force;
        float torque = std::get<1>(ft);

        deriv.velocity = currentBody.velocity;
        deriv.acceleration = force * currentBody.invMass;
        deriv.angularVelocity = currentBody.angularVelocity;
        deriv.angularAcceleration = torque * currentBody.invMomentOfInertia;
    }
    else
    {
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
        for (auto forceSource : object->getForces())
        {
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
    body->position += body->velocity * dt;
    body->velocity += body->acceleration * dt;

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
    tempBody.position += tempBody.velocity * dt;
    tempBody.velocity += tempBody.acceleration * dt;

    tempBody.netTorque = std::get<1>(netForceTorque);
    tempBody.angularAcceleration = tempBody.netTorque * tempBody.invMomentOfInertia;
    tempBody.rotation += tempBody.angularVelocity * dt;
    tempBody.angularVelocity += tempBody.angularAcceleration * dt;

    return tempBody;
}

// Euler (Symplectic) Solver Implementation
void EulerSympSolver::step(float dt)
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
Body EulerSympSolver::simulate(float dt)
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
Body RK4Solver::simulate(float dt)
{
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

// DOPRI5 Solver Implementation
void DOPRI5Solver::step(float dt)
{
    Body *body = object->body;

    // K1: Evaluate at the current state
    RKDerivatives k1 = evaluateDerivatives(*body, object, dt, 0.0f, {}, true);
    body->netForce = k1.acceleration * body->mass;
    body->netTorque = k1.angularAcceleration * body->momentOfInertia;

    // K2: Evaluate at t + (1/5)*dt
    RKDerivatives k2 = evaluateDerivatives(*body, object, dt, 1.0f / 5.0f, k1, false);

    // K3: Evaluate at t + (3/10)*dt
    // Use weighted combination: 3/40*k1 + 9/40*k2
    RKDerivatives k3_input;
    k3_input.velocity = k1.velocity * (3.0f / 40.0f) + k2.velocity * (9.0f / 40.0f);
    k3_input.acceleration = k1.acceleration * (3.0f / 40.0f) + k2.acceleration * (9.0f / 40.0f);
    k3_input.angularVelocity = k1.angularVelocity * (3.0f / 40.0f) + k2.angularVelocity * (9.0f / 40.0f);
    k3_input.angularAcceleration = k1.angularAcceleration * (3.0f / 40.0f) + k2.angularAcceleration * (9.0f / 40.0f);

    Body intermediateBody3 = *body;
    intermediateBody3.position = body->position + k3_input.velocity * dt;
    intermediateBody3.velocity = body->velocity + k3_input.acceleration * dt;
    intermediateBody3.rotation = body->rotation + k3_input.angularVelocity * dt;
    intermediateBody3.angularVelocity = body->angularVelocity + k3_input.angularAcceleration * dt;

    RKDerivatives k3;
    k3.velocity = intermediateBody3.velocity;
    k3.angularVelocity = intermediateBody3.angularVelocity;
    k3.acceleration = Vec2(0.0f, 0.0f);
    k3.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody3);
        k3.acceleration += std::get<0>(ft).force * body->invMass;
        k3.angularAcceleration += std::get<1>(ft) * body->invMomentOfInertia;
    }

    // K4: Evaluate at t + (4/5)*dt
    // Use weighted combination: 44/45*k1 - 56/15*k2 + 32/9*k3
    RKDerivatives k4_input;
    k4_input.velocity = k1.velocity * (44.0f / 45.0f) - k2.velocity * (56.0f / 15.0f) + k3.velocity * (32.0f / 9.0f);
    k4_input.acceleration = k1.acceleration * (44.0f / 45.0f) - k2.acceleration * (56.0f / 15.0f) + k3.acceleration * (32.0f / 9.0f);
    k4_input.angularVelocity = k1.angularVelocity * (44.0f / 45.0f) - k2.angularVelocity * (56.0f / 15.0f) + k3.angularVelocity * (32.0f / 9.0f);
    k4_input.angularAcceleration = k1.angularAcceleration * (44.0f / 45.0f) - k2.angularAcceleration * (56.0f / 15.0f) + k3.angularAcceleration * (32.0f / 9.0f);

    Body intermediateBody4 = *body;
    intermediateBody4.position = body->position + k4_input.velocity * dt;
    intermediateBody4.velocity = body->velocity + k4_input.acceleration * dt;
    intermediateBody4.rotation = body->rotation + k4_input.angularVelocity * dt;
    intermediateBody4.angularVelocity = body->angularVelocity + k4_input.angularAcceleration * dt;

    RKDerivatives k4;
    k4.velocity = intermediateBody4.velocity;
    k4.angularVelocity = intermediateBody4.angularVelocity;
    k4.acceleration = Vec2(0.0f, 0.0f);
    k4.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody4);
        k4.acceleration += std::get<0>(ft).force * body->invMass;
        k4.angularAcceleration += std::get<1>(ft) * body->invMomentOfInertia;
    }

    // K5: Evaluate at t + (8/9)*dt
    // Use weighted combination: 19372/6561*k1 - 25360/2187*k2 + 64448/6561*k3 - 212/729*k4
    RKDerivatives k5_input;
    k5_input.velocity = k1.velocity * (19372.0f / 6561.0f) - k2.velocity * (25360.0f / 2187.0f) + k3.velocity * (64448.0f / 6561.0f) - k4.velocity * (212.0f / 729.0f);
    k5_input.acceleration = k1.acceleration * (19372.0f / 6561.0f) - k2.acceleration * (25360.0f / 2187.0f) + k3.acceleration * (64448.0f / 6561.0f) - k4.acceleration * (212.0f / 729.0f);
    k5_input.angularVelocity = k1.angularVelocity * (19372.0f / 6561.0f) - k2.angularVelocity * (25360.0f / 2187.0f) + k3.angularVelocity * (64448.0f / 6561.0f) - k4.angularVelocity * (212.0f / 729.0f);
    k5_input.angularAcceleration = k1.angularAcceleration * (19372.0f / 6561.0f) - k2.angularAcceleration * (25360.0f / 2187.0f) + k3.angularAcceleration * (64448.0f / 6561.0f) - k4.angularAcceleration * (212.0f / 729.0f);

    Body intermediateBody5 = *body;
    intermediateBody5.position = body->position + k5_input.velocity * dt;
    intermediateBody5.velocity = body->velocity + k5_input.acceleration * dt;
    intermediateBody5.rotation = body->rotation + k5_input.angularVelocity * dt;
    intermediateBody5.angularVelocity = body->angularVelocity + k5_input.angularAcceleration * dt;

    RKDerivatives k5;
    k5.velocity = intermediateBody5.velocity;
    k5.angularVelocity = intermediateBody5.angularVelocity;
    k5.acceleration = Vec2(0.0f, 0.0f);
    k5.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody5);
        k5.acceleration += std::get<0>(ft).force * body->invMass;
        k5.angularAcceleration += std::get<1>(ft) * body->invMomentOfInertia;
    }

    // K6: Evaluate at t + dt
    // Use weighted combination: 9017/3168*k1 - 355/33*k2 + 46732/5247*k3 + 49/176*k4 - 5103/18656*k5
    RKDerivatives k6_input;
    k6_input.velocity = k1.velocity * (9017.0f / 3168.0f) - k2.velocity * (355.0f / 33.0f) + k3.velocity * (46732.0f / 5247.0f) + k4.velocity * (49.0f / 176.0f) - k5.velocity * (5103.0f / 18656.0f);
    k6_input.acceleration = k1.acceleration * (9017.0f / 3168.0f) - k2.acceleration * (355.0f / 33.0f) + k3.acceleration * (46732.0f / 5247.0f) + k4.acceleration * (49.0f / 176.0f) - k5.acceleration * (5103.0f / 18656.0f);
    k6_input.angularVelocity = k1.angularVelocity * (9017.0f / 3168.0f) - k2.angularVelocity * (355.0f / 33.0f) + k3.angularVelocity * (46732.0f / 5247.0f) + k4.angularVelocity * (49.0f / 176.0f) - k5.angularVelocity * (5103.0f / 18656.0f);
    k6_input.angularAcceleration = k1.angularAcceleration * (9017.0f / 3168.0f) - k2.angularAcceleration * (355.0f / 33.0f) + k3.angularAcceleration * (46732.0f / 5247.0f) + k4.angularAcceleration * (49.0f / 176.0f) - k5.angularAcceleration * (5103.0f / 18656.0f);

    Body intermediateBody6 = *body;
    intermediateBody6.position = body->position + k6_input.velocity * dt;
    intermediateBody6.velocity = body->velocity + k6_input.acceleration * dt;
    intermediateBody6.rotation = body->rotation + k6_input.angularVelocity * dt;
    intermediateBody6.angularVelocity = body->angularVelocity + k6_input.angularAcceleration * dt;

    RKDerivatives k6;
    k6.velocity = intermediateBody6.velocity;
    k6.angularVelocity = intermediateBody6.angularVelocity;
    k6.acceleration = Vec2(0.0f, 0.0f);
    k6.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody6);
        k6.acceleration += std::get<0>(ft).force * body->invMass;
        k6.angularAcceleration += std::get<1>(ft) * body->invMomentOfInertia;
    }

    // K7: Evaluate at t + dt (same as k6 position, but different formula)
    // Use weighted combination: 35/384*k1 + 0*k2 + 500/1113*k3 + 125/192*k4 - 2187/6784*k5 + 11/84*k6
    RKDerivatives k7_input;
    k7_input.velocity = k1.velocity * (35.0f / 384.0f) + k3.velocity * (500.0f / 1113.0f) + k4.velocity * (125.0f / 192.0f) - k5.velocity * (2187.0f / 6784.0f) + k6.velocity * (11.0f / 84.0f);
    k7_input.acceleration = k1.acceleration * (35.0f / 384.0f) + k3.acceleration * (500.0f / 1113.0f) + k4.acceleration * (125.0f / 192.0f) - k5.acceleration * (2187.0f / 6784.0f) + k6.acceleration * (11.0f / 84.0f);
    k7_input.angularVelocity = k1.angularVelocity * (35.0f / 384.0f) + k3.angularVelocity * (500.0f / 1113.0f) + k4.angularVelocity * (125.0f / 192.0f) - k5.angularVelocity * (2187.0f / 6784.0f) + k6.angularVelocity * (11.0f / 84.0f);
    k7_input.angularAcceleration = k1.angularAcceleration * (35.0f / 384.0f) + k3.angularAcceleration * (500.0f / 1113.0f) + k4.angularAcceleration * (125.0f / 192.0f) - k5.angularAcceleration * (2187.0f / 6784.0f) + k6.angularAcceleration * (11.0f / 84.0f);

    Body intermediateBody7 = *body;
    intermediateBody7.position = body->position + k7_input.velocity * dt;
    intermediateBody7.velocity = body->velocity + k7_input.acceleration * dt;
    intermediateBody7.rotation = body->rotation + k7_input.angularVelocity * dt;
    intermediateBody7.angularVelocity = body->angularVelocity + k7_input.angularAcceleration * dt;

    RKDerivatives k7;
    k7.velocity = intermediateBody7.velocity;
    k7.angularVelocity = intermediateBody7.angularVelocity;
    k7.acceleration = Vec2(0.0f, 0.0f);
    k7.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody7);
        k7.acceleration += std::get<0>(ft).force * body->invMass;
        k7.angularAcceleration += std::get<1>(ft) * body->invMomentOfInertia;
    }

    // Final weighted combination for 5th order solution
    // y_n+1 = y_n + dt * (35/384*k1 + 0*k2 + 500/1113*k3 + 125/192*k4 - 2187/6784*k5 + 11/84*k6 + 0*k7)
    body->velocity += (k1.acceleration * (35.0f / 384.0f) + k3.acceleration * (500.0f / 1113.0f) + k4.acceleration * (125.0f / 192.0f) - k5.acceleration * (2187.0f / 6784.0f) + k6.acceleration * (11.0f / 84.0f)) * dt;
    body->position += (k1.velocity * (35.0f / 384.0f) + k3.velocity * (500.0f / 1113.0f) + k4.velocity * (125.0f / 192.0f) - k5.velocity * (2187.0f / 6784.0f) + k6.velocity * (11.0f / 84.0f)) * dt;
    body->acceleration = k1.acceleration * (35.0f / 384.0f) + k3.acceleration * (500.0f / 1113.0f) + k4.acceleration * (125.0f / 192.0f) - k5.acceleration * (2187.0f / 6784.0f) + k6.acceleration * (11.0f / 84.0f);

    body->angularVelocity += (k1.angularAcceleration * (35.0f / 384.0f) + k3.angularAcceleration * (500.0f / 1113.0f) + k4.angularAcceleration * (125.0f / 192.0f) - k5.angularAcceleration * (2187.0f / 6784.0f) + k6.angularAcceleration * (11.0f / 84.0f)) * dt;
    body->rotation += (k1.angularVelocity * (35.0f / 384.0f) + k3.angularVelocity * (500.0f / 1113.0f) + k4.angularVelocity * (125.0f / 192.0f) - k5.angularVelocity * (2187.0f / 6784.0f) + k6.angularVelocity * (11.0f / 84.0f)) * dt;
    body->angularAcceleration = k1.angularAcceleration * (35.0f / 384.0f) + k3.angularAcceleration * (500.0f / 1113.0f) + k4.angularAcceleration * (125.0f / 192.0f) - k5.angularAcceleration * (2187.0f / 6784.0f) + k6.angularAcceleration * (11.0f / 84.0f);
}
Body DOPRI5Solver::simulate(float dt)
{
    Body tempBody = *object->body;

    // K1: Evaluate at the current state
    RKDerivatives k1 = evaluateDerivatives(tempBody, object, dt, 0.0f, {}, true);
    tempBody.netForce = k1.acceleration * tempBody.mass;
    tempBody.netTorque = k1.angularAcceleration * tempBody.momentOfInertia;

    // K2: Evaluate at t + (1/5)*dt
    RKDerivatives k2 = evaluateDerivatives(tempBody, object, dt, 1.0f / 5.0f, k1, false);

    // K3: Evaluate at t + (3/10)*dt
    RKDerivatives k3_input;
    k3_input.velocity = k1.velocity * (3.0f / 40.0f) + k2.velocity * (9.0f / 40.0f);
    k3_input.acceleration = k1.acceleration * (3.0f / 40.0f) + k2.acceleration * (9.0f / 40.0f);
    k3_input.angularVelocity = k1.angularVelocity * (3.0f / 40.0f) + k2.angularVelocity * (9.0f / 40.0f);
    k3_input.angularAcceleration = k1.angularAcceleration * (3.0f / 40.0f) + k2.angularAcceleration * (9.0f / 40.0f);

    Body intermediateBody3 = tempBody;
    intermediateBody3.position = tempBody.position + k3_input.velocity * dt;
    intermediateBody3.velocity = tempBody.velocity + k3_input.acceleration * dt;
    intermediateBody3.rotation = tempBody.rotation + k3_input.angularVelocity * dt;
    intermediateBody3.angularVelocity = tempBody.angularVelocity + k3_input.angularAcceleration * dt;

    RKDerivatives k3;
    k3.velocity = intermediateBody3.velocity;
    k3.angularVelocity = intermediateBody3.angularVelocity;
    k3.acceleration = Vec2(0.0f, 0.0f);
    k3.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody3);
        k3.acceleration += std::get<0>(ft).force * tempBody.invMass;
        k3.angularAcceleration += std::get<1>(ft) * tempBody.invMomentOfInertia;
    }

    // K4: Evaluate at t + (4/5)*dt
    RKDerivatives k4_input;
    k4_input.velocity = k1.velocity * (44.0f / 45.0f) - k2.velocity * (56.0f / 15.0f) + k3.velocity * (32.0f / 9.0f);
    k4_input.acceleration = k1.acceleration * (44.0f / 45.0f) - k2.acceleration * (56.0f / 15.0f) + k3.acceleration * (32.0f / 9.0f);
    k4_input.angularVelocity = k1.angularVelocity * (44.0f / 45.0f) - k2.angularVelocity * (56.0f / 15.0f) + k3.angularVelocity * (32.0f / 9.0f);
    k4_input.angularAcceleration = k1.angularAcceleration * (44.0f / 45.0f) - k2.angularAcceleration * (56.0f / 15.0f) + k3.angularAcceleration * (32.0f / 9.0f);

    Body intermediateBody4 = tempBody;
    intermediateBody4.position = tempBody.position + k4_input.velocity * dt;
    intermediateBody4.velocity = tempBody.velocity + k4_input.acceleration * dt;
    intermediateBody4.rotation = tempBody.rotation + k4_input.angularVelocity * dt;
    intermediateBody4.angularVelocity = tempBody.angularVelocity + k4_input.angularAcceleration * dt;

    RKDerivatives k4;
    k4.velocity = intermediateBody4.velocity;
    k4.angularVelocity = intermediateBody4.angularVelocity;
    k4.acceleration = Vec2(0.0f, 0.0f);
    k4.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody4);
        k4.acceleration += std::get<0>(ft).force * tempBody.invMass;
        k4.angularAcceleration += std::get<1>(ft) * tempBody.invMomentOfInertia;
    }

    // K5: Evaluate at t + (8/9)*dt
    RKDerivatives k5_input;
    k5_input.velocity = k1.velocity * (19372.0f / 6561.0f) - k2.velocity * (25360.0f / 2187.0f) + k3.velocity * (64448.0f / 6561.0f) - k4.velocity * (212.0f / 729.0f);
    k5_input.acceleration = k1.acceleration * (19372.0f / 6561.0f) - k2.acceleration * (25360.0f / 2187.0f) + k3.acceleration * (64448.0f / 6561.0f) - k4.acceleration * (212.0f / 729.0f);
    k5_input.angularVelocity = k1.angularVelocity * (19372.0f / 6561.0f) - k2.angularVelocity * (25360.0f / 2187.0f) + k3.angularVelocity * (64448.0f / 6561.0f) - k4.angularVelocity * (212.0f / 729.0f);
    k5_input.angularAcceleration = k1.angularAcceleration * (19372.0f / 6561.0f) - k2.angularAcceleration * (25360.0f / 2187.0f) + k3.angularAcceleration * (64448.0f / 6561.0f) - k4.angularAcceleration * (212.0f / 729.0f);

    Body intermediateBody5 = tempBody;
    intermediateBody5.position = tempBody.position + k5_input.velocity * dt;
    intermediateBody5.velocity = tempBody.velocity + k5_input.acceleration * dt;
    intermediateBody5.rotation = tempBody.rotation + k5_input.angularVelocity * dt;
    intermediateBody5.angularVelocity = tempBody.angularVelocity + k5_input.angularAcceleration * dt;

    RKDerivatives k5;
    k5.velocity = intermediateBody5.velocity;
    k5.angularVelocity = intermediateBody5.angularVelocity;
    k5.acceleration = Vec2(0.0f, 0.0f);
    k5.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody5);
        k5.acceleration += std::get<0>(ft).force * tempBody.invMass;
        k5.angularAcceleration += std::get<1>(ft) * tempBody.invMomentOfInertia;
    }

    // K6: Evaluate at t + dt
    RKDerivatives k6_input;
    k6_input.velocity = k1.velocity * (9017.0f / 3168.0f) - k2.velocity * (355.0f / 33.0f) + k3.velocity * (46732.0f / 5247.0f) + k4.velocity * (49.0f / 176.0f) - k5.velocity * (5103.0f / 18656.0f);
    k6_input.acceleration = k1.acceleration * (9017.0f / 3168.0f) - k2.acceleration * (355.0f / 33.0f) + k3.acceleration * (46732.0f / 5247.0f) + k4.acceleration * (49.0f / 176.0f) - k5.acceleration * (5103.0f / 18656.0f);
    k6_input.angularVelocity = k1.angularVelocity * (9017.0f / 3168.0f) - k2.angularVelocity * (355.0f / 33.0f) + k3.angularVelocity * (46732.0f / 5247.0f) + k4.angularVelocity * (49.0f / 176.0f) - k5.angularVelocity * (5103.0f / 18656.0f);
    k6_input.angularAcceleration = k1.angularAcceleration * (9017.0f / 3168.0f) - k2.angularAcceleration * (355.0f / 33.0f) + k3.angularAcceleration * (46732.0f / 5247.0f) + k4.angularAcceleration * (49.0f / 176.0f) - k5.angularAcceleration * (5103.0f / 18656.0f);

    Body intermediateBody6 = tempBody;
    intermediateBody6.position = tempBody.position + k6_input.velocity * dt;
    intermediateBody6.velocity = tempBody.velocity + k6_input.acceleration * dt;
    intermediateBody6.rotation = tempBody.rotation + k6_input.angularVelocity * dt;
    intermediateBody6.angularVelocity = tempBody.angularVelocity + k6_input.angularAcceleration * dt;

    RKDerivatives k6;
    k6.velocity = intermediateBody6.velocity;
    k6.angularVelocity = intermediateBody6.angularVelocity;
    k6.acceleration = Vec2(0.0f, 0.0f);
    k6.angularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(intermediateBody6);
        k6.acceleration += std::get<0>(ft).force * tempBody.invMass;
        k6.angularAcceleration += std::get<1>(ft) * tempBody.invMomentOfInertia;
    }

    // Final weighted combination for 5th order solution
    tempBody.velocity += (k1.acceleration * (35.0f / 384.0f) + k3.acceleration * (500.0f / 1113.0f) + k4.acceleration * (125.0f / 192.0f) - k5.acceleration * (2187.0f / 6784.0f) + k6.acceleration * (11.0f / 84.0f)) * dt;
    tempBody.position += (k1.velocity * (35.0f / 384.0f) + k3.velocity * (500.0f / 1113.0f) + k4.velocity * (125.0f / 192.0f) - k5.velocity * (2187.0f / 6784.0f) + k6.velocity * (11.0f / 84.0f)) * dt;
    tempBody.acceleration = k1.acceleration * (35.0f / 384.0f) + k3.acceleration * (500.0f / 1113.0f) + k4.acceleration * (125.0f / 192.0f) - k5.acceleration * (2187.0f / 6784.0f) + k6.acceleration * (11.0f / 84.0f);

    tempBody.angularVelocity += (k1.angularAcceleration * (35.0f / 384.0f) + k3.angularAcceleration * (500.0f / 1113.0f) + k4.angularAcceleration * (125.0f / 192.0f) - k5.angularAcceleration * (2187.0f / 6784.0f) + k6.angularAcceleration * (11.0f / 84.0f)) * dt;
    tempBody.rotation += (k1.angularVelocity * (35.0f / 384.0f) + k3.angularVelocity * (500.0f / 1113.0f) + k4.angularVelocity * (125.0f / 192.0f) - k5.angularVelocity * (2187.0f / 6784.0f) + k6.angularVelocity * (11.0f / 84.0f)) * dt;
    tempBody.angularAcceleration = k1.angularAcceleration * (35.0f / 384.0f) + k3.angularAcceleration * (500.0f / 1113.0f) + k4.angularAcceleration * (125.0f / 192.0f) - k5.angularAcceleration * (2187.0f / 6784.0f) + k6.angularAcceleration * (11.0f / 84.0f);

    return tempBody;
}

// Velocity Verlet Solver Implementation
void VerletSolver::step(float dt)
{
    Body *body = object->body;

    // Get current acceleration
    std::tuple<Force, float> netForceTorque = object->getNetForce();
    body->netForce = std::get<0>(netForceTorque).force;
    body->netTorque = std::get<1>(netForceTorque);
    Vec2 a_n = body->netForce * body->invMass;
    float alpha_n = body->netTorque * body->invMomentOfInertia;

    // Half-step velocity update
    Vec2 v_half = body->velocity + a_n * (dt * 0.5f);
    float omega_half = body->angularVelocity + alpha_n * (dt * 0.5f);

    // Full-step position update using half-step velocity
    body->position += v_half * dt;
    body->rotation += omega_half * dt;

    // Recalculate forces at new position
    netForceTorque = object->getNetForce();
    body->netForce = std::get<0>(netForceTorque).force;
    body->netTorque = std::get<1>(netForceTorque);
    body->acceleration = body->netForce * body->invMass;
    body->angularAcceleration = body->netTorque * body->invMomentOfInertia;

    // Complete velocity update with new acceleration
    body->velocity = v_half + body->acceleration * (dt * 0.5f);
    body->angularVelocity = omega_half + body->angularAcceleration * (dt * 0.5f);
}
Body VerletSolver::simulate(float dt)
{
    Body tempBody = *object->body;

    // Get current acceleration
    std::tuple<Force, float> netForceTorque = object->getNetForce();
    tempBody.netForce = std::get<0>(netForceTorque).force;
    tempBody.netTorque = std::get<1>(netForceTorque);
    Vec2 a_n = tempBody.netForce * tempBody.invMass;
    float alpha_n = tempBody.netTorque * tempBody.invMomentOfInertia;

    // Half-step velocity update
    Vec2 v_half = tempBody.velocity + a_n * (dt * 0.5f);
    float omega_half = tempBody.angularVelocity + alpha_n * (dt * 0.5f);

    // Full-step position update using half-step velocity
    tempBody.position += v_half * dt;
    tempBody.rotation += omega_half * dt;

    // Recalculate forces at new position
    Vec2 newAcceleration = Vec2(0.0f, 0.0f);
    float newAngularAcceleration = 0.0f;
    for (auto forceSource : object->getForces())
    {
        std::tuple<Force, float> ft = forceSource->calculateForce(tempBody);
        newAcceleration += std::get<0>(ft).force * tempBody.invMass;
        newAngularAcceleration += std::get<1>(ft) * tempBody.invMomentOfInertia;
    }
    tempBody.netForce = newAcceleration * tempBody.mass;
    tempBody.netTorque = newAngularAcceleration * tempBody.momentOfInertia;
    tempBody.acceleration = newAcceleration;
    tempBody.angularAcceleration = newAngularAcceleration;

    // Complete velocity update with new acceleration
    tempBody.velocity = v_half + newAcceleration * (dt * 0.5f);
    tempBody.angularVelocity = omega_half + newAngularAcceleration * (dt * 0.5f);

    return tempBody;
}

// Adams-Bashforth Solver Implementation
void ABSolver::step(float dt)
{
    Body *body = object->body;

    // Compute current forces and accelerations
    std::tuple<Force, float> netForceTorque = object->getNetForce();
    Vec2 force = std::get<0>(netForceTorque).force;
    float torque = std::get<1>(netForceTorque);

    body->netForce = force;
    body->netTorque = torque;
    body->acceleration = force * body->invMass;
    body->angularAcceleration = torque * body->invMomentOfInertia;

    // If we don't have enough history, use RK4 to bootstrap
    if (previousStates.size() < 3)
    {
        // Store current derivative (acceleration) before RK4 update
        Body currentDerivative;
        currentDerivative.velocity = body->velocity;
        currentDerivative.acceleration = body->acceleration;
        currentDerivative.angularVelocity = body->angularVelocity;
        currentDerivative.angularAcceleration = body->angularAcceleration;
        previousStates.push_back(currentDerivative);

        // Use RK4 for initial steps
        RK4Solver rk4Solver(object);
        rk4Solver.step(dt);
    }
    else
    {
        // Have enough history for AB4
        // AB4 formula: y_{n+1} = y_n + dt/24 * (55*f_n - 59*f_{n-1} + 37*f_{n-2} - 9*f_{n-3})
        
        // Current derivatives
        Vec2 v_n = body->velocity;
        Vec2 a_n = body->acceleration;
        float omega_n = body->angularVelocity;
        float alpha_n = body->angularAcceleration;

        // Previous derivatives from history
        Vec2 v_n1 = previousStates[2].velocity;
        Vec2 a_n1 = previousStates[2].acceleration;
        float omega_n1 = previousStates[2].angularVelocity;
        float alpha_n1 = previousStates[2].angularAcceleration;

        Vec2 v_n2 = previousStates[1].velocity;
        Vec2 a_n2 = previousStates[1].acceleration;
        float omega_n2 = previousStates[1].angularVelocity;
        float alpha_n2 = previousStates[1].angularAcceleration;

        Vec2 v_n3 = previousStates[0].velocity;
        Vec2 a_n3 = previousStates[0].acceleration;
        float omega_n3 = previousStates[0].angularVelocity;
        float alpha_n3 = previousStates[0].angularAcceleration;

        // Store current derivative before update
        Body currentDerivative;
        currentDerivative.velocity = v_n;
        currentDerivative.acceleration = a_n;
        currentDerivative.angularVelocity = omega_n;
        currentDerivative.angularAcceleration = alpha_n;

        // Update using AB4
        body->position += (v_n * 55.0f - v_n1 * 59.0f + v_n2 * 37.0f - v_n3 * 9.0f) * (dt / 24.0f);
        body->velocity += (a_n * 55.0f - a_n1 * 59.0f + a_n2 * 37.0f - a_n3 * 9.0f) * (dt / 24.0f);
        
        body->rotation += (omega_n * 55.0f - omega_n1 * 59.0f + omega_n2 * 37.0f - omega_n3 * 9.0f) * (dt / 24.0f);
        body->angularVelocity += (alpha_n * 55.0f - alpha_n1 * 59.0f + alpha_n2 * 37.0f - alpha_n3 * 9.0f) * (dt / 24.0f);

        // Shift history: remove oldest, add current derivative
        previousStates.erase(previousStates.begin());
        previousStates.push_back(currentDerivative);
    }
}
Body ABSolver::simulate(float dt)
{
    Body tempBody = *object->body;

    // Compute current forces and accelerations
    std::tuple<Force, float> netForceTorque = object->getNetForce();
    Vec2 force = std::get<0>(netForceTorque).force;
    float torque = std::get<1>(netForceTorque);

    tempBody.netForce = force;
    tempBody.netTorque = torque;
    tempBody.acceleration = force * tempBody.invMass;
    tempBody.angularAcceleration = torque * tempBody.invMomentOfInertia;

    // If we don't have enough history, use RK4 to simulate
    if (previousStates.size() < 3)
    {
        // Use RK4 for initial steps
        RK4Solver rk4Solver(object);
        return rk4Solver.simulate(dt);
    }
    else
    {
        // Have enough history for AB4
        // AB4 formula: y_{n+1} = y_n + dt/24 * (55*f_n - 59*f_{n-1} + 37*f_{n-2} - 9*f_{n-3})
        
        // Current derivatives
        Vec2 v_n = tempBody.velocity;
        Vec2 a_n = tempBody.acceleration;
        float omega_n = tempBody.angularVelocity;
        float alpha_n = tempBody.angularAcceleration;

        // Previous derivatives from history
        Vec2 v_n1 = previousStates[2].velocity;
        Vec2 a_n1 = previousStates[2].acceleration;
        float omega_n1 = previousStates[2].angularVelocity;
        float alpha_n1 = previousStates[2].angularAcceleration;

        Vec2 v_n2 = previousStates[1].velocity;
        Vec2 a_n2 = previousStates[1].acceleration;
        float omega_n2 = previousStates[1].angularVelocity;
        float alpha_n2 = previousStates[1].angularAcceleration;

        Vec2 v_n3 = previousStates[0].velocity;
        Vec2 a_n3 = previousStates[0].acceleration;
        float omega_n3 = previousStates[0].angularVelocity;
        float alpha_n3 = previousStates[0].angularAcceleration;

        // Update using AB4 (don't modify previousStates in simulate)
        tempBody.position += (v_n * 55.0f - v_n1 * 59.0f + v_n2 * 37.0f - v_n3 * 9.0f) * (dt / 24.0f);
        tempBody.velocity += (a_n * 55.0f - a_n1 * 59.0f + a_n2 * 37.0f - a_n3 * 9.0f) * (dt / 24.0f);
        
        tempBody.rotation += (omega_n * 55.0f - omega_n1 * 59.0f + omega_n2 * 37.0f - omega_n3 * 9.0f) * (dt / 24.0f);
        tempBody.angularVelocity += (alpha_n * 55.0f - alpha_n1 * 59.0f + alpha_n2 * 37.0f - alpha_n3 * 9.0f) * (dt / 24.0f);

        return tempBody;
    }
}

// Adams-Bashforth-Moulton Solver Implementation (Predictor-Corrector)
void ABMSolver::step(float dt)
{
    Body *body = object->body;

    // Compute current forces and accelerations
    std::tuple<Force, float> netForceTorque = object->getNetForce();
    Vec2 force = std::get<0>(netForceTorque).force;
    float torque = std::get<1>(netForceTorque);

    body->netForce = force;
    body->netTorque = torque;
    body->acceleration = force * body->invMass;
    body->angularAcceleration = torque * body->invMomentOfInertia;

    // If we don't have enough history, use RK4 to bootstrap
    if (previousStates.size() < 3)
    {
        // Store current derivative before RK4 update
        Body currentDerivative;
        currentDerivative.velocity = body->velocity;
        currentDerivative.acceleration = body->acceleration;
        currentDerivative.angularVelocity = body->angularVelocity;
        currentDerivative.angularAcceleration = body->angularAcceleration;
        previousStates.push_back(currentDerivative);

        // Use RK4 for initial steps
        RK4Solver rk4Solver(object);
        rk4Solver.step(dt);
    }
    else
    {
        // PREDICTOR: Use AB4 to predict next state
        ABSolver abPredictor(object);
        abPredictor.setPreviousStates(this->previousStates);  // Share history
        Body predictedBody = abPredictor.simulate(dt);
        
        // Current derivatives
        Vec2 v_n = body->velocity;
        Vec2 a_n = body->acceleration;
        float omega_n = body->angularVelocity;
        float alpha_n = body->angularAcceleration;

        // Previous derivatives from history
        Vec2 v_n1 = previousStates[2].velocity;
        Vec2 a_n1 = previousStates[2].acceleration;
        float omega_n1 = previousStates[2].angularVelocity;
        float alpha_n1 = previousStates[2].angularAcceleration;

        Vec2 v_n2 = previousStates[1].velocity;
        Vec2 a_n2 = previousStates[1].acceleration;
        float omega_n2 = previousStates[1].angularVelocity;
        float alpha_n2 = previousStates[1].angularAcceleration;

        // Store current derivative before update
        Body currentDerivative;
        currentDerivative.velocity = v_n;
        currentDerivative.acceleration = a_n;
        currentDerivative.angularVelocity = omega_n;
        currentDerivative.angularAcceleration = alpha_n;

        // Evaluate forces at predicted state to get f_{n+1}
        Vec2 predictedAcceleration = Vec2(0.0f, 0.0f);
        float predictedAngularAcceleration = 0.0f;
        for (auto forceSource : object->getForces())
        {
            std::tuple<Force, float> ft = forceSource->calculateForce(predictedBody);
            predictedAcceleration += std::get<0>(ft).force * body->invMass;
            predictedAngularAcceleration += std::get<1>(ft) * body->invMomentOfInertia;
        }

        // CORRECTOR: Use AM4 to correct the prediction
        // AM4 formula: y_{n+1} = y_n + dt/24 * (9*f_{n+1} + 19*f_n - 5*f_{n-1} + f_{n-2})
        body->position += (predictedBody.velocity * 9.0f + v_n * 19.0f - v_n1 * 5.0f + v_n2 * 1.0f) * (dt / 24.0f);
        body->velocity += (predictedAcceleration * 9.0f + a_n * 19.0f - a_n1 * 5.0f + a_n2 * 1.0f) * (dt / 24.0f);
        body->acceleration = predictedAcceleration;
        
        body->rotation += (predictedBody.angularVelocity * 9.0f + omega_n * 19.0f - omega_n1 * 5.0f + omega_n2 * 1.0f) * (dt / 24.0f);
        body->angularVelocity += (predictedAngularAcceleration * 9.0f + alpha_n * 19.0f - alpha_n1 * 5.0f + alpha_n2 * 1.0f) * (dt / 24.0f);
        body->angularAcceleration = predictedAngularAcceleration;

        // Shift history: remove oldest, add current derivative
        previousStates.erase(previousStates.begin());
        previousStates.push_back(currentDerivative);
    }
}
Body ABMSolver::simulate(float dt)
{
    Body tempBody = *object->body;

    return tempBody;
}

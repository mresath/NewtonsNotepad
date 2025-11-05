#include "ODE.hpp"

void EulerSolver::step(float dt)
{
    Body *body = object->body;

    body->netForce = object->getNetForce().force;
    body->acceleration = body->netForce * body->invMass;
    body->velocity += body->acceleration * dt;
    body->position += body->velocity * dt;
}
Body EulerSolver::simulate(float dt)
{
    Body tempBody = *object->body;

    tempBody.netForce = object->getNetForce().force;
    tempBody.acceleration = tempBody.netForce * tempBody.invMass;
    tempBody.velocity += tempBody.acceleration * dt;
    tempBody.position += tempBody.velocity * dt;
    return tempBody;
}

void RK2Solver::step(float dt)
{
    Body *body = object->body;

    // K1: Evaluate at the current state
    body->netForce = object->getNetForce().force;
    Vec2 k1_acceleration = body->netForce * body->invMass;
    Vec2 k1_velocity = body->velocity;

    // K2: Evaluate at the midpoint using K1
    Body midBody = *body;
    midBody.position = body->position + k1_velocity * (dt * 0.5f);
    midBody.velocity = body->velocity + k1_acceleration * (dt * 0.5f);
    midBody.netForce = Vec2(0.0f, 0.0f);

    Vec2 k2_velocity = midBody.velocity;
    Vec2 k2_acceleration = Vec2(0.0f, 0.0f);

    for (auto forceSource : object->getForces())
    {
        k2_acceleration += forceSource->calculateForce(midBody).force * body->invMass;
    }

    // Update using K2 (the midpoint slope)
    body->velocity += k2_acceleration * dt;
    body->position += k2_velocity * dt;
    body->acceleration = k2_acceleration;
}
Body RK2Solver::simulate(float dt)
{
    Body tempBody = *object->body;

    // K1: Evaluate at the current state
    tempBody.netForce = object->getNetForce().force;
    Vec2 k1_acceleration = tempBody.netForce * tempBody.invMass;
    Vec2 k1_velocity = tempBody.velocity;

    // K2: Evaluate at the midpoint using K1
    Body midBody = tempBody;
    midBody.position = tempBody.position + k1_velocity * (dt * 0.5f);
    midBody.velocity = tempBody.velocity + k1_acceleration * (dt * 0.5f);
    midBody.netForce = Vec2(0.0f, 0.0f);

    Vec2 k2_velocity = midBody.velocity;
    Vec2 k2_acceleration = Vec2(0.0f, 0.0f);

    for (auto forceSource : object->getForces())
    {
        k2_acceleration += forceSource->calculateForce(midBody).force * tempBody.invMass;
    }

    // Update using K2 (the midpoint slope)
    tempBody.velocity += k2_acceleration * dt;
    tempBody.position += k2_velocity * dt;
    tempBody.acceleration = k2_acceleration;

    return tempBody;
}

void RK4Solver::step(float dt)
{
    Body *body = object->body;

    // K1: Evaluate at the current state
    body->netForce = object->getNetForce().force;
    Vec2 k1_acceleration = body->netForce * body->invMass;
    Vec2 k1_velocity = body->velocity;

    // K2: Evaluate at the midpoint using K1
    Body k2Body = *body;
    k2Body.position = body->position + k1_velocity * (dt * 0.5f);
    k2Body.velocity = body->velocity + k1_acceleration * (dt * 0.5f);
    k2Body.netForce = Vec2(0.0f, 0.0f);

    Vec2 k2_velocity = k2Body.velocity;
    Vec2 k2_acceleration = Vec2(0.0f, 0.0f);

    for (auto forceSource : object->getForces())
    {
        k2_acceleration += forceSource->calculateForce(k2Body).force * body->invMass;
    }

    // K3: Evaluate at the midpoint using K2
    Body k3Body = *body;
    k3Body.position = body->position + k2_velocity * (dt * 0.5f);
    k3Body.velocity = body->velocity + k2_acceleration * (dt * 0.5f);
    k3Body.netForce = Vec2(0.0f, 0.0f);

    Vec2 k3_velocity = k3Body.velocity;
    Vec2 k3_acceleration = Vec2(0.0f, 0.0f);

    for (auto forceSource : object->getForces())
    {
        k3_acceleration += forceSource->calculateForce(k3Body).force * body->invMass;
    }

    // K4: Evaluate at the endpoint using K3
    Body k4Body = *body;
    k4Body.position = body->position + k3_velocity * dt;
    k4Body.velocity = body->velocity + k3_acceleration * dt;
    k4Body.netForce = Vec2(0.0f, 0.0f);

    Vec2 k4_velocity = k4Body.velocity;
    Vec2 k4_acceleration = Vec2(0.0f, 0.0f);

    for (auto forceSource : object->getForces())
    {
        k4_acceleration += forceSource->calculateForce(k4Body).force * body->invMass;
    }

    // Weighted average: (k1 + 2*k2 + 2*k3 + k4) / 6
    body->velocity += (k1_acceleration + k2_acceleration * 2.0f + k3_acceleration * 2.0f + k4_acceleration) * (dt / 6.0f);
    body->position += (k1_velocity + k2_velocity * 2.0f + k3_velocity * 2.0f + k4_velocity) * (dt / 6.0f);
    body->acceleration = (k1_acceleration + k2_acceleration * 2.0f + k3_acceleration * 2.0f + k4_acceleration) / 6.0f;
}
Body RK4Solver::simulate(float dt) {
     Body tempBody = *object->body;

        // K1: Evaluate at the current state
        tempBody.netForce = object->getNetForce().force;
        Vec2 k1_acceleration = tempBody.netForce * tempBody.invMass;
        Vec2 k1_velocity = tempBody.velocity;

        // K2: Evaluate at the midpoint using K1
        Body k2Body = tempBody;
        k2Body.position = tempBody.position + k1_velocity * (dt * 0.5f);
        k2Body.velocity = tempBody.velocity + k1_acceleration * (dt * 0.5f);
        k2Body.netForce = Vec2(0.0f, 0.0f);
        
        Vec2 k2_velocity = k2Body.velocity;
        Vec2 k2_acceleration = Vec2(0.0f, 0.0f);
        
        for (auto forceSource : object->getForces())
        {
            k2_acceleration += forceSource->calculateForce(k2Body).force * tempBody.invMass;
        }

        // K3: Evaluate at the midpoint using K2
        Body k3Body = tempBody;
        k3Body.position = tempBody.position + k2_velocity * (dt * 0.5f);
        k3Body.velocity = tempBody.velocity + k2_acceleration * (dt * 0.5f);
        k3Body.netForce = Vec2(0.0f, 0.0f);
        
        Vec2 k3_velocity = k3Body.velocity;
        Vec2 k3_acceleration = Vec2(0.0f, 0.0f);
        
        for (auto forceSource : object->getForces())
        {
            k3_acceleration += forceSource->calculateForce(k3Body).force * tempBody.invMass;
        }

        // K4: Evaluate at the endpoint using K3
        Body k4Body = tempBody;
        k4Body.position = tempBody.position + k3_velocity * dt;
        k4Body.velocity = tempBody.velocity + k3_acceleration * dt;
        k4Body.netForce = Vec2(0.0f, 0.0f);
        
        Vec2 k4_velocity = k4Body.velocity;
        Vec2 k4_acceleration = Vec2(0.0f, 0.0f);
        
        for (auto forceSource : object->getForces())
        {
            k4_acceleration += forceSource->calculateForce(k4Body).force * tempBody.invMass;
        }

        // Weighted average: (k1 + 2*k2 + 2*k3 + k4) / 6
        tempBody.velocity += (k1_acceleration + k2_acceleration * 2.0f + k3_acceleration * 2.0f + k4_acceleration) * (dt / 6.0f);
        tempBody.position += (k1_velocity + k2_velocity * 2.0f + k3_velocity * 2.0f + k4_velocity) * (dt / 6.0f);
        tempBody.acceleration = (k1_acceleration + k2_acceleration * 2.0f + k3_acceleration * 2.0f + k4_acceleration) / 6.0f;
        
        return tempBody;
}
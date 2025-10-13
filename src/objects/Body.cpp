#include "Body.hpp"
#include "math/Util.hpp"

// Constructors
Body::Body(const Vec2 &position, float mass)
{
    this->position = position;
    this->mass = mass;

    this->invMass = (mass == 0.0f) ? 0.0f : 1.0f / mass;
}
Body::Body(const Vec2 &position, float mass, float dragCoefficient, float area)
{
    this->area = area;
    this->dragCoefficient = dragCoefficient;
}

// Methods
void Body::applyForce(const Vec2 &force)
{
    netForce += force;
}

void Body::update(float dt)
{
    acceleration = metersToPixels(netForce) * invMass;
    velocity += acceleration * dt;
    position += velocity * dt;

    netForce = Vec2(0.0f, 0.0f);
}

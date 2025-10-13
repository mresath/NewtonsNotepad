#include "World.hpp"

World::World() : gravity(0.0f, 9.81f) {}
World::World(const Vec2 &gravity)
{
    this->gravity = gravity;
}
World::~World()
{
    for (Body *body : bodies)
    {
        delete body;
    }
}

void World::addBody(Body *body)
{
    bodies.push_back(body);
}

void World::removeBody(size_t index)
{
    if (index < bodies.size())
    {
        bodies.erase(bodies.begin() + index);
    }
}

void World::update(float dt)
{
    for (Body *body : bodies)
    {
        body->applyForce(gravity * body->mass);

        body->update(dt);
    }
}

const std::vector<Body *> &World::getBodies() const
{
    return bodies;
}

void World::setGravity(const Vec2 &newGravity)
{
    gravity = newGravity;
}

Vec2 World::getGravity() const
{
    return gravity;
}
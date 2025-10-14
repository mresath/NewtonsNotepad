#include "World.hpp"

World::World() : gravity(0.0f, 9.81f) {}
World::World(const Vec2 &gravity)
{
    this->gravity = gravity;
}
World::~World()
{
    for (Object *object : objects)
    {
        delete object;
    }
}

void World::addObject(Object *object)
{
    objects.push_back(object);
}

void World::removeObject(size_t index)
{
    if (index < objects.size())
    {
        objects.erase(objects.begin() + index);
    }
}

void World::update(float dt)
{
    for (Object *object : objects)
    {
        Body *body = object->body;

        body->applyForce(gravity * body->mass);
        
        object->update(dt);
    }
}

void World::draw(sf::RenderWindow *window)
{
    for (Object *object : objects)
    {
        object->draw(window);
    }
}

const std::vector<Object *> &World::getObjects() const
{
    return objects;
}

void World::setGravity(const Vec2 &newGravity)
{
    gravity = newGravity;
}

Vec2 World::getGravity() const
{
    return gravity;
}
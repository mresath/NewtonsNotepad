#include "World.hpp"
#include "engine/Collision.hpp"

World::World() : gravity(DEFAULT_GRAVITY) {}
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
    // Apply global forces
    for (Object *object : objects)
    {
        Body *body = object->body;

        if (object->doGravity)
        {
            object->applyForce(gravity * body->mass);
        }
    }

    // Collision detection and forces
    for (size_t i = 0; i < objects.size(); i++)
    {
        for (size_t j = i + 1; j < objects.size(); j++)
        {
            Object *objA = objects[i];
            Object *objB = objects[j];

            if (objA->isStatic && objB->isStatic)
                continue;

            CollisionInfo info = checkCollision(objA, objB);

            if (info.isColliding)
            {
                resolveCollision(objA, objB, info, dt);
            }
        }
    }

    // Update all objects
    for (Object *object : objects)
    {
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

void World::setODESolver(SolverType type)
{
    odeSolver = type;
    for (Object *object : objects)
    {
        object->switchSolver(type);
    }
}
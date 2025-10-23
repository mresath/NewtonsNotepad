#include "World.hpp"
#include "engine/Collision.hpp"

World::World() : gravity(DEFAULT_GRAVITY), airDensity(DEFAULT_AIR_DENSITY) {}
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

        if (object->doDrag && body->dragCoefficient != 0.0f)
        {
            Vec2 dragForce = body->velocity * -1.0f;
            float speedSq = body->velocity.lengthSquared();
            if (speedSq > 0.0f)
            {
                dragForce = dragForce.normalized();
                float dragMagnitude = 0.5f * airDensity * speedSq * object->dimensions.x * body->dragCoefficient;
                dragForce *= dragMagnitude;
                object->applyForce(dragForce);
            }
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

void World::setODESolver(SolverType type)
{
    odeSolver = type;
    for (Object *object : objects)
    {
        object->switchSolver(type);
    }
}

SolverType World::getODESolver() const
{
    return odeSolver;
}
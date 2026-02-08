#include "World.hpp"
#include "engine/Collision.hpp"

World::World() : gravity(DEFAULT_GRAVITY), airDensity(DEFAULT_AIR_DENSITY) {}
World::~World()
{
    for (Connector *connector : connectors)
    {
        delete connector;
    }

    for (Object *object : objects)
    {
        delete object;
    }
}

void World::addObject(Object *object)
{
    object->setID(nextObjectID++);
    object->setGravityPointer(&gravity);
    object->switchSolver(odeSolver);
    objects.push_back(object);
}

void World::removeObject(size_t index)
{
    if (index < objects.size())
    {
        Object *objToRemove = objects[index];

        objToRemove->clearForces();

        objects.erase(objects.begin() + index);

        for (Connector *connector : connectors)
        {
            if (connector->isConnectedTo(objToRemove))
            {
                connectors.erase(std::remove(connectors.begin(), connectors.end(), connector), connectors.end());
                delete connector;
            }
        }
        delete objToRemove;
    }
}

void World::clearObjects()
{
    for (Object *object : objects)
    {
        delete object;
    }
    objects.clear();
    nextObjectID = 0;
}

void World::addConnector(Connector *connector)
{
    connector->setID(nextConnectorID++);
    connector->connectForces();
    connectors.push_back(connector);
}

void World::removeConnector(size_t index)
{
    if (index < connectors.size())
    {
        delete connectors[index];
        connectors.erase(connectors.begin() + index);
    }
}

void World::clearConnectors()
{
    for (Connector *connector : connectors)
    {
        delete connector;
    }
    connectors.clear();
}

void World::update(float dt)
{
    // Update simulation time
    time += dt;

    // Apply global forces
    for (Object *object : objects)
    {
        Body *body = object->body;

        if (object->doGravity)
        {
            Vec2 *gravityPointer = &gravity;
            ForceSource gravitySource("gravity", [gravityPointer](Body state)
                                      {
                Vec2 gForce = *gravityPointer * state.mass;
                return Force(Vec2(0, 0), gForce); });
            object->applyForce(gravitySource);
        }
        else
        {
            object->deleteForce("gravity");
        }

        if (object->doDrag && body->dragCoefficient != 0.0f)
        {
            float airDensity = this->airDensity;
            float area = M_PI * object->dimensions.x;

            ForceSource dragSource("drag", [airDensity, area](Body state)
                                   {
                Vec2 df = state.velocity * -1.0f;
                float speedSq = state.velocity.lengthSquared();
                if (speedSq > 0.0f)
                {
                    df = df.normalized();
                    float dragMagnitude = 0.5f * airDensity * speedSq * area * state.dragCoefficient;
                    df *= dragMagnitude;
                }
                return Force(Vec2(0, 0), df); });
            object->applyForce(dragSource);
        }
        else
        {
            object->deleteForce("drag");
        }

        if (object->doMagnus && body->liftCoefficient != 0.0f)
        {
            float airDensity = this->airDensity;
            float area = M_PI * object->dimensions.x;
            ForceSource magnusSource("magnus", [airDensity, area](Body state)
                                     {
                Vec2 liftDir = state.velocity.perpendicular().normalized();
                float speedSq = state.velocity.lengthSquared();
                float magnusMagnitude = 0.5f * airDensity * speedSq * area * state.liftCoefficient * state.angularVelocity;
                Vec2 liftForce = liftDir * magnusMagnitude;
                return Force(Vec2(0, 0), liftForce); });
            object->applyForce(magnusSource);
        }
        else
        {
            object->deleteForce("magnus");
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
    float energySum = 0.0f;
    for (Object *object : objects)
    {
        object->update(dt);
    }
    for (Connector *connector : connectors)
    {
        connector->applyEnergy();
    }
    for (Object *object : objects)
    {
        object->totalEnergy();
        energySum += object->body->totalEnergy;
    }
    totalEnergy = energySum;
}

void World::draw(sf::RenderWindow *window)
{
    draw(window, false);
}

void World::draw(sf::RenderWindow *window, bool showAttachmentPoints)
{
    for (Object *object : objects)
    {
        object->draw(window, showAttachmentPoints);
    }

    for (Connector *connector : connectors)
    {
        connector->draw(window);
    }
}

const std::vector<Object *> &World::getObjects() const
{
    return objects;
}

const std::vector<Connector *> &World::getConnectors() const
{
    return connectors;
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

float World::getTime() const
{
    return time;
}

void World::initialize()
{
    Object *ground = new Object(*pixelsToMeters(new Vec2(0, DEF_HEIGHT - HALF_WALL_THICKNESS)), *pixelsToMeters(new Vec2(WORLD_WIDTH, WALL_THICKNESS)), 1.0f, RECTANGLE);
    Object *leftWall = new Object(*pixelsToMeters(new Vec2(-(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS), (DEF_HEIGHT - HALF_WALL_THICKNESS) - (WORLD_HEIGHT / 2))), *pixelsToMeters(new Vec2(WALL_THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *rightWall = new Object(*pixelsToMeters(new Vec2(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS, (DEF_HEIGHT - HALF_WALL_THICKNESS) - (WORLD_HEIGHT / 2))), *pixelsToMeters(new Vec2(WALL_THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *ceiling = new Object(*pixelsToMeters(new Vec2(0, (DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT)), *pixelsToMeters(new Vec2(WORLD_WIDTH, WALL_THICKNESS)), 1.0f, RECTANGLE);
    ground->setConstant();
    leftWall->setConstant();
    rightWall->setConstant();
    ceiling->setConstant();
    ground->shape->setFillColor(WALL_COLOR);
    leftWall->shape->setFillColor(WALL_COLOR);
    rightWall->shape->setFillColor(WALL_COLOR);
    ceiling->shape->setFillColor(WALL_COLOR);
    addObject(ground);
    addObject(leftWall);
    addObject(rightWall);
    addObject(ceiling);
}

void World::clear()
{
    clearObjects();
    clearConnectors();
    time = 0.0f;
}

void World::reset()
{
    clear();
    initialize();
}

Object *World::loadTestScene()
{
    reset();

    Vec2 amplitude = Vec2(0.0f, 1.0f);
    Vec2 origin = Vec2(0.0f, 0.0f);
    Vec2 length = Vec2(0.0f, 3.0f);

    Object *newCircle = new Object(origin + amplitude, Vec2(0.25f, 0.25f), 1.0f, CIRCLE);

    newCircle->body->dragCoefficient = 0.0f;
    newCircle->body->liftCoefficient = 0.0f;
    newCircle->body->frictionCoefficient = 0.0f;
    newCircle->body->restitution = 0.7f;

    addObject(newCircle);

    Spring *newSpring = new Spring(newCircle, Vec2(0.0f, 0.0f), nullptr, origin - length, 50.0f, 0.0f, length.y);

    addConnector(newSpring);

    return newCircle;
}
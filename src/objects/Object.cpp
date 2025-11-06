#include "Object.hpp"

Object::Object(Vec2 position, Vec2 dimensions, float density, ShapeType type)
{
    this->shapeType = type;
    this->dimensions = dimensions;

    Vec2 *len = metersToPixels(&dimensions);
    if (type == CIRCLE)
    {
        shape = new sf::CircleShape(len->x);
        shape->setOrigin(sf::Vector2f(len->x, len->x));
        this->volume = M_PI * dimensions.x * dimensions.x;
    }
    else if (type == RECTANGLE)
    {
        shape = new sf::RectangleShape(sf::Vector2f(len->x, len->y));
        shape->setOrigin(sf::Vector2f(len->x / 2, len->y / 2));
        this->volume = dimensions.x * dimensions.y;
    }
    delete len;

    shape->setFillColor(sf::Color::White);

    body = new Body(position, density * volume);

    switch (DEFAULT_SOLVER)
    {
    case EULER:
        solver = new EulerSolver(this);
        break;
    case RK2:
        solver = new RK2Solver(this);
        break;
    case RK4:
        solver = new RK4Solver(this);
        break;
    }
}

Object::~Object()
{
    delete shape;
    delete body;
    delete solver;
    for (auto &source : forceSources)
    {
        delete source;
    }
    forceSources.clear();
}

void Object::setStatic(bool isStatic)
{
    this->isStatic = isStatic;
    doGravity = !isStatic;
    if (isStatic)
    {
        doDrag = false;
        doFriction = false;
        canApplyFriction = true;
    }
}

void Object::setConstant()
{
    isSelectable = false;
    setStatic(true);
}

void Object::applyForce(const ForceSource &force)
{
    deleteForce(force.name);
    forceSources.push_back(new ForceSource(force));
}

void Object::deleteForce(const std::string &name)
{
    for (auto it = forceSources.begin(); it != forceSources.end(); ++it)
    {
        if ((*it)->name == name)
        {
            delete *it;
            forceSources.erase(it);
            break;
        }
    }
}

const std::vector<ForceSource *> &Object::getForces() const
{
    return forceSources;
}

const Force Object::getNetForce() const
{
    Vec2 netForce(0.0f, 0.0f);
    Vec2 netTorquePoint(0.0f, 0.0f);
    for (const auto &source : forceSources)
    {
        Force f = source->calculateForce(*body);
        netForce += f.force;
        netTorquePoint += f.position * f.force.length();
    }
    return Force(netTorquePoint, netForce);
}

void Object::switchSolver(SolverType type)
{
    ODESolver *newSolver = nullptr;
    switch (type)
    {
    case EULER:
        newSolver = new EulerSolver(this);
        break;
    case RK2:
        newSolver = new RK2Solver(this);
        break;
    case RK4:
        newSolver = new RK4Solver(this);
        ;
        break;
    }
    if (newSolver)
    {
        if (solver)
            delete solver;
        solver = newSolver;
    }
}

void Object::calculateEnergies()
{
    body->kineticEnergy = 0.5f * body->mass * body->velocity.lengthSquared();
    body->gravitationalPotential = dot(standardizePosition(body->position), *gravityPtr) * body->mass;
}

void Object::update(float dt)
{
    if (!isStatic)
    {
        solver->step(dt);

        Vec2 *minPixels = new Vec2(-WORLD_WIDTH / 2, DEF_HEIGHT - WORLD_HEIGHT + HALF_WALL_THICKNESS);
        Vec2 *maxPixels = new Vec2(WORLD_WIDTH / 2, DEF_HEIGHT - HALF_WALL_THICKNESS);
        body->position.constrain(*pixelsToMeters(minPixels), *pixelsToMeters(maxPixels));

        body->netForce = Vec2(0.0f, 0.0f);
    }

    calculateEnergies();

    Vec2 *pos = metersToPixels(&body->position);
    shape->setPosition(sf::Vector2f(pos->x, pos->y));
    delete pos;
}

void Object::draw(sf::RenderWindow *window)
{
    window->draw(*shape);
}

int Object::getID() const {
    return id;
}

void Object::setID(int newID) {
    id = newID;
}

void Object::setGravityPointer(Vec2* gravity) {
    gravityPtr = gravity;
}
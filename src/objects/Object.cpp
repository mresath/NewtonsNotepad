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

    float mass = density * volume;
    float inertia = type == CIRCLE ? 0.5f * mass * dimensions.x * dimensions.x
                                 : (1.0f / 12.0f) * mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y);

    body = new Body(position, mass, inertia);

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
    case VERLET:
        // solver = new VerletSolver(this);
        break;
    case DOPRI5:
        // solver = new DOPRI5Solver(this);
        break;
    case AB:
        // solver = new ABSolver(this);
        break;
    case AM:    
        // solver = new AMSolver(this);
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
    body->restitution = 1.0f;
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

const std::tuple<Force, float> Object::getNetForce() const
{
    Vec2 netForce(0.0f, 0.0f);
    float netTorque = 0.0f;
    for (const auto &source : forceSources)
    {
        std::tuple<Force, float> ft = source->calculateForce(*body);

        netForce += std::get<0>(ft).force;
        netTorque += std::get<1>(ft);
    }
    return std::make_tuple(Force(Vec2(0.0f, 0.0f), netForce), netTorque);
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
    case VERLET:
        // newSolver = new VerletSolver(this);
        break;
    case DOPRI5:
        // newSolver = new DOPRI5Solver(this);
        break;
    case AB:
        // newSolver = new ABSolver(this);
        break;
    case AM:    
        // newSolver = new AMSolver(this);
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
    body->momentum = body->velocity * body->mass;
    body->angularMomentum = body->momentOfInertia * body->angularVelocity;

    body->kineticEnergy = 0.5f * body->mass * body->velocity.lengthSquared();
    body->rotationalKineticEnergy = 0.5f * body->momentOfInertia * body->angularVelocity * body->angularVelocity;
    body->gravitationalPotential = dot(standardizePosition(body->position), *gravityPtr) * body->mass;

    body->totalEnergy = body->kineticEnergy + body->rotationalKineticEnergy + body->gravitationalPotential;
}

void Object::update(float dt)
{
    if (!isStatic)
    {
        solver->step(dt);

        Vec2 *minPixels = new Vec2(-WORLD_WIDTH / 2, DEF_HEIGHT - WORLD_HEIGHT + HALF_WALL_THICKNESS);
        Vec2 *maxPixels = new Vec2(WORLD_WIDTH / 2, DEF_HEIGHT - HALF_WALL_THICKNESS);
        body->position.constrain(*pixelsToMeters(minPixels), *pixelsToMeters(maxPixels));

        body->rotation = std::fmod(body->rotation, 2.0f * M_PI);

        body->netForce = Vec2(0.0f, 0.0f);
        body->netTorque = 0.0f;

        delete minPixels;
        delete maxPixels;
    }

    calculateEnergies();

    Vec2 *pos = metersToPixels(&body->position);
    shape->setPosition(sf::Vector2f(pos->x, pos->y));
    shape->setRotation(sf::radians(body->rotation));
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
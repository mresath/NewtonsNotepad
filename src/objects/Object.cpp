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

        sf::Texture *texture = new sf::Texture();
        texture->loadFromFile("assets/ball.png");
        shape->setTexture(texture);

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

    Vec2 posPixels = *metersToPixels(&position);
    shape->setPosition(sf::Vector2f(posPixels.x, posPixels.y));

    float mass = density * volume;
    float inertia = type == CIRCLE ? 0.5f * mass * dimensions.x * dimensions.x
                                   : (1.0f / 12.0f) * mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y);

    body = new Body(position, mass, inertia);

    switch (DEFAULT_SOLVER)
    {
    case EULER:
        solver = new EulerSolver(this);
        break;
    case EULERS:
        solver = new EulerSympSolver(this);
        break;
    case RK2:
        solver = new RK2Solver(this);
        break;
    case RK4:
        solver = new RK4Solver(this);
        break;
    case VERLET:
        solver = new VerletSolver(this);
        break;
    case DOPRI5:
        solver = new DOPRI5Solver(this);
        break;
    case AB:
        solver = new ABSolver(this);
        break;
    case ABM:
        solver = new ABMSolver(this);
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
    body->frictionCoefficient = 0.0f;
    setStatic(true);
}

void Object::applyForce(const ForceSource &force)
{
    deleteForce(force.name);
    forceSources.push_back(new ForceSource(force));
}

void Object::deleteForce(const std::string &name)
{
    if (forceSources.empty())
        return;
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

void Object::clearForces()
{
    for (auto &source : forceSources)
    {
        delete source;
    }
    forceSources.clear();
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
    case EULERS:
        newSolver = new EulerSympSolver(this);
        break;
    case RK2:
        newSolver = new RK2Solver(this);
        break;
    case RK4:
        newSolver = new RK4Solver(this);
        break;
    case VERLET:
        newSolver = new VerletSolver(this);
        break;
    case DOPRI5:
        newSolver = new DOPRI5Solver(this);
        break;
    case AB:
        newSolver = new ABSolver(this);
        break;
    case ABM:
        newSolver = new ABMSolver(this);
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
}

void Object::zeroEnergies()
{
    body->kineticEnergy = 0.0f;
    body->rotationalKineticEnergy = 0.0f;
    body->gravitationalPotential = 0.0f;
    body->springPotential = 0.0f;
    body->totalEnergy = 0.0f;
}

void Object::totalEnergy()
{
    body->totalEnergy = body->kineticEnergy + body->rotationalKineticEnergy + body->gravitationalPotential + body->springPotential;
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

    zeroEnergies();
    calculateEnergies();

    Vec2 *pos = metersToPixels(&body->position);
    shape->setPosition(sf::Vector2f(pos->x, pos->y));
    shape->setRotation(sf::radians(body->rotation));
    delete pos;

    // Update path trace
    updatePathTrace(dt);
}

void Object::updatePathTrace(float dt)
{
    if (!pathTraceEnabled)
        return;

    // Age all existing points
    for (auto &point : pathTrace)
    {
        point.age += dt;
    }

    // Remove points that have exceeded fade time
    while (!pathTrace.empty() && pathTrace.front().age > pathTraceFadeTime)
    {
        pathTrace.pop_front();
    }

    // Record new point if enough time has passed
    timeSinceLastRecord += dt;
    if (timeSinceLastRecord >= pathTraceRecordInterval)
    {
        pathTrace.push_back({body->position, 0.0f});
        timeSinceLastRecord = 0.0f;
    }
}

void Object::drawPathTrace(sf::RenderWindow *window)
{
    if (!pathTraceEnabled || pathTrace.empty())
        return;

    for (const auto &point : pathTrace)
    {
        // Calculate alpha based on age (fade out as point gets older)
        float alpha = 1.0f - (point.age / pathTraceFadeTime);
        alpha = std::max(0.0f, std::min(1.0f, alpha));

        // Create circle for this trace point
        sf::CircleShape tracePoint(PATH_TRACE_POINT_RADIUS);
        tracePoint.setOrigin(sf::Vector2f(PATH_TRACE_POINT_RADIUS, PATH_TRACE_POINT_RADIUS));

        // Interpolate color based on age
        sf::Color startColor = PATH_TRACE_POINT_COLOR;
        sf::Color endColor = PATH_TRACE_FADE_COLOR;
        sf::Color pointColor(
            static_cast<unsigned char>(startColor.r * alpha + endColor.r * (1.0f - alpha)),
            static_cast<unsigned char>(startColor.g * alpha + endColor.g * (1.0f - alpha)),
            static_cast<unsigned char>(startColor.b * alpha + endColor.b * (1.0f - alpha)),
            static_cast<unsigned char>(startColor.a * alpha + endColor.a * (1.0f - alpha))
        );
        tracePoint.setFillColor(pointColor);

        // Convert world position to pixel coordinates
        Vec2 *pixelPos = metersToPixels(&point.position);
        tracePoint.setPosition(sf::Vector2f(pixelPos->x, pixelPos->y));
        delete pixelPos;

        window->draw(tracePoint);
    }
}

void Object::clearPathTrace()
{
    pathTrace.clear();
    timeSinceLastRecord = 0.0f;
}

void Object::draw(sf::RenderWindow *window)
{
    draw(window, false);
}

void Object::draw(sf::RenderWindow *window, bool showAttachmentPoints)
{
    window->draw(*shape);
    if (isSelectable && showAttachmentPoints)
    {
        sf::CircleShape attachmentPointShape(ATTACHMENT_POINT_RADIUS);
        attachmentPointShape.setFillColor(sf::Color::Green);
        attachmentPointShape.setPosition(shape->getPosition());
        attachmentPointShape.setOrigin(sf::Vector2f(ATTACHMENT_POINT_RADIUS, ATTACHMENT_POINT_RADIUS));
        window->draw(attachmentPointShape);
    }
}

int Object::getID() const
{
    return id;
}

void Object::setID(int newID)
{
    id = newID;
}

void Object::setGravityPointer(Vec2 *gravity)
{
    gravityPtr = gravity;
}
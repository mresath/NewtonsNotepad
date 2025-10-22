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
        solver = new EulerSolver(body);
        break;
    case RK4:
        solver = new RK4Solver(body);
        break;
    }
}

Object::~Object()
{
    delete shape;
    delete body;
    delete solver;
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

void Object::applyForce(const Vec2 &force)
{
    if (!isStatic)
        body->applyForce(force);
}

void Object::switchSolver(SolverType type)
{
    ODESolver *newSolver = nullptr;
    if (type == EULER)
    {
        newSolver = new EulerSolver(body);
    }
    else if (type == RK4)
    {
        newSolver = new RK4Solver(body);
    }
    if (solver)
        delete solver;
    solver = newSolver;
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

    Vec2 *pos = metersToPixels(&body->position);
    shape->setPosition(sf::Vector2f(pos->x, pos->y));
    delete pos;
}

void Object::draw(sf::RenderWindow *window)
{
    window->draw(*shape);
}



#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <math.h>
#include "objects/Body.hpp"
#include "math/Util.hpp"
#include "engine/ODE.hpp"

enum ShapeType
{
    CIRCLE,
    RECTANGLE,
};

class Object
{
private:
    ODESolver *solver;

public:
    Body *body;
    sf::Shape *shape;
    ShapeType shapeType;

    Vec2 dimensions;
    float volume;

    bool isSelectable = true;
    bool isStatic = false;
    bool doGravity = true;
    bool doDrag = true;
    bool doFriction = true;
    bool canApplyFriction = true;

    bool isGrabbed = false;

    Object(Vec2 position, Vec2 dimensions, float density, ShapeType type);
    ~Object();

    void setStatic(bool isStatic);
    void setConstant();

    void applyForce(const Vec2 &force);

    void switchSolver(SolverType type);

    void update(float dt);

    void draw(sf::RenderWindow *window);
};
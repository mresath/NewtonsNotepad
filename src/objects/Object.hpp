#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include "objects/Body.hpp"
#include "objects/Force.hpp"
#include "math/Util.hpp"
#include "engine/ODE.hpp"

enum SolverType : unsigned short;
class ODESolver;

enum ShapeType
{
    CIRCLE,
    RECTANGLE,
};

class Object
{
private:
    ODESolver *solver;
    std::vector<ForceSource *> forceSources;
    int id = 0;

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

    void applyForce(const ForceSource &force);

    void deleteForce(const std::string &name);

    const std::vector<ForceSource *> &getForces() const;

    const Force getNetForce() const;

    void switchSolver(SolverType type);

    void update(float dt);

    void draw(sf::RenderWindow *window);

    int getID() const;
    void setID(int newID);
};
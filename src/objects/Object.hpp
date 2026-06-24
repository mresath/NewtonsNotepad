#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include <tuple>
#include <deque>
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

struct PathTracePoint
{
    Vec2 position;      // meters
    float age = 0.0f; // seconds
};

class Object
{
private:
    ODESolver *solver;
    std::vector<ForceSource *> forceSources;
    int id = 0;
    Vec2* gravityPtr;

    std::deque<PathTracePoint> pathTrace;
    float pathTraceRecordInterval = 0.05f;
    float timeSinceLastRecord = 0.0f;

    std::vector<std::tuple<Force, ForceType>> forceArrows;

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

    bool pathTraceEnabled = false;
    float pathTraceFadeTime = DEFAULT_PATH_TRACE_FADE_TIME;

    bool arrowEnabled = false;

    Object(Vec2 position, Vec2 dimensions, float density, ShapeType type);
    ~Object();

    void setStatic(bool isStatic);
    void setConstant();

    void applyForce(const ForceSource &force);

    void deleteForce(const std::string &name);
    void clearForces();

    const std::vector<ForceSource *> &getForces() const;

    const std::tuple<Force, float> getNetForce();

    void switchSolver(SolverType type);

    void calculateEnergies();
    void zeroEnergies();
    void totalEnergy();
    void update(float dt);

    void updatePathTrace(float dt);
    void drawPathTrace(sf::RenderWindow *window);
    void clearPathTrace();

    void addForceArrow(const Force &force, ForceType type);
    void clearForceArrows();
    void drawForceArrows(sf::RenderWindow *window);

    void draw(sf::RenderWindow *window);
    void draw(sf::RenderWindow *window, bool showAttachmentPoints);

    int getID() const;
    void setID(int newID);

    void setGravityPointer(Vec2* gravity);
};
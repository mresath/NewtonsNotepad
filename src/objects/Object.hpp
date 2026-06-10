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

// Structure to store a point in the path trace with its age
struct PathTracePoint
{
    Vec2 position;      // World position in meters
    float age = 0.0f;   // Time since this point was recorded (in seconds)
};

class Object
{
private:
    ODESolver *solver;
    std::vector<ForceSource *> forceSources;
    int id = 0;
    Vec2* gravityPtr;

    // Path trace storage
    std::deque<PathTracePoint> pathTrace;
    float pathTraceRecordInterval = 0.05f;  // Record every 0.05 seconds
    float timeSinceLastRecord = 0.0f;

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

    // Path trace properties
    bool pathTraceEnabled = false;
    float pathTraceFadeTime = DEFAULT_PATH_TRACE_FADE_TIME;

    Object(Vec2 position, Vec2 dimensions, float density, ShapeType type);
    ~Object();

    void setStatic(bool isStatic);
    void setConstant();

    void applyForce(const ForceSource &force);

    void deleteForce(const std::string &name);
    void clearForces();

    const std::vector<ForceSource *> &getForces() const;

    const std::tuple<Force, float> getNetForce() const;

    void switchSolver(SolverType type);

    void calculateEnergies();
    void zeroEnergies();
    void totalEnergy();
    void update(float dt);

    // Path trace methods
    void updatePathTrace(float dt);
    void drawPathTrace(sf::RenderWindow *window);
    void clearPathTrace();

    void draw(sf::RenderWindow *window);
    void draw(sf::RenderWindow *window, bool showAttachmentPoints);

    int getID() const;
    void setID(int newID);

    void setGravityPointer(Vec2* gravity);
};
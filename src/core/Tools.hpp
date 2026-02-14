#pragma once

#include "Config.hpp"

#include "objects/Tool.hpp"

struct ToolSettings
{
};

struct PushSettings : ToolSettings
{
    float forceMagnitude = DEFAULT_FORCE;
};

struct PullSettings : ToolSettings
{
    float forceMagnitude = DEFAULT_FORCE;
};

struct CircleSettings : ToolSettings
{
    bool isStatic = false;
    float radius = DEFAULT_LENGTH;
    float density = DEFAULT_DENSITY;
    float dragCoefficient = DEFAULT_DRAG;
    float frictionCoefficient = DEFAULT_FRICTION;
    float restitution = DEFAULT_RESTITUTION;
};

struct RopeSettings : ToolSettings
{
    float totalLength = MAX_PERCENTAGE;
};

struct SpringSettings : ToolSettings
{
    float stiffness = DEFAULT_STIFFNESS;
    float damping = DEFAULT_DAMPING;
    float restingLength = MAX_PERCENTAGE;
};

class Tools
{
private:
    std::vector<Tool *> tools;
    Tool *currentTool;

public:
    ToolSettings *settings;

    Tools();
    ~Tools();

    void addTool(Tool *tool);
    void removeTool(size_t index);

    void setCurrentTool(Tool *tool);
    Tool *getCurrentTool() const;

    void draw();

    const std::vector<Tool *> &getTools() const;
};
#pragma once

#include "Config.h"

#include "objects/Tool.hpp"

struct ToolSettings {};

struct PushSettings : ToolSettings {
    float forceMagnitude = DEFAULT_FORCE;
};

struct PullSettings : ToolSettings {
    float forceMagnitude = DEFAULT_FORCE;
};

struct CircleSettings : ToolSettings {
    bool isStatic = false;
    float radius = DEFAULT_LENGTH;
    float density = DEFAULT_DENSITY;
    float dragCoefficient = DEFAULT_DRAG;
    float liftCoefficient = DEFAULT_LIFT;
    float frictionCoefficient = DEFAULT_FRICTION;
    float restitution = DEFAULT_RESTITUTION;
};

struct RopeSettings : ToolSettings {};

struct SpringSettings : ToolSettings {
    float springConstant = DEFAULT_SPRING_CONSTANT;
    float restLength = DEFAULT_LENGTH;
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
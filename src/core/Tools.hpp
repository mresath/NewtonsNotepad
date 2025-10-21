#pragma once

#include "objects/Tool.hpp"

class Tools
{
private:
    std::vector<Tool *> tools;
    Tool *currentTool;

public:
    Tools();
    ~Tools();

    void addTool(Tool *tool);
    void removeTool(size_t index);

    void setCurrentTool(Tool *tool);
    Tool *getCurrentTool() const;

    void draw();

    const std::vector<Tool *> &getTools() const;
};
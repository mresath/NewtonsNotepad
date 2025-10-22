#include <imgui-SFML.h>
#include "Tools.hpp"


Tools::Tools() : currentTool(nullptr)
{
    for (int i = SELECT; i < TOOL_END; i++)
    {
        tools.push_back(new Tool(static_cast<ToolType>(i), getToolTexture(static_cast<ToolType>(i))));
    }

    currentTool = tools[0];
    settings = new ToolSettings();
}
Tools::~Tools()
{
    for (Tool *tool : tools)
    {
        delete tool;
    }
    delete settings;
}

void Tools::addTool(Tool *tool)
{
    tools.push_back(tool);
}
void Tools::removeTool(size_t index)
{
    if (index < tools.size())
    {
        tools.erase(tools.begin() + index);
    }
}

void Tools::setCurrentTool(Tool *tool)
{
    currentTool = tool;
    delete settings;
    switch (tool->type)
    {
        case PULL:
            settings = new PullSettings();
            break;
        case PUSH:
            settings = new PushSettings();
            break;
        case DRAW_CIRCLE:
            settings = new CircleSettings();
            break;
        case DRAW_RECTANGLE:
            settings = new RectSettings();
            break;
        case DRAW_ROPE:
            settings = new RopeSettings();
            break;
        case DRAW_SPRING:
            settings = new SpringSettings();
            break;
        default:
            settings = new ToolSettings();
            break;
    }
}
Tool *Tools::getCurrentTool() const
{
    return currentTool;
}

void Tools::draw()
{
    for (Tool *tool : tools)
    {
        bool selected = (tool == currentTool);
        bool pressed = ImGui::ImageButton(tool->getId(), *tool->texture, sf::Vector2f(TOOLS_ICON_SIZE, TOOLS_ICON_SIZE), selected ? TOOL_SELECT_COLOR : TOOL_BG_COLOR);
        if (pressed)
        {
            setCurrentTool(tool);
        }
    }
}

const std::vector<Tool *> &Tools::getTools() const
{
    return tools;
}
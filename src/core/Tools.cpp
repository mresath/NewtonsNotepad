#include <imgui-SFML.h>
#include "Tools.hpp"


Tools::Tools() : currentTool(nullptr)
{
    for (int i = SELECT; i <= DRAW_SPRING; i++)
    {
        sf::Texture *texture = getToolTexture(static_cast<ToolType>(i));

        tools.push_back(new Tool(static_cast<ToolType>(i), texture, Vec2(TOOLS_MARGIN, TOOLS_MARGIN + i * (TOOLS_ICON_SIZE + TOOLS_MARGIN))));
    }
}
Tools::~Tools()
{
    for (Tool *tool : tools)
    {
        delete tool;
    }
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
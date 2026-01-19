#pragma once

#include "Config.h"

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "math/Vec2.hpp"

enum ToolType
{
    SELECT,
    MOVE,
    PULL,
    PUSH,
    DRAW_CIRCLE,
    DRAW_RECTANGLE,
    DRAW_ROPE,
    DRAW_SPRING,
    ERASE,
    TOOL_END
};

inline sf::Texture *getToolTexture(ToolType type)
{
    sf::Texture *texture = new sf::Texture();

    texture->setSmooth(true);
    texture->setRepeated(false);
    bool resized = texture->resize(sf::Vector2u(TOOLS_ICON_SIZE, TOOLS_ICON_SIZE));

    bool loaded = false;
    switch (type)
    {
    case SELECT:
        loaded = texture->loadFromFile("assets/cursor.png");
        break;
    case MOVE:
        loaded = texture->loadFromFile("assets/hand.png");
        break;
    case PULL:
        break;
    case PUSH:
        break;
    case DRAW_CIRCLE:
        break;
    case DRAW_RECTANGLE:
        break;
    case DRAW_ROPE:
        break;
    case DRAW_SPRING:
        break;
    case ERASE:
        break;
    default:
        break;
    }
    return texture;
}

struct Tool
{
    ToolType type;
    sf::Texture *texture;

    Tool(ToolType type, sf::Texture *texture) : type(type), texture(texture)
    {
    }

    ~Tool()
    {
        delete texture;
    }

    const char *getId() const
    {
        switch (type)
        {
        case SELECT:
            return "select";
        case MOVE:
            return "move";
        case PULL:
            return "pull";
        case PUSH:
            return "push";
        case DRAW_CIRCLE:
            return "draw_circle";
        case DRAW_RECTANGLE:
            return "draw_rectangle";
        case DRAW_ROPE:
            return "draw_rope";
        case DRAW_SPRING:
            return "draw_spring";
        case ERASE:
            return "erase";
        default:
            return "unknown_tool";
        }
    }

    const char *getName() const
    {
        switch (type)
        {
        case SELECT:
            return "Select";
        case MOVE:
            return "Move";
        case PULL:
            return "Pull";
        case PUSH:
            return "Push";
        case DRAW_CIRCLE:
            return "Draw Circle";
        case DRAW_RECTANGLE:
            return "Draw Rectangle";
        case DRAW_ROPE:
            return "Draw Rope";
        case DRAW_SPRING:
            return "Draw Spring";
        case ERASE:
            return "Erase";
        default:
            return "Unknown Tool";
        }
    }
};
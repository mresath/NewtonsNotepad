#pragma once

#define TOOLS_ICON_SIZE 32
#define TOOLS_MARGIN 8
#define TOOL_BG_COLOR sf::Color(41, 74, 122, 102)
#define TOOL_SELECT_COLOR sf::Color(66, 150, 250, 200)

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "math/Vec2.hpp"

enum ToolType {
    SELECT,
    MOVE,
    DRAW_CIRCLE,
    DRAW_RECTANGLE,
    DRAW_ROPE,
    DRAW_SPRING,
};

inline sf::Texture* getToolTexture(ToolType type) {
    sf::Texture* texture = new sf::Texture();
    bool loaded = false;
    switch (type) {
        case SELECT:
            loaded = texture->loadFromFile("assets/cursor.png");
            break;
        case MOVE:
            loaded = texture->loadFromFile("assets/hand.png");
            break;
        case DRAW_CIRCLE:
            break;
        case DRAW_RECTANGLE:
            break;
        case DRAW_ROPE:
            break;
        case DRAW_SPRING:
            break;
    }
    return texture;
}

struct Tool {
    ToolType type;
    sf::Texture* texture;
    Vec2 position;

    Tool(ToolType type, sf::Texture* texture, Vec2 position) : type(type), texture(texture), position(position) {
    }

    ~Tool() {
        delete texture;
    }

    const char* getId() const {
        switch (type) {
            case SELECT:
                return "select";
            case MOVE:
                return "move";
            case DRAW_CIRCLE:
                return "draw_circle";
            case DRAW_RECTANGLE:
                return "draw_rectangle";
            case DRAW_ROPE:
                return "draw_rope";
            case DRAW_SPRING:
                return "draw_spring";
            default:
                return "unknown_tool";
        }
    }

    const char* getName() const {
        switch (type) {
            case SELECT:
                return "Select";
            case MOVE:
                return "Move";
            case DRAW_CIRCLE:
                return "Draw Circle";
            case DRAW_RECTANGLE:
                return "Draw Rectangle";
            case DRAW_ROPE:
                return "Draw Rope";
            case DRAW_SPRING:
                return "Draw Spring";
            default:
                return "Unknown Tool";
        }
    }
};
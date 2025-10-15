#pragma once

#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <math.h>
#include "objects/Body.hpp"
#include "math/Util.hpp"

enum ShapeType
{
    CIRCLE,
    RECTANGLE,
};

struct Object
{
    Body *body;
    sf::Shape *shape;
    ShapeType shapeType;

    Vec2 dimensions;
    float volume;

    bool isStatic = false;
    bool doGravity = true;
    bool doDrag = false;
    bool doFriction = false;
    bool canApplyFriction = true;

    Object(Vec2 position, Vec2 dimensions, float density, ShapeType type)
    {
        this->shapeType = type;
        this->dimensions = dimensions;
        Vec2 *len = metersToPixels(&dimensions);
        if (type == CIRCLE)
        {
            shape = new sf::CircleShape(len->x);
            shape->setOrigin(len->x, len->x);
            this->volume = M_PI * dimensions.x * dimensions.x;
        }
        else if (type == RECTANGLE)
        {
            shape = new sf::RectangleShape(sf::Vector2f(len->x, len->y));
            shape->setOrigin(len->x / 2, len->y / 2);
            this->volume = dimensions.x * dimensions.y;
        }
        shape->setFillColor(sf::Color::White);
        body = new Body(position, density * volume);
        delete len;
    }

    void setStatic(bool isStatic)
    {
        this->isStatic = isStatic;
        doGravity = !isStatic;
        if (isStatic)
        {
            doDrag = false;
            doFriction = false;
            canApplyFriction = true;
        }
    }

    void applyForce(const Vec2 &force)
    {
        if (!isStatic)
            body->applyForce(force);
    }

    void update(float dt)
    {
        if (!isStatic)
            body->update(dt);

        Vec2 *pos = metersToPixels(&body->position);
        shape->setPosition(pos->x, pos->y);
        delete pos;
    }

    void draw(sf::RenderWindow *window)
    {
        window->draw(*shape);
    }
};
#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>
#include "objects/Body.hpp"
#include "math/Util.hpp"

enum ShapeType {
    CIRCLE,
    SQUARE
};

struct Object {
    Body* body;
    sf::Shape* shape;

    float r;

    Object(float r, Body* b, sf::Shape* s) : body(b), shape(s), r(r) {}
    Object(Vec2 position, float r, float mass, ShapeType type) {
        this->r = r;
        float len = metersToPixels(r);
        body = new Body(position, mass);
        if (type == CIRCLE) {
            shape = new sf::CircleShape(len);
            shape->setOrigin(len, len);
            body->area = M_PI * r * r;
        } else if (type == SQUARE) {
            shape = new sf::RectangleShape(sf::Vector2f(len, len));
            shape->setOrigin(len / 2, len / 2);
            body->area = r * r;
        }
        shape->setFillColor(sf::Color::White);
    }
    Object(Vec2 position, float r, float mass, ShapeType type, float dragC) {
        this->r = r;
        body = new Body(position, mass);
        body->dragCoefficient = dragC;
        if (type == CIRCLE) {
            shape = new sf::CircleShape(r);
            shape->setOrigin(r, r);
            body->area = M_PI * r * r;
        } else if (type == SQUARE) {
            shape = new sf::RectangleShape(sf::Vector2f(r, r));
            shape->setOrigin(r / 2, r / 2);
            body->area = r * r;
        }
        shape->setFillColor(sf::Color::White);
    }

    void update(float dt) {
        body->update(dt);

        Vec2 * pos = metersToPixels(&body->position);
        shape->setPosition(pos->x, pos->y);
        delete pos;
    }

    void draw(sf::RenderWindow *window) {
        window->draw(*shape);
    }
};
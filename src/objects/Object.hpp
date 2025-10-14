#pragma once

#include <SFML/Graphics.hpp>
#include "objects/Body.hpp"
#include "math/Util.hpp"

struct Object {
    Body* body;
    sf::Shape* shape;

    Object(Body* b, sf::Shape* s) : body(b), shape(s) {}

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
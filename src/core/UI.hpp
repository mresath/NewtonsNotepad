#pragma once

#include "Config.h"

#include <SFML/Graphics.hpp>

inline void handleResize(sf::Window *window, sf::View *newView, sf::View *oldView, const sf::Event::Resized *resized)
{
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
    newView->setSize(sf::Vector2f(visibleArea.size.x, visibleArea.size.y));

    // Clamp position after resize
    sf::Vector2f center = newView->getCenter();
    sf::Vector2f size = oldView->getSize();
    float halfWidth = size.x / 2.0f;
    float halfHeight = size.y / 2.0f;
    float minX = -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) + halfWidth;
    float maxX = (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) - halfWidth;
    float minY = ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT) + halfHeight;
    float maxY = (DEF_HEIGHT - HALF_WALL_THICKNESS) - halfHeight;

    if (center.x - halfWidth < -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = minX;
    else if (center.x + halfWidth > (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = maxX;

    if (center.y - halfHeight < ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT))
        center.y = minY;
    else if (center.y + halfHeight > (DEF_HEIGHT - HALF_WALL_THICKNESS))
        center.y = maxY;

    newView->setCenter(center);
}

inline void handleZoom(sf::RenderWindow *window, sf::View *newView, sf::View *oldView, float delta, float *accumulatedZoom)
{
    auto viewSize = oldView->getSize();
    float decFactor = 1 - ZOOM_STEP;
    float incFactor = 1 + ZOOM_STEP;

    // Get mouse position in world coordinates before zoom
    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(*window);
    sf::Vector2f mouseWorldPosBefore = window->mapPixelToCoords(mousePixelPos, *oldView);

    // Apply zoom
    if (delta > 0 && viewSize.x * decFactor >= DEF_WIDTH / 2.5 && viewSize.y * decFactor >= DEF_HEIGHT / 2.5)
    {
        newView->zoom(decFactor);
        *accumulatedZoom /= incFactor;
    }
    else if (delta < 0 && viewSize.x * incFactor <= WORLD_WIDTH && viewSize.y * incFactor <= WORLD_HEIGHT)
    {
        newView->zoom(incFactor);
        *accumulatedZoom /= decFactor;
    }

    // Get mouse position in world coordinates after zoom
    sf::Vector2f mouseWorldPosAfter = window->mapPixelToCoords(mousePixelPos, *newView);

    // Adjust view center to keep mouse position fixed in world space
    sf::Vector2f offset = mouseWorldPosBefore - mouseWorldPosAfter;
    newView->move(offset);

    // Clamp position after zoom
    sf::Vector2f center = newView->getCenter();
    sf::Vector2f size = newView->getSize();
    float halfWidth = size.x / 2.0f;
    float halfHeight = size.y / 2.0f;
    float minX = -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) + halfWidth;
    float maxX = (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) - halfWidth;
    float minY = ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT) + halfHeight;
    float maxY = (DEF_HEIGHT - HALF_WALL_THICKNESS) - halfHeight;

    if (center.x - halfWidth < -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = minX;
    else if (center.x + halfWidth > (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = maxX;

    if (center.y - halfHeight < ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT))
        center.y = minY;
    else if (center.y + halfHeight > (DEF_HEIGHT - HALF_WALL_THICKNESS))
        center.y = maxY;

    newView->setCenter(center);
}

inline void handlePanMouse(sf::RenderWindow *window, sf::View *newView, sf::View *oldView, const sf::Event::MouseMoved *mouseMoved, sf::Vector2f &lastMousePos, float accumulatedZoom)
{
    // Move based on mouse delta
    sf::Vector2f currentMousePos = sf::Vector2f(mouseMoved->position.x, mouseMoved->position.y);
    sf::Vector2f deltaPos = lastMousePos - currentMousePos;
    deltaPos *= accumulatedZoom;

    newView->move(deltaPos);
    lastMousePos = currentMousePos;

    // Clamp position to stay within world bounds
    sf::Vector2f center = newView->getCenter();
    sf::Vector2f size = newView->getSize();
    float halfWidth = size.x / 2.0f;
    float halfHeight = size.y / 2.0f;
    float minX = -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) + halfWidth;
    float maxX = (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) - halfWidth;
    float minY = ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT) + halfHeight;
    float maxY = (DEF_HEIGHT - HALF_WALL_THICKNESS) - halfHeight;

    if (center.x - halfWidth < -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = minX;
    else if (center.x + halfWidth > (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = maxX;

    if (center.y - halfHeight < ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT))
        center.y = minY;
    else if (center.y + halfHeight > (DEF_HEIGHT - HALF_WALL_THICKNESS))
        center.y = maxY;

    newView->setCenter(center);
}

inline void handlePanKeyboard(sf::RenderWindow *window, sf::View *newView, sf::View *oldView)
{
    sf::Vector2f movement(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
    {
        movement.y -= PAN_SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        movement.y += PAN_SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        movement.x -= PAN_SPEED;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        movement.x += PAN_SPEED;
    }

    newView->move(movement);

    // Clamp position after keyboard pan
    sf::Vector2f center = newView->getCenter();
    sf::Vector2f size = newView->getSize();
    float halfWidth = size.x / 2.0f;
    float halfHeight = size.y / 2.0f;
    float minX = -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) + halfWidth;
    float maxX = (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS) - halfWidth;
    float minY = ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT) + halfHeight;
    float maxY = (DEF_HEIGHT - HALF_WALL_THICKNESS) - halfHeight;

    if (center.x - halfWidth < -(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = minX;
    else if (center.x + halfWidth > (WORLD_WIDTH / 2 - HALF_WALL_THICKNESS))
        center.x = maxX;

    if (center.y - halfHeight < ((DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT))
        center.y = minY;
    else if (center.y + halfHeight > (DEF_HEIGHT - HALF_WALL_THICKNESS))
        center.y = maxY;
        
    newView->setCenter(center);
}
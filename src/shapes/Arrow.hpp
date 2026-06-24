#pragma once

#include <SFML/Graphics.hpp>

class Arrow : public sf::Drawable, public sf::Transformable {
public:
    Arrow(float length, float thickness, float headLength, float headWidth, sf::Color color) {
        // Define the 7 points of the arrow facing right, starting from the back center
        arrowVertices.setPrimitiveType(sf::PrimitiveType::TriangleFan);
        arrowVertices.resize(7);

        float shaftLength = length - headLength;
        float halfThickness = thickness / 2.0f;
        float halfHeadWidth = headWidth / 2.0f;

        // Coordinates relative to the origin (0, 0)
        arrowVertices[0].position = sf::Vector2f(length, 0.0f);                  // Tip
        arrowVertices[1].position = sf::Vector2f(shaftLength, -halfHeadWidth);   // Head top corner
        arrowVertices[2].position = sf::Vector2f(shaftLength, -halfThickness);   // Shaft top-right
        arrowVertices[3].position = sf::Vector2f(0.0f, -halfThickness);          // Shaft top-left
        arrowVertices[4].position = sf::Vector2f(0.0f, halfThickness);           // Shaft bottom-left
        arrowVertices[5].position = sf::Vector2f(shaftLength, halfThickness);    // Shaft bottom-right
        arrowVertices[6].position = sf::Vector2f(shaftLength, halfHeadWidth);    // Head bottom corner

        // Apply color
        for (size_t i = 0; i < 7; ++i) {
            arrowVertices[i].color = color;
        }
    }

private:
    sf::VertexArray arrowVertices;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states.transform *= getTransform();
        target.draw(arrowVertices, states);
    }
};

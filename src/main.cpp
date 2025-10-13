#include <SFML/Graphics.hpp>
#include "objects/Body.hpp"
#include "core/World.hpp"

// Methods
void updateShapes(std::vector<sf::CircleShape> &shapes, const std::vector<Body *> &bodies) // Update all shapes to match body positions
{
    for (size_t i = 0; i < bodies.size(); ++i)
    {
        shapes[i].setPosition(bodies[i]->position.x, bodies[i]->position.y);
    }
}

void drawShapes(sf::RenderWindow &window, const std::vector<sf::CircleShape> &shapes) // Draw all shapes to the window
{
    for (const auto &shape : shapes)
    {
        window.draw(shape);
    }
}

sf::CircleShape createCircleShape(std::vector<sf::CircleShape> &shapes, float radius, const sf::Color &color) // Create and store a circle shape
{
    sf::CircleShape circle(radius);
    circle.setFillColor(color);
    circle.setOrigin(radius, radius);
    shapes.push_back(circle);
    return circle;
}

sf::RectangleShape createSquareShape(std::vector<sf::RectangleShape> &shapes, const float size, const sf::Color &color) // Create and store a square shape
{
    sf::RectangleShape square(sf::Vector2f(size, size));
    square.setFillColor(color);
    square.setOrigin(size / 2, size / 2);
    shapes.push_back(square);
    return square;
}

// Entry point
int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Newton's Notepad");
    window.setFramerateLimit(60);

    // Clock for calculating delta time
    sf::Clock clock;

    // Initialize world and objects
    World world;
    std::vector<sf::CircleShape> shapes;

    // Create a ball for demonstration
    Body *ball = new Body(Vec2(400, 300), 1.0f);
    world.addBody(ball);
    sf::CircleShape circle = createCircleShape(shapes, 10.0f, sf::Color::Green);

    // Main loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window : exit
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        // Calculate delta time independent of frame rate
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f)
            dt = 0.05f;

        // Update world and bodies
        world.update(dt);

        // Update shapes to match body positions
        const std::vector<Body *> &bodies = world.getBodies();
        updateShapes(shapes, bodies);

        // Clear screen and draw shapes
        window.clear(sf::Color::Black);
        drawShapes(window, shapes);
        window.display();
    }

    return 0;
}
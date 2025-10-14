#include <SFML/Graphics.hpp>
#include "core/World.hpp"

// Methods
sf::CircleShape* createCircleShape(float radius, const sf::Color &color)
{
    sf::CircleShape *circle = new sf::CircleShape(radius);
    circle->setFillColor(color);
    circle->setOrigin(radius, radius);
    return circle;
}

sf::RectangleShape* createSquareShape(const float size, const sf::Color &color)
{
    sf::RectangleShape* square = new sf::RectangleShape(sf::Vector2f(size, size));
    square->setFillColor(color);
    square->setOrigin(size / 2, size / 2);
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

    // Create a ball for demonstration
    Object *ballObject = new Object(*pixelsToMeters(new Vec2(400, 300)), 0.25f, 1.0f, CIRCLE);
    world.addObject(ballObject);

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

        // Clear screen and draw shapes
        window.clear(sf::Color::Black);
        world.draw(&window);
        window.display();
    }

    return 0;
}
#include <SFML/Graphics.hpp>
#include "core/World.hpp"

#define WORLD_WIDTH 4000
#define WORLD_HEIGHT 4000
#define THICKNESS 30
#define LIGHTGREY sf::Color(140, 140, 140, 255)

// Methods
sf::CircleShape *createCircleShape(float radius, const sf::Color &color)
{
    sf::CircleShape *circle = new sf::CircleShape(radius);
    circle->setFillColor(color);
    circle->setOrigin(radius, radius);
    return circle;
}

sf::RectangleShape *createSquareShape(const float size, const sf::Color &color)
{
    sf::RectangleShape *square = new sf::RectangleShape(sf::Vector2f(size, size));
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
    Object *ballObject = new Object(*pixelsToMeters(new Vec2(400, 300)), Vec2(0.25, 0.25), 1.0f, CIRCLE);
    world.addObject(ballObject);

    // Create ground and walls
    Object *ground = new Object(*pixelsToMeters(new Vec2(400, 600 - THICKNESS / 2)), *pixelsToMeters(new Vec2(WORLD_WIDTH, THICKNESS)), 1.0f, RECTANGLE);
    Object *leftWall = new Object(*pixelsToMeters(new Vec2(-(WORLD_WIDTH + THICKNESS / 2), WORLD_HEIGHT / 2)), *pixelsToMeters(new Vec2(THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *rightWall = new Object(*pixelsToMeters(new Vec2(WORLD_WIDTH + THICKNESS / 2, WORLD_HEIGHT / 2)), *pixelsToMeters(new Vec2(THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    ground->setStatic(true);
    leftWall->setStatic(true);
    rightWall->setStatic(true);
    ground->shape->setFillColor(LIGHTGREY);
    leftWall->shape->setFillColor(LIGHTGREY);
    rightWall->shape->setFillColor(LIGHTGREY);
    world.addObject(ground);
    world.addObject(leftWall);
    world.addObject(rightWall);

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
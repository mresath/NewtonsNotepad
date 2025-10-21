#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include "core/World.hpp"
#include "core/Tools.hpp"

#define WORLD_HEIGHT 2250.f
#define WORLD_WIDTH 4000.f
#define DEF_HEIGHT 450.f
#define DEF_WIDTH 800.f
#define WALL_THICKNESS 30.f
#define HALF_WALL_THICKNESS (WALL_THICKNESS / 2)
#define WALL_COLOR sf::Color(140, 140, 140, 255)
#define ZOOM_STEP 0.025f

// Methods
sf::CircleShape *createCircleShape(float radius, const sf::Color &color)
{
    sf::CircleShape *circle = new sf::CircleShape(radius);
    circle->setFillColor(color);
    circle->setOrigin(sf::Vector2f(radius, radius));
    return circle;
}

sf::RectangleShape *createSquareShape(const float size, const sf::Color &color)
{
    sf::RectangleShape *square = new sf::RectangleShape(sf::Vector2f(size, size));
    square->setFillColor(color);
    square->setOrigin(sf::Vector2f(size / 2, size / 2));
    return square;
}

// Entry point
int main()
{
    // Create the main window and ui
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(DEF_WIDTH, DEF_HEIGHT)), "Newton's Notepad");
    if(!ImGui::SFML::Init(window)) return -1;
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(sf::Vector2f(-DEF_HEIGHT / 2, -DEF_WIDTH / 2), sf::Vector2f(DEF_WIDTH, DEF_HEIGHT)));
    view.setCenter(sf::Vector2f(0, DEF_HEIGHT / 2));
    window.setView(view);

    // UI Styling
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 6.0f);

    // Clock for calculating delta time
    sf::Clock clock;

    // Initialize world and objects
    World world;
    Tools tools;

    // Create a ball for demonstration
    Object *ballObject = new Object(*pixelsToMeters(new Vec2(0, DEF_HEIGHT / 2)), Vec2(0.25, 0.25), 1.0f, CIRCLE);
    world.addObject(ballObject);

    // Create ground and walls
    Object *ground = new Object(*pixelsToMeters(new Vec2(0, DEF_HEIGHT - HALF_WALL_THICKNESS)), *pixelsToMeters(new Vec2(WORLD_WIDTH, WALL_THICKNESS)), 1.0f, RECTANGLE);
    Object *leftWall = new Object(*pixelsToMeters(new Vec2(-(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS), (DEF_HEIGHT - HALF_WALL_THICKNESS) - (WORLD_HEIGHT / 2))), *pixelsToMeters(new Vec2(WALL_THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *rightWall = new Object(*pixelsToMeters(new Vec2(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS, (DEF_HEIGHT - HALF_WALL_THICKNESS) - (WORLD_HEIGHT / 2))), *pixelsToMeters(new Vec2(WALL_THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *ceiling = new Object(*pixelsToMeters(new Vec2(0, (DEF_HEIGHT - HALF_WALL_THICKNESS) - WORLD_HEIGHT)), *pixelsToMeters(new Vec2(WORLD_WIDTH, WALL_THICKNESS)), 1.0f, RECTANGLE);
    ground->setStatic(true);
    leftWall->setStatic(true);
    rightWall->setStatic(true);
    ceiling->setStatic(true);
    ground->shape->setFillColor(WALL_COLOR);
    leftWall->shape->setFillColor(WALL_COLOR);
    rightWall->shape->setFillColor(WALL_COLOR);
    ceiling->shape->setFillColor(WALL_COLOR);
    world.addObject(ground);
    world.addObject(leftWall);
    world.addObject(rightWall);
    world.addObject(ceiling);

    // Main loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            sf::View newView(window.getView());
            // Close window : exit
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto *resized = event->getIf<sf::Event::Resized>()) // Resize event: adjust the view to the new window size
            {
                sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized->size));
                newView.setSize(sf::Vector2f(visibleArea.size.x, visibleArea.size.y));
                newView.setCenter(sf::Vector2f(view.getCenter().x, (DEF_HEIGHT - HALF_WALL_THICKNESS) - (visibleArea.size.y / 2)));
            }
            else if (const auto *scroll = event->getIf<sf::Event::MouseWheelScrolled>()) // Zoom in/out with mouse wheel
            {
                auto viewSize = view.getSize();
                float decFactor = 1 - ZOOM_STEP;
                float incFactor = 1 + ZOOM_STEP;
                if (scroll->delta > 0 && viewSize.x * decFactor >= DEF_WIDTH / 2.5 && viewSize.y * decFactor >= DEF_HEIGHT / 2.5)
                    newView.zoom(decFactor); // Zoom in
                else if (scroll->delta < 0 && viewSize.x * incFactor <= WORLD_WIDTH + WALL_THICKNESS && viewSize.y * incFactor <= WORLD_HEIGHT + WALL_THICKNESS)
                    newView.zoom(incFactor); // Zoom out
                newView.setCenter(sf::Vector2f(view.getCenter().x, (DEF_HEIGHT - HALF_WALL_THICKNESS) - (newView.getSize().y / 2)));
            }
            // Clamp and update view
            newView.setCenter(sf::Vector2f(newView.getCenter().x, (DEF_HEIGHT - HALF_WALL_THICKNESS) - (newView.getSize().y / 2)));
            window.setView(newView);
            view = newView;
        }

        // Calculate delta time independent of frame rate
        sf::Time dtTime = clock.restart();
        float dt = dtTime.asSeconds();
        if (dt > 0.05f)
            dt = 0.05f;

        // Update UI and tools
        ImGui::SFML::Update(window, dtTime);
        
        ImGui::Begin("Tools");
        tools.draw();
        ImGui::End();

        // Update world and bodies
        world.update(dt);

        // Clear screen and draw world & ui
        window.clear(sf::Color::Black);
        world.draw(&window);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
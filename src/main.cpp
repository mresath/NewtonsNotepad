#include "Config.h"

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include "core/World.hpp"
#include "core/Tools.hpp"

// Entry point
int main()
{
    // Constant and global variables
    const int toolFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    const int propFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    const int toolSettingsFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    Object *selectedObject = nullptr;
    Object *grabbedObject = nullptr;

    bool isPanning = false;
    float toolForceMag = 0.0f;
    float accumulatedZoom = 1.0f;
    sf::Vector2f lastMousePos;

    // Create the main window and ui
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(DEF_WIDTH, DEF_HEIGHT)), "Newton's Notepad");
    if (!ImGui::SFML::Init(window))
        return -1;
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(sf::Vector2f(-DEF_HEIGHT / 2, -DEF_WIDTH / 2), sf::Vector2f(DEF_WIDTH, DEF_HEIGHT)));
    view.setCenter(sf::Vector2f(0, DEF_HEIGHT / 2));
    window.setView(view);

    // UI Styling
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, Y_ITEM_SPACING);

    // Clock for calculating delta time
    sf::Clock clock;

    // Initialize world and objects
    World world;
    Tools tools;

    // Create ground and walls
    Object *ground = new Object(*pixelsToMeters(new Vec2(0, DEF_HEIGHT - HALF_WALL_THICKNESS)), *pixelsToMeters(new Vec2(WORLD_WIDTH, WALL_THICKNESS)), 1.0f, RECTANGLE);
    Object *leftWall = new Object(*pixelsToMeters(new Vec2(-(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS), (DEF_HEIGHT - HALF_WALL_THICKNESS) - (WORLD_HEIGHT / 2))), *pixelsToMeters(new Vec2(WALL_THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *rightWall = new Object(*pixelsToMeters(new Vec2(WORLD_WIDTH / 2 - HALF_WALL_THICKNESS, (DEF_HEIGHT - HALF_WALL_THICKNESS) - (WORLD_HEIGHT / 2))), *pixelsToMeters(new Vec2(WALL_THICKNESS, WORLD_HEIGHT)), 1.0f, RECTANGLE);
    Object *ceiling = new Object(*pixelsToMeters(new Vec2(0, (DEF_HEIGHT + HALF_WALL_THICKNESS) - WORLD_HEIGHT)), *pixelsToMeters(new Vec2(WORLD_WIDTH, WALL_THICKNESS)), 1.0f, RECTANGLE);
    ground->setConstant();
    leftWall->setConstant();
    rightWall->setConstant();
    ceiling->setConstant();
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

                // Clamp position after resize
                sf::Vector2f center = view.getCenter();
                sf::Vector2f size = newView.getSize();
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

                newView.setCenter(center);
            }
            else if (!ImGui::GetIO().WantCaptureMouse)
            {
                // Make sure UI is not using the mouse before processing mouse events
                if (const auto *scroll = event->getIf<sf::Event::MouseWheelScrolled>()) // Zoom in/out with mouse wheel
                {
                    auto viewSize = view.getSize();
                    float decFactor = 1 - ZOOM_STEP;
                    float incFactor = 1 + ZOOM_STEP;

                    // Get mouse position in world coordinates before zoom
                    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
                    sf::Vector2f mouseWorldPosBefore = window.mapPixelToCoords(mousePixelPos, view);

                    // Apply zoom
                    if (scroll->delta > 0 && viewSize.x * decFactor >= DEF_WIDTH / 2.5 && viewSize.y * decFactor >= DEF_HEIGHT / 2.5)
                    {
                        newView.zoom(decFactor);
                        accumulatedZoom /= incFactor;
                    }
                    else if (scroll->delta < 0 && viewSize.x * incFactor <= WORLD_WIDTH && viewSize.y * incFactor <= WORLD_HEIGHT)
                    {
                        newView.zoom(incFactor);
                        accumulatedZoom /= decFactor;
                    }

                    // Get mouse position in world coordinates after zoom
                    sf::Vector2f mouseWorldPosAfter = window.mapPixelToCoords(mousePixelPos, newView);

                    // Adjust view center to keep mouse position fixed in world space
                    sf::Vector2f offset = mouseWorldPosBefore - mouseWorldPosAfter;
                    newView.move(offset);

                    // Clamp position after zoom
                    sf::Vector2f center = newView.getCenter();
                    sf::Vector2f size = newView.getSize();
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

                    newView.setCenter(center);
                }
                else if (const auto *mouseMoved = event->getIf<sf::Event::MouseMoved>()) // Pan view when right mouse button is held
                {
                    if (isPanning)
                    {
                        // Move based on mouse delta
                        sf::Vector2f currentMousePos = sf::Vector2f(mouseMoved->position.x, mouseMoved->position.y);
                        sf::Vector2f deltaPos = lastMousePos - currentMousePos;
                        deltaPos *= accumulatedZoom;
                        newView.move(deltaPos);
                        lastMousePos = currentMousePos;

                        // Clamp position to stay within world bounds
                        sf::Vector2f center = newView.getCenter();
                        sf::Vector2f size = newView.getSize();
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

                        newView.setCenter(center);
                    }
                }
                else if (const auto *mouseDown = event->getIf<sf::Event::MouseButtonPressed>())
                {
                    if (mouseDown->button == sf::Mouse::Button::Middle)
                    {
                        // Reset view on middle mouse button
                        newView.setSize(sf::Vector2f(DEF_WIDTH, DEF_HEIGHT));
                        newView.setCenter(sf::Vector2f(0, DEF_HEIGHT / 2));
                        accumulatedZoom = 1.0f;
                    }
                    else if (mouseDown->button == sf::Mouse::Button::Right)
                    {
                        // Start panning on right mouse button
                        isPanning = true;
                        lastMousePos = sf::Vector2f(sf::Mouse::getPosition(window));
                    }
                    else if (mouseDown->button == sf::Mouse::Button::Left)
                    {
                        // Handle tool actions
                        ToolType type = tools.getCurrentTool()->type;
                        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(mouseDown->position));
                        Vec2 pixelsPos = Vec2(mousePos.x, mousePos.y);
                        Vec2 metersPos = *pixelsToMeters(&pixelsPos);
                        if (type == SELECT)
                        {
                            bool found = false;
                            for (Object *obj : world.getObjects())
                            {
                                if (obj->isSelectable && obj->shape->getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseDown->position))))
                                {
                                    selectedObject = obj;
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                            {
                                selectedObject = nullptr;
                            }
                        }
                        else if (type == MOVE)
                        {
                            for (Object *obj : world.getObjects())
                            {
                                if (obj->isSelectable && obj->shape->getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseDown->position))))
                                {
                                    obj->isGrabbed = true;
                                    grabbedObject = obj;
                                    break;
                                }
                            }
                        }
                        else if (type == PULL || type == PUSH)
                        {
                            float forceMag;
                            if (type == PULL)
                            {
                                forceMag = static_cast<PullSettings *>(tools.settings)->forceMagnitude * -1;
                            }
                            else
                            {
                                forceMag = static_cast<PushSettings *>(tools.settings)->forceMagnitude;
                            }
                            toolForceMag = forceMag;

                            for (Object *obj : world.getObjects())
                            {
                                Vec2 objPos = obj->body->position;
                                Vec2 diff = objPos - metersPos;
                                float distanceSquared = diff.lengthSquared();
                                if (distanceSquared < 0.0001f) // Prevent singularity
                                    continue;
                                float attenuation = 1.0f / distanceSquared;
                                float mag = attenuation;
                                Vec2 force = diff.normalized() * mag;
                                if (distanceSquared < mag) // Apply force only if within effective range of more than 1N of applied force
                                {
                                    obj->applyForce(force);
                                }
                            }
                        }
                        else if (type == DRAW_CIRCLE)
                        {
                            CircleSettings *circleSettings = static_cast<CircleSettings *>(tools.settings);
                            Object *newCircle = new Object(metersPos, Vec2(circleSettings->radius, circleSettings->radius), circleSettings->density, CIRCLE);
                            newCircle->setStatic(circleSettings->isStatic);

                            newCircle->body->dragCoefficient = circleSettings->dragCoefficient;
                            newCircle->body->staticFriction = circleSettings->staticFriction;
                            newCircle->body->kineticFriction = circleSettings->kineticFriction;
                            newCircle->body->restitution = circleSettings->restitution;

                            world.addObject(newCircle);
                        }
                        else if (type == DRAW_RECTANGLE)
                        {
                            RectSettings *rectSettings = static_cast<RectSettings *>(tools.settings);
                            Object *newRect = new Object(metersPos, Vec2(rectSettings->width, rectSettings->height), rectSettings->density, RECTANGLE);

                            newRect->body->dragCoefficient = rectSettings->dragCoefficient;
                            newRect->body->staticFriction = rectSettings->staticFriction;
                            newRect->body->kineticFriction = rectSettings->kineticFriction;
                            newRect->body->restitution = rectSettings->restitution;

                            newRect->setStatic(rectSettings->isStatic);
                            world.addObject(newRect);
                        }
                        else if (type == ERASE)
                        {
                            for (size_t i = 0; i < world.getObjects().size(); ++i)
                            {
                                Object *obj = world.getObjects()[i];
                                if (obj->isSelectable && obj->shape->getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseDown->position))))
                                {
                                    world.removeObject(i);
                                    if (selectedObject == obj)
                                        selectedObject = nullptr;
                                    break;
                                }
                            }
                        }
                    }
                }
                else if (const auto *mouseUp = event->getIf<sf::Event::MouseButtonReleased>())
                {
                    if (mouseUp->button == sf::Mouse::Button::Left)
                    {
                        // Release grabbed object on left mouse button release
                        if (grabbedObject != nullptr)
                            grabbedObject->isGrabbed = false;
                        grabbedObject = nullptr;

                        toolForceMag = 0.0f;
                    }
                    else if (mouseUp->button == sf::Mouse::Button::Right)
                    {
                        // Stop panning on right mouse button release
                        isPanning = false;
                    }
                }
            }
            // Update view
            window.setView(newView);
            view = newView;
        }

        // Calculate delta time independent of frame rate
        sf::Time dtTime = clock.restart();
        float dt = dtTime.asSeconds();
        if (dt > MAX_DT)
            dt = MAX_DT;

        // Update UI and tools
        ImGui::SFML::Update(window, dtTime);

        ImGui::Begin("Tools", nullptr, toolFlags);
        tools.draw();
        ImGui::End();

        Tool *currentTool = tools.getCurrentTool();
        ToolType type = currentTool->type;

        ImGui::Begin("Tool Settings", nullptr, toolSettingsFlags);
        char nameBuffer[256];
        std::strcpy(nameBuffer, currentTool->getName());
        std::strcat(nameBuffer, " Tool");
        ImGui::Text(nameBuffer);
        if (type == SELECT)
        {
            ImGui::Text("Left Click to select object");
        }
        else if (type == MOVE)
        {
            ImGui::Text("Left Click and drag to move object");
        }
        else if (type == PULL || type == PUSH)
        {
            ImGui::Text("Left Click to apply force");
            ImGui::Separator();
            if (type == PULL)
            {
                ImGui::DragFloat("Strength", &static_cast<PullSettings *>(tools.settings)->forceMagnitude, FORCE_STEP, MIN_FORCE, MAX_FORCE);
            }
            else if (type == PUSH)
            {
                ImGui::DragFloat("Strength", &static_cast<PushSettings *>(tools.settings)->forceMagnitude, FORCE_STEP, MIN_FORCE, MAX_FORCE);
            }
        }
        else if (type == DRAW_CIRCLE || type == DRAW_RECTANGLE)
        {
            ImGui::Text("Left Click to place object");
            ImGui::Separator();
            if (type == DRAW_CIRCLE)
            {
                CircleSettings *circleSettings = static_cast<CircleSettings *>(tools.settings);
                ImGui::Checkbox("Is Static", &circleSettings->isStatic);
                ImGui::DragFloat("Radius", &circleSettings->radius, LENGTH_STEP, MIN_LENGTH, MAX_LENGTH);
                ImGui::DragFloat("Density", &circleSettings->density, DENSITY_STEP, MIN_DENSITY, MAX_DENSITY);
                ImGui::DragFloat("Drag Coefficient", &circleSettings->dragCoefficient, DRAG_STEP, MIN_DRAG, MAX_DRAG);
                ImGui::DragFloat("Static Friction", &circleSettings->staticFriction, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
                ImGui::DragFloat("Kinetic Friction", &circleSettings->kineticFriction, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
                ImGui::DragFloat("Restitution", &circleSettings->restitution, RESTITUTION_STEP, MIN_RESTITUTION, MAX_RESTITUTION);
            }
            else if (type == DRAW_RECTANGLE)
            {
                RectSettings *rectSettings = static_cast<RectSettings *>(tools.settings);
                ImGui::Checkbox("Is Static", &rectSettings->isStatic);
                ImGui::DragFloat("Width", &rectSettings->width, LENGTH_STEP, MIN_LENGTH * 2, MAX_LENGTH * 2);
                ImGui::DragFloat("Height", &rectSettings->height, LENGTH_STEP, MIN_LENGTH * 2, MAX_LENGTH * 2);
                ImGui::DragFloat("Density", &rectSettings->density, DENSITY_STEP, MIN_DENSITY, MAX_DENSITY);
                ImGui::DragFloat("Drag", &rectSettings->dragCoefficient, DRAG_STEP, MIN_DRAG, MAX_DRAG);
                ImGui::DragFloat("Static Friction", &rectSettings->staticFriction, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
                ImGui::DragFloat("Kinetic Friction", &rectSettings->kineticFriction, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
                ImGui::DragFloat("Restitution", &rectSettings->restitution, RESTITUTION_STEP, MIN_RESTITUTION, MAX_RESTITUTION);
            }
        }
        else if (type == DRAW_ROPE || type == DRAW_SPRING)
        {
            ImGui::Text("Left Click and drag to create connection");
            ImGui::Separator();
        }
        else if (type == ERASE)
        {
            ImGui::Text("Left Click to erase object");
        }
        ImGui::End();

        if (selectedObject != nullptr)
        {
            ImGui::Begin("Object Properties", nullptr, propFlags);
            ImGui::Text(selectedObject->shapeType == CIRCLE ? "Circle" : "Rectangle");
            ImGui::Separator();

            ImGui::End();
        }

        // Handle grabbed object movement
        if (grabbedObject != nullptr && grabbedObject->isSelectable)
        {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            Vec2 pixelsPos = Vec2(mouseWorldPos.x, mouseWorldPos.y);
            Vec2 targetPos = *pixelsToMeters(&pixelsPos);
            if (grabbedObject->isStatic)
            {
                grabbedObject->body->position = targetPos;
                grabbedObject->body->velocity = Vec2(0.0f, 0.0f);
            }
            else
            {
                Vec2 posDiff = targetPos - grabbedObject->body->position;
                Vec2 desiredVelocity = posDiff / 0.05f; // Proportional controller
                Vec2 deltaV = desiredVelocity - grabbedObject->body->velocity;
                Vec2 approxForce = deltaV * grabbedObject->body->mass / dt;
                grabbedObject->applyForce(approxForce);
            }
        }

        // Handle tool continuous effects
        if (toolForceMag != 0.0f)
        {
            sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePixelPos);
            Vec2 pixelsPos = Vec2(mouseWorldPos.x, mouseWorldPos.y);
            Vec2 metersPos = *pixelsToMeters(&pixelsPos);
            for (Object *obj : world.getObjects())
            {
                Vec2 objPos = obj->body->position;
                Vec2 diff = objPos - metersPos;
                float distanceSquared = diff.lengthSquared();
                if (distanceSquared < 0.0001f) // Prevent singularity
                    continue;
                float attenuation = 1.0f / distanceSquared;
                float mag = toolForceMag * attenuation;
                Vec2 force = diff.normalized() * mag;
                if (distanceSquared < std::abs(mag)) // Apply force only if within effective range of more than 1N of applied force
                {
                    obj->applyForce(force);
                }
            }
        }

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
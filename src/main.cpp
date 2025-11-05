#include "Config.h"

#include <iostream>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include "core/World.hpp"
#include "core/Tools.hpp"
#include "core/UI.hpp"

// Entry point
int main()
{
    // Constant and global variables
    const int toolFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    const int propFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    const int toolSettingsFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

    bool settingsOpen = false;

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

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
    Vec2 pixelsPos = Vec2(worldPos.x, worldPos.y);
    Vec2 *posPointer = pixelsToMeters(&pixelsPos);

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
            else if (const sf::Event::Resized *resized = event->getIf<sf::Event::Resized>()) // Resize event: adjust the view to the new window size
            {
                handleResize(&window, &newView, &view, resized);
            }
            else if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>())
            {
                if (keyReleased->code == sf::Keyboard::Key::Escape)
                {
                    settingsOpen = !settingsOpen;
                }
            }
            else if (!ImGui::GetIO().WantCaptureMouse)
            {
                // Make sure UI is not using the mouse before processing mouse events
                if (const auto *scroll = event->getIf<sf::Event::MouseWheelScrolled>()) // Zoom in/out with mouse wheel
                {
                    handleZoom(&window, &newView, &view, scroll->delta, &accumulatedZoom);
                }
                else if (const auto *mouseMoved = event->getIf<sf::Event::MouseMoved>()) // Pan view when right mouse button is held
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                    Vec2 pixelsPos = Vec2(worldPos.x, worldPos.y);
                    *posPointer = *pixelsToMeters(&pixelsPos);
                    if (isPanning)
                    {
                        handlePanMouse(&window, &newView, &view, mouseMoved, lastMousePos, accumulatedZoom);
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
                                    selectedObject = obj;

                                    bool isStatic = obj->isStatic;

                                    obj->applyForce(ForceSource("grab", [posPointer, isStatic](Body state)
                                                                {
                                                                    if (isStatic)
                                                                        return Force();

                                                                    Vec2 posDiff = *posPointer - state.position;
                                                                    Vec2 desiredVel = posDiff / 0.05f;
                                                                    Vec2 deltaV = desiredVel - state.velocity;
                                                                    Vec2 approxForce = deltaV * state.mass / 0.05f;
                                                                    return Force(Vec2(0.0f, 0.0f), approxForce); }));

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
                                obj->applyForce(ForceSource("tool", [posPointer, forceMag](Body state)
                                                            {
                                    Vec2 pos = state.position;
                                    Vec2 diff = pos - *posPointer;
                                    float distanceSquared = diff.lengthSquared();
                                    float attenuation = std::max(1.0f / distanceSquared, MAX_ATTENUATION);
                                    Vec2 force = diff.normalized() * attenuation * forceMag * FORCE_SCALE;
                                    return Force(Vec2(0, 0), force); }));
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
                        {
                            grabbedObject->deleteForce("grab");
                            grabbedObject->isGrabbed = false;
                        };
                        grabbedObject = nullptr;

                        if (toolForceMag != 0.0f)
                        {
                            for (Object *obj : world.getObjects())
                            {
                                obj->deleteForce("tool");
                            }
                        }
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

        // Tool settings window
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

        // Object properties window
        if (selectedObject != nullptr)
        {
            ImGui::Begin("Object Properties", nullptr, propFlags);
            if (selectedObject->shapeType == CIRCLE)
            {
                ImGui::Text("Circle [%d]", selectedObject->getID());
                ImGui::Separator();
                ImGui::Text("Radius: %.2f m", selectedObject->dimensions.x);
            }
            else
            {
                ImGui::Text("Rectangle [%d]", selectedObject->getID());
                ImGui::Separator();
                ImGui::Text("Width: %.2f m", selectedObject->dimensions.x);
                ImGui::Text("Height: %.2f m", selectedObject->dimensions.y);
            }
            ImGui::Text("Mass: %.2f kg", selectedObject->body->mass);
            ImGui::Separator();
            ImGui::Text("Net Force: %s N", (selectedObject->body->acceleration * selectedObject->body->mass).toString().c_str());
            ImGui::Text("Acceleration: %s m/s²", selectedObject->body->acceleration.toString().c_str());
            ImGui::Text("Velocity: %s m/s", selectedObject->body->velocity.toString().c_str());
            ImGui::Text("Position: %s m", selectedObject->body->position.toString().c_str());
            ImGui::Separator();
            ImGui::DragFloat("Drag Coefficient", &selectedObject->body->dragCoefficient, DRAG_STEP, MIN_DRAG, MAX_DRAG);
            ImGui::DragFloat("Static Friction", &selectedObject->body->staticFriction, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
            ImGui::DragFloat("Kinetic Friction", &selectedObject->body->kineticFriction, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
            ImGui::DragFloat("Restitution", &selectedObject->body->restitution, RESTITUTION_STEP, MIN_RESTITUTION, MAX_RESTITUTION);
            ImGui::End();
        }

        // Simulation settings window
        if (settingsOpen)
        {
            ImGui::Begin("Simulation Settings", &settingsOpen, propFlags);
            ImGui::DragFloat("Gravity", &world.gravity.y, GRAVITY_STEP, MIN_GRAVITY, MAX_GRAVITY);
            ImGui::DragFloat("Air Density", &world.airDensity, AIR_DENSITY_STEP, MIN_AIR_DENSITY, MAX_AIR_DENSITY);
            static const char *solverItems[] = {"Euler", "RK2", "RK4"};
            static int currentSolver = static_cast<int>(world.getODESolver());
            if (ImGui::Combo("ODE Solver", &currentSolver, solverItems, IM_ARRAYSIZE(solverItems)))
            {
                world.setODESolver(static_cast<SolverType>(currentSolver));
            }
            ImGui::End();
        }

        // Handle grabbed object position update (if static)
        if (grabbedObject != nullptr && grabbedObject->isStatic)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            Vec2 pixelsPos = Vec2(worldPos.x, worldPos.y);
            Vec2 metersPos = *pixelsToMeters(&pixelsPos);
            grabbedObject->body->position = metersPos;
            grabbedObject->body->velocity = Vec2(0.0f, 0.0f);
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
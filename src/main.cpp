#include "Config.h"

#include <iostream>
#include <fmt/format.h>
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

    Object *object1 = nullptr;
    Object *object2 = nullptr;
    Vec2 *anchor1 = nullptr;
    Vec2 *anchor2 = nullptr;

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

    // Mouse pos pointer for tools & force calculations
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
                                                                    Vec2 desiredVel = posDiff / (1.0f / DEFAULT_CALC_FREQ);
                                                                    Vec2 deltaV = desiredVel - state.velocity;
                                                                    Vec2 approxForce = deltaV * state.mass / (1.0f / DEFAULT_CALC_FREQ);
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
                            newCircle->body->liftCoefficient = circleSettings->liftCoefficient;
                            newCircle->body->frictionCoefficient = circleSettings->frictionCoefficient;
                            newCircle->body->restitution = circleSettings->restitution;

                            world.addObject(newCircle);

                            selectedObject = newCircle;
                        }
                        else if (type == DRAW_ROPE || type == DRAW_SPRING)
                        {
                            for (size_t i = 0; i < world.getObjects().size(); ++i)
                            {
                                Object *obj = world.getObjects()[i];
                                if (obj->isSelectable && obj->shape->getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseDown->position))))
                                {
                                    object1 = obj;
                                    anchor1 = new Vec2(0, 0);
                                    break;
                                }
                                object1 = nullptr;
                                anchor1 = new Vec2(metersPos.x, metersPos.y);
                            }
                        }
                        else if (type == ERASE)
                        {
                            bool found = false;
                            for (size_t i = 0; i < world.getObjects().size(); ++i)
                            {
                                Object *obj = world.getObjects()[i];
                                if (obj->isSelectable && obj->shape->getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseDown->position))))
                                {
                                    world.removeObject(i);
                                    if (selectedObject == obj)
                                        selectedObject = nullptr;
                                    found = true;
                                    break;
                                }
                            }

                            if (!found)
                            {
                                for (size_t i = 0; i < world.getConnectors().size(); ++i)
                                {
                                    Connector *conn = world.getConnectors()[i];
                                    sf::RectangleShape bbox = conn->getBoundingBox();
                                    if (bbox.getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseDown->position))))
                                    {
                                        world.removeConnector(i);
                                        break;
                                    }
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

                        // Handle rope/spring creation
                        ToolType type = tools.getCurrentTool()->type;
                        if (type == DRAW_ROPE || type == DRAW_SPRING)
                        {
                            if (anchor1 != nullptr)
                            {
                                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(mouseUp->position));
                                Vec2 pixelsPos = Vec2(mousePos.x, mousePos.y);
                                Vec2 metersPos = *pixelsToMeters(&pixelsPos);

                                for (size_t i = 0; i < world.getObjects().size(); ++i)
                                {
                                    Object *obj = world.getObjects()[i];
                                    if (obj->isSelectable && obj->shape->getGlobalBounds().contains(window.mapPixelToCoords(sf::Vector2i(mouseUp->position))))
                                    {
                                        object2 = obj;
                                        anchor2 = new Vec2(0, 0);
                                        break;
                                    }
                                    object2 = nullptr;
                                    anchor2 = new Vec2(metersPos.x, metersPos.y);
                                }

                                if (object1 != nullptr || object2 != nullptr)
                                {
                                    if (type == DRAW_ROPE)
                                    {
                                        RopeSettings *ropeSettings = static_cast<RopeSettings *>(tools.settings);

                                        float totalLength;
                                        if (object1 && object2)
                                        {
                                            Vec2 pos1 = object1->body->position + *anchor1;
                                            Vec2 pos2 = object2->body->position + *anchor2;
                                            totalLength = (pos2 - pos1).length();
                                        }
                                        else if (object1 && !object2)
                                        {
                                            Vec2 pos1 = object1->body->position + *anchor1;
                                            totalLength = (*anchor2 - pos1).length();
                                        }
                                        else if (!object1 && object2)
                                        {
                                            Vec2 pos2 = object2->body->position + *anchor2;
                                            totalLength = (*anchor1 - pos2).length();
                                        }
                                        else
                                        {
                                            totalLength = 0.0f;
                                        }
                                        totalLength *= ropeSettings->totalLength / 100.0f;

                                        Rope *newRope = new Rope(object1, *anchor1, object2, *anchor2, totalLength, ropeSettings->segmentCount, ropeSettings->segmentMass);
                                        world.addConnector(newRope);
                                    }
                                    else if (type == DRAW_SPRING)
                                    {
                                        SpringSettings *springSettings = static_cast<SpringSettings *>(tools.settings);

                                        float restLength;
                                        if (object1 && object2)
                                        {
                                            Vec2 pos1 = object1->body->position + *anchor1;
                                            Vec2 pos2 = object2->body->position + *anchor2;
                                            restLength = (pos2 - pos1).length();
                                        }
                                        else if (object1 && !object2)
                                        {
                                            Vec2 pos1 = object1->body->position + *anchor1;
                                            restLength = (*anchor2 - pos1).length();
                                        }
                                        else if (!object1 && object2)
                                        {
                                            Vec2 pos2 = object2->body->position + *anchor2;
                                            restLength = (*anchor1 - pos2).length();
                                        }
                                        else
                                        {
                                            restLength = 0.0f;
                                        }
                                        restLength *= springSettings->restingLength / 100.0f;

                                        Spring *newSpring = new Spring(object1, *anchor1, object2, *anchor2, springSettings->stiffness, springSettings->damping, restLength);
                                        world.addConnector(newSpring);
                                    }
                                }
                                delete anchor1;
                                delete anchor2;
                                object1 = nullptr;
                                object2 = nullptr;
                                anchor1 = nullptr;
                                anchor2 = nullptr;
                            }
                        }
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
        ImGui::Text(fmt::format("{} Tool", currentTool->getName()).c_str());
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
        else if (type == DRAW_CIRCLE)
        {
            ImGui::Text("Left Click to place object");
            ImGui::Separator();
            CircleSettings *circleSettings = static_cast<CircleSettings *>(tools.settings);
            ImGui::Checkbox("Is Static", &circleSettings->isStatic);
            ImGui::DragFloat("Radius (m)", &circleSettings->radius, LENGTH_STEP, MIN_LENGTH, MAX_LENGTH);
            ImGui::DragFloat("Density (kg/m²)", &circleSettings->density, DENSITY_STEP, MIN_DENSITY, MAX_DENSITY);
            ImGui::DragFloat("Drag Coefficient", &circleSettings->dragCoefficient, DRAG_STEP, MIN_DRAG, MAX_DRAG);
            ImGui::DragFloat("Lift Coefficient", &circleSettings->liftCoefficient, LIFT_STEP, MIN_LIFT, MAX_LIFT);
            ImGui::DragFloat("Friction Coefficient", &circleSettings->frictionCoefficient, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
            ImGui::DragFloat("Restitution", &circleSettings->restitution, RESTITUTION_STEP, MIN_RESTITUTION, MAX_RESTITUTION);
        }
        else if (type == DRAW_ROPE || type == DRAW_SPRING)
        {
            ImGui::Text("Left Click and drag to create connection");
            ImGui::Separator();
            if (type == DRAW_ROPE)
            {
                RopeSettings *ropeSettings = static_cast<RopeSettings *>(tools.settings);
                ImGui::DragFloat("Segment Count", &ropeSettings->segmentCount, SEGMENTS_STEP, MIN_SEGMENTS, MAX_SEGMENTS);
                ImGui::DragFloat("Segment Mass (kg)", &ropeSettings->segmentMass, SEGMENT_MASS_STEP, MIN_SEGMENT_MASS, MAX_SEGMENT_MASS);
                ImGui::DragFloat("Total Length (% of start)", &ropeSettings->totalLength, PERCENTAGE_STEP, MIN_PERCENTAGE, MAX_PERCENTAGE);
            }
            else if (type == DRAW_SPRING)
            {
                SpringSettings *springSettings = static_cast<SpringSettings *>(tools.settings);
                ImGui::DragFloat("Stiffness (N/m)", &springSettings->stiffness, STIFFNESS_STEP, MIN_STIFFNESS, MAX_STIFFNESS);
                ImGui::DragFloat("Damping (N·s/m)", &springSettings->damping, DAMPING_STEP, MIN_DAMPING, MAX_DAMPING);
                ImGui::DragFloat("Resting Length (% of start)", &springSettings->restingLength, PERCENTAGE_STEP, MIN_PERCENTAGE, MAX_PERCENTAGE);
            }
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
            ImGui::Text("Velocity: %s m/s", selectedObject->body->velocity.toString().c_str());
            ImGui::Text("Position: %s m", standardizePosition(selectedObject->body->position).toString().c_str());
            ImGui::Separator();
            ImGui::Text("Angular Velocity: %.2f rad/s", selectedObject->body->angularVelocity);
            ImGui::Text("Rotation: %.2f rad", selectedObject->body->rotation);
            ImGui::Separator();
            ImGui::Text("Momentum: %s kg·m/s", selectedObject->body->momentum.toString().c_str());
            ImGui::Text("Angular Momentum: %.2f kg·m²/s", selectedObject->body->angularMomentum);
            ImGui::Separator();
            ImGui::Text("Translational Energy: %.2f J", selectedObject->body->kineticEnergy);
            ImGui::Text("Rotational Energy: %.2f J", selectedObject->body->rotationalKineticEnergy);
            ImGui::Text("Gravitational Potential: %.2f J", selectedObject->body->gravitationalPotential);
            ImGui::Text("Spring Potential: %.2f J", selectedObject->body->springPotential);
            ImGui::Text("Total Mechanical Energy: %.2f J", selectedObject->body->totalEnergy);
            ImGui::Separator();
            ImGui::DragFloat("Drag Coefficient", &selectedObject->body->dragCoefficient, DRAG_STEP, MIN_DRAG, MAX_DRAG);
            ImGui::DragFloat("Friction Coefficient", &selectedObject->body->frictionCoefficient, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
            ImGui::DragFloat("Restitution", &selectedObject->body->restitution, RESTITUTION_STEP, MIN_RESTITUTION, MAX_RESTITUTION);
            ImGui::End();
        }

        // Simulation settings window
        if (settingsOpen)
        {
            ImGui::Begin("Simulation Settings", &settingsOpen, propFlags);
            ImGui::DragFloat("Gravity (m/s²)", &world.gravity.y, GRAVITY_STEP, MIN_GRAVITY, MAX_GRAVITY);
            ImGui::DragFloat("Air Density (kg/m²)", &world.airDensity, AIR_DENSITY_STEP, MIN_AIR_DENSITY, MAX_AIR_DENSITY);
            static const char *solverItems[] = {"Euler", "RK2", "RK4", "Verlet", "DOPRI5", "AB", "AM"};
            static int currentSolver = static_cast<int>(world.getODESolver());
            if (ImGui::Combo("ODE Solver", &currentSolver, solverItems, IM_ARRAYSIZE(solverItems)))
            {
                world.setODESolver(static_cast<SolverType>(currentSolver));
            }
            ImGui::DragFloat("Calculation Frequency (Hz)", &world.calculationFrequency, CALC_FREQ_STEP, MIN_CALC_FREQ, MAX_CALC_FREQ);
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
        if (!settingsOpen) world.update(dt);

        // Clear screen and draw world & ui
        window.clear(sf::Color::Black);
        world.draw(&window, type == DRAW_ROPE || type == DRAW_SPRING);
        if ((type == DRAW_ROPE || type == DRAW_SPRING) && (anchor1 != nullptr && anchor2 == nullptr))
        {
            Vec2 pos1 = object1 ? (object1->body->position + *anchor1) : *anchor1;
            Vec2 pos2 = *posPointer;

            Connector *tempConnector = nullptr;
            if (type == DRAW_ROPE)
            {
                RopeSettings *ropeSettings = static_cast<RopeSettings *>(tools.settings);

                float restLength = (pos2 - pos1).length();
                restLength *= ropeSettings->totalLength / 100.0f;

                tempConnector = new Rope(nullptr, pos1, nullptr, pos2, restLength, ropeSettings->segmentCount, 0.0f);
            }
            else if (type == DRAW_SPRING)
            {
                SpringSettings *springSettings = static_cast<SpringSettings *>(tools.settings);

                float restLength = (pos2 - pos1).length();
                restLength *= springSettings->restingLength / 100.0f;

                tempConnector = new Spring(nullptr, pos1, nullptr, pos2, springSettings->stiffness, springSettings->damping, restLength);
            }

            tempConnector->draw(&window);
            delete tempConnector;
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
#include "Config.hpp"

#include <iostream>
#include <fmt/format.h>
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include "core/World.hpp"
#include "core/Tools.hpp"
#include "core/UI.hpp"
#include "logging/Logger.hpp"
#include "graphing/Grapher.hpp"

// Helper for gridlines
void drawGridlines(sf::RenderWindow &window, float majorSpacing = GRID_MAJOR_SPACING, float minorSpacing = GRID_MINOR_SPACING)
{
    // Get current view bounds
    sf::View view = window.getView();
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();

    float left = viewCenter.x - viewSize.x / 2.0f;
    float right = viewCenter.x + viewSize.x / 2.0f;
    float top = viewCenter.y - viewSize.y / 2.0f;
    float bottom = viewCenter.y + viewSize.y / 2.0f;

    // Convert to meter spacing in pixels
    float minorSpacingPixels = metersToPixels(minorSpacing);
    float majorSpacingPixels = metersToPixels(majorSpacing);

    // Find starting positions aligned to grid
    float startXMinor = std::floor(left / minorSpacingPixels) * minorSpacingPixels;
    float startYMinor = std::floor(top / minorSpacingPixels) * minorSpacingPixels;

    const sf::Color &minorGridColor = GRID_MINOR_COLOR;
    const sf::Color &majorGridColor = GRID_MAJOR_COLOR;
    const sf::Color &axisColor = GRID_AXIS_COLOR;

    // Draw minor vertical lines (1m)
    for (float x = startXMinor; x <= right; x += minorSpacingPixels)
    {
        // Skip if this is a major gridline position
        float xMeters = x / metersToPixels(1.0f);
        bool isMajor = (std::abs(std::fmod(xMeters, majorSpacing)) < 0.01f);
        if (!isMajor)
        {
            sf::VertexArray line(sf::PrimitiveType::Lines, 2);
            line[0] = {{x, top}, minorGridColor};
            line[1] = {{x, bottom}, minorGridColor};
            window.draw(line);
        }
    }

    // Draw minor horizontal lines (1m)
    for (float y = startYMinor; y <= bottom; y += minorSpacingPixels)
    {
        // Skip if this is a major gridline position
        float yMeters = y / metersToPixels(1.0f);
        bool isMajor = (std::abs(std::fmod(yMeters, majorSpacing)) < 0.01f);
        if (!isMajor)
        {
            sf::VertexArray line(sf::PrimitiveType::Lines, 2);
            line[0] = {{left, y}, minorGridColor};
            line[1] = {{right, y}, minorGridColor};
            window.draw(line);
        }
    }

    // Draw major vertical lines (5m)
    float startXMajor = std::floor(left / majorSpacingPixels) * majorSpacingPixels;
    for (float x = startXMajor; x <= right; x += majorSpacingPixels)
    {
        bool isAxis = (std::abs(x) < 0.1f); // Check if this is the Y-axis
        sf::Color color = isAxis ? axisColor : majorGridColor;

        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0] = {{x, top}, color};
        line[1] = {{x, bottom}, color};
        window.draw(line);
    }

    // Draw major horizontal lines (5m)
    float startYMajor = std::floor(top / majorSpacingPixels) * majorSpacingPixels;
    for (float y = startYMajor; y <= bottom; y += majorSpacingPixels)
    {
        bool isAxis = (std::abs(y) < 0.1f); // Check if this is the X-axis
        sf::Color color = isAxis ? axisColor : majorGridColor;

        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0] = {{left, y}, color};
        line[1] = {{right, y}, color};
        window.draw(line);
    }
}

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

    // Fixed timestep accumulator for physics calculations
    float accumulator = 0.0f;

    // Initialize logger & grapher
    Logger logger(&world);
    Grapher grapher(&logger);

    // Create ground and walls
    world.reset();

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
                    if (grapher.isGraphOpen())
                    {
                        grapher.closeGraph();
                    }
                    else
                    {
                        settingsOpen = !settingsOpen;
                    }
                }
                else if (keyReleased->code == sf::Keyboard::Key::G)
                {
                    if (!settingsOpen)
                    {
                        grapher.toggleGraph();
                    }
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

                                        Rope *newRope = new Rope(object1, *anchor1, object2, *anchor2, totalLength);
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
        float frameTime = dtTime.asSeconds();
        if (frameTime > MAX_DT)
            frameTime = MAX_DT;

        // Update UI and tools
        ImGui::SFML::Update(window, dtTime);

        // FPS and Calculation Frequency
        float fps = frameTime > 0.0f ? 1.0f / frameTime : 0.0f;
        ImGui::SetNextWindowPos(ImVec2(window.getSize().x * 0.5f, 10), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        ImGui::Begin("Stats", nullptr, toolFlags);
        ImGui::Text("FPS: %.1f  |  Calc Freq: %.0f Hz", fps, world.calculationFrequency);
        ImGui::End();

        // Tools window
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::Begin("Tools", nullptr, toolFlags);
        tools.draw();
        ImVec2 toolsWindowSize = ImGui::GetWindowSize();
        ImGui::End();

        Tool *currentTool = tools.getCurrentTool();
        ToolType type = currentTool->type;

        // Tool settings window
        ImGui::SetNextWindowPos(ImVec2(10 + toolsWindowSize.x + 10, 10), ImGuiCond_Always);
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
            ImGui::SameLine();
            if (ImGui::Button("Graph##00"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), MASS);
            }

            ImGui::Separator();

            ImGui::Text("Position: %s m", standardizePosition(selectedObject->body->position).toString().c_str());
            ImGui::SameLine();
            if (ImGui::Button("Graph X##01"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), POSITION_X);
            }
            ImGui::SameLine();
            if (ImGui::Button("Graph Y##01"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), POSITION_Y);
            }

            ImGui::Text("Velocity: %s m/s", selectedObject->body->velocity.toString().c_str());
            ImGui::SameLine();
            if (ImGui::Button("Graph X##02"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), VELOCITY_X);
            }
            ImGui::SameLine();
            if (ImGui::Button("Graph Y##02"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), VELOCITY_Y);
            }

            ImGui::Separator();

            ImGui::Text("Rotation: %.2f rad", selectedObject->body->rotation);
            ImGui::SameLine();
            if (ImGui::Button("Graph##03"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), ROTATION);
            }

            ImGui::Text("Angular Velocity: %.2f rad/s", selectedObject->body->angularVelocity);
            ImGui::SameLine();
            if (ImGui::Button("Graph##04"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), ANGULAR_VELOCITY);
            }

            ImGui::Separator();

            ImGui::Text("Momentum: %s kg·m/s", selectedObject->body->momentum.toString().c_str());
            ImGui::SameLine();
            if (ImGui::Button("Graph X##05"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), MOMENTUM_X);
            }
            ImGui::SameLine();
            if (ImGui::Button("Graph Y##05"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), MOMENTUM_Y);
            }

            ImGui::Text("Angular Momentum: %.2f kg·m²/s", selectedObject->body->angularMomentum);
            ImGui::SameLine();
            if (ImGui::Button("Graph##06"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), ANGULAR_MOMENTUM);
            }

            ImGui::Separator();

            ImGui::Text("Translational Energy: %.2f J", selectedObject->body->kineticEnergy);
            ImGui::SameLine();
            if (ImGui::Button("Graph##07"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), KINETIC_ENERGY);
            }

            ImGui::Text("Rotational Energy: %.2f J", selectedObject->body->rotationalKineticEnergy);
            ImGui::SameLine();
            if (ImGui::Button("Graph##08"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), ROTATIONAL_KINETIC_ENERGY);
            }

            ImGui::Text("Gravitational Potential: %.2f J", selectedObject->body->gravitationalPotential);
            ImGui::SameLine();
            if (ImGui::Button("Graph##09"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), GRAVITATIONAL_POTENTIAL);
            }

            ImGui::Text("Spring Potential: %.2f J", selectedObject->body->springPotential);
            ImGui::SameLine();
            if (ImGui::Button("Graph##10"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), SPRING_POTENTIAL);
            }

            ImGui::Text("Total Mechanical Energy: %.2f J", selectedObject->body->totalEnergy);
            ImGui::SameLine();
            if (ImGui::Button("Graph##11"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), TOTAL_ENERGY);
            }

            ImGui::Separator();

            ImGui::DragFloat("Drag Coefficient", &selectedObject->body->dragCoefficient, DRAG_STEP, MIN_DRAG, MAX_DRAG);
            ImGui::SameLine();
            if (ImGui::Button("Graph##12"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), DRAG_COEFFICIENT);
            }

            ImGui::DragFloat("Lift Coefficient", &selectedObject->body->liftCoefficient, LIFT_STEP, MIN_LIFT, MAX_LIFT);
            ImGui::SameLine();
            if (ImGui::Button("Graph##13"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), LIFT_COEFFICIENT);
            }

            ImGui::DragFloat("Friction Coefficient", &selectedObject->body->frictionCoefficient, FRICTION_STEP, MIN_FRICTION, MAX_FRICTION);
            ImGui::SameLine();
            if (ImGui::Button("Graph##14"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), FRICTION_COEFFICIENT);
            }

            ImGui::DragFloat("Restitution", &selectedObject->body->restitution, RESTITUTION_STEP, MIN_RESTITUTION, MAX_RESTITUTION);
            ImGui::SameLine();
            if (ImGui::Button("Graph##15"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), RESTITUTION);
            }

            if (ImGui::Button("Test Graph 1"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), POSITION_Y);
                grapher.toggleProperty(std::to_string(selectedObject->getID()), VELOCITY_Y);
                grapher.openGraph();
            }
            ImGui::SameLine();
            if (ImGui::Button("Test Graph 2"))
            {
                grapher.toggleProperty(std::to_string(selectedObject->getID()), KINETIC_ENERGY);
                grapher.toggleProperty(std::to_string(selectedObject->getID()), GRAVITATIONAL_POTENTIAL);
                grapher.toggleProperty(std::to_string(selectedObject->getID()), SPRING_POTENTIAL);
                grapher.toggleProperty(std::to_string(selectedObject->getID()), TOTAL_ENERGY);
                grapher.openGraph();
            }

            if (ImGui::Button("Clear Graph"))
            {
                grapher.clearGraph();
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete Object"))
            {
                world.removeObject(selectedObject->getID());
                selectedObject = nullptr;
            }

            ImVec2 propWindowSize = ImGui::GetWindowSize();
            ImGui::SetNextWindowPos(ImVec2(window.getSize().x - propWindowSize.x - 10, 10), ImGuiCond_Once);
            ImGui::End();
        }

        // Simulation settings window
        if (settingsOpen)
        {
            // Position Simulation Settings window at center
            ImGui::Begin("Simulation Settings", &settingsOpen, propFlags | ImGuiWindowFlags_NoMove);
            ImGui::DragFloat("Gravity (m/s²)", &world.gravity.y, GRAVITY_STEP, MIN_GRAVITY, MAX_GRAVITY);
            ImGui::DragFloat("Air Density (kg/m²)", &world.airDensity, AIR_DENSITY_STEP, MIN_AIR_DENSITY, MAX_AIR_DENSITY);
            static const char *solverItems[] = {"Euler", "RK2", "RK4", "Verlet", "DOPRI5", "AB", "ABM"};
            static int currentSolver = static_cast<int>(world.getODESolver());
            if (ImGui::Combo("ODE Solver", &currentSolver, solverItems, IM_ARRAYSIZE(solverItems)))
            {
                world.setODESolver(static_cast<SolverType>(currentSolver));
            }
            ImGui::DragFloat("Calculation Frequency (Hz)", &world.calculationFrequency, CALC_FREQ_STEP, MIN_CALC_FREQ, MAX_CALC_FREQ);
            if (ImGui::Button("Save Logs"))
            {
                logger.saveAll();
            }
            ImGui::SameLine();
            if (ImGui::Button("Open Logs Folder"))
            {
                logger.openLogFolder();
            }
            if (ImGui::Button("Load Test Scene"))
            {
                selectedObject = nullptr;
                grabbedObject = nullptr;
                object1 = nullptr;
                object2 = nullptr;
                anchor1 = nullptr;
                anchor2 = nullptr;

                selectedObject = world.loadTestScene();
                logger.clearAll();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset Simulation"))
            {
                selectedObject = nullptr;
                grabbedObject = nullptr;
                object1 = nullptr;
                object2 = nullptr;
                anchor1 = nullptr;
                anchor2 = nullptr;

                world.reset();
                logger.clearAll();
            }
            ImVec2 settingWindowSize = ImGui::GetWindowSize();
            ImGui::SetWindowPos(ImVec2((window.getSize().x - settingWindowSize.x) * 0.5f, (window.getSize().y - settingWindowSize.y) * 0.5f), ImGuiCond_Always);
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

        // Update world and bodies with fixed timestep
        if (!settingsOpen && !grapher.isGraphOpen())
        {
            // Calculate fixed timestep from calculation frequency
            float fixedDt = 1.0f / world.calculationFrequency;

            // Add frame time to accumulator
            accumulator += frameTime;

            // Update physics in fixed timesteps
            while (accumulator >= fixedDt)
            {
                world.update(fixedDt);
                accumulator -= fixedDt;
            }

            logger.logWorld();
        }
        else
        {
            std::string pauseReason;
            if (settingsOpen && !grapher.isGraphOpen())
                pauseReason = "Settings Open";
            else if (!settingsOpen && grapher.isGraphOpen())
                pauseReason = "Graph Open";
            else
                pauseReason = "Settings and Graph Open"; // Will never happen but just in case

            std::string pauseText = "Paused: " + pauseReason;

            ImGui::Begin("Stats", nullptr, toolFlags);
            auto statsWidth = ImGui::GetWindowSize().x;
            auto pauseTextWidth = ImGui::CalcTextSize(pauseText.c_str()).x;
            ImGui::SetCursorPosX((statsWidth - pauseTextWidth) * 0.5f);
            ImGui::Text("%s", pauseText.c_str());
            ImGui::End();
        }

        // Clear screen and draw world & ui
        window.clear(BACKGROUND_COLOR);
        drawGridlines(window);
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

                tempConnector = new Rope(nullptr, pos1, nullptr, pos2, restLength);
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
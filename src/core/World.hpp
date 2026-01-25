#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "objects/Object.hpp"
#include "objects/Connector.hpp"

class World
{
private:
    std::vector<Object *> objects;         // List of objects in the world
    std::vector<Connector *> connectors;   // List of connectors (ropes, springs, etc.)
    SolverType odeSolver = DEFAULT_SOLVER; // Default ODE solver
    int nextObjectID = 0;                  // ID counter for objects
    int nextConnectorID = 0;               // ID counter for connectors

public:
    // World properties
    float calculationFrequency = DEFAULT_CALC_FREQ; // Frequency of physics calculations (Hz)
    Vec2 gravity = DEFAULT_GRAVITY;                 // Default gravity pointing downwards
    float airDensity = DEFAULT_AIR_DENSITY;         // Air density for drag calculations

    // World Variables
    float totalEnergy = 0.0f; // Total energy in the world

    // Constructors & Destructor
    World();
    ~World();

    // Methods
    void addObject(Object *object);  // Add an object to the world
    void removeObject(size_t index); // Remove an object from the world by index
    void clearObjects();             // Remove all objects from the world

    void addConnector(Connector *connector);    // Add a connector to the world
    void removeConnector(size_t index);         // Remove a connector from the world by index
    void clearConnectors();                     // Remove all connectors from the world

    void update(float dt);               // Update each object in the world based on forces and time step
    void draw(sf::RenderWindow *window); // Draw all objects in the world
    void draw(sf::RenderWindow *window, bool showAttachmentPoints);

    const std::vector<Object *> &getObjects() const; // Get the list of objects
    const std::vector<Connector *> &getConnectors() const; // Get the list of connectors

    void setODESolver(SolverType type); // Set the ODE solver type
    SolverType getODESolver() const;    // Get the current ODE solver type
};
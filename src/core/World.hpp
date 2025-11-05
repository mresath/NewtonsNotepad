#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "objects/Object.hpp"

class World
{
private:
    std::vector<Object *> objects;    // List of objects in the world
    SolverType odeSolver = DEFAULT_SOLVER;       // Default ODE solver
    int nextObjectID = 0;              // ID counter for objects

public:
    // World properties
    Vec2 gravity = DEFAULT_GRAVITY; // Default gravity pointing downwards
    float airDensity = DEFAULT_AIR_DENSITY;        // Air density for drag calculations

    // Constructors & Destructor
    World();
    ~World();

    // Methods
    void addObject(Object *object);  // Add an object to the world
    void removeObject(size_t index); // Remove an object from the world by index

    void update(float dt);               // Update each object in the world based on forces and time step
    void draw(sf::RenderWindow *window); // Draw all objects in the world

    const std::vector<Object *> &getObjects() const; // Get the list of objects

    void setODESolver(SolverType type); // Set the ODE solver type
    SolverType getODESolver() const;     // Get the current ODE solver type
};
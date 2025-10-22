#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "objects/Object.hpp"

class World
{
private:
    std::vector<Object *> objects;    // List of objects in the world
    Vec2 gravity = Vec2(0.0f, 9.81f); // Default gravity pointing downwards
    SolverType odeSolver = DEFAULT_SOLVER;       // Default ODE solver

public:
    // Constructors and Destructor
    World();
    World(const Vec2 &gravity);
    ~World();

    // Methods
    void addObject(Object *object);  // Add an object to the world
    void removeObject(size_t index); // Remove an object from the world by index

    void update(float dt);               // Update each object in the world based on forces and time step
    void draw(sf::RenderWindow *window); // Draw all objects in the world

    const std::vector<Object *> &getObjects() const; // Get the list of objects

    void setGravity(const Vec2 &newGravity); // Set the gravity vector
    Vec2 getGravity() const;                 // Get the current gravity vector

    void setODESolver(SolverType type); // Set the ODE solver type
};
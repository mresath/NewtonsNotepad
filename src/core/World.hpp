#pragma once

#include <vector>
#include "objects/Body.hpp"

class World
{
private:
    std::vector<Body *> bodies;       // List of bodies in the world
    Vec2 gravity = Vec2(0.0f, 9.81f); // Default gravity pointing downwards

public:
    // Constructors and Destructor
    World();
    World(const Vec2 &gravity);
    ~World();

    // Methods
    void addBody(Body *body);      // Add a body to the world
    void removeBody(size_t index); // Remove a body from the world by index

    void update(float dt); // Update each body in the world based on forces and time step

    const std::vector<Body *> &getBodies() const; // Get the list of bodies

    void setGravity(const Vec2 &newGravity); // Set the gravity vector
    Vec2 getGravity() const;                 // Get the current gravity vector
};
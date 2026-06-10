#pragma once

#include "objects/Object.hpp"

struct Connector
{
protected:
    int id;

    Object *objectA;
    Vec2 anchorA;
    Object *objectB;
    Vec2 anchorB;

public:
    Connector(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB);
    ~Connector();

    virtual ForceSource getForceSource(bool isObjectA) const;

    void connectForces() const;
    void removeForces() const;

    virtual void draw(sf::RenderWindow *window) const;

    virtual float getEnergy() const { return 0.0f; };
    virtual void applyEnergy() const { };

    sf::RectangleShape getBoundingBox() const;

    bool isConnectedTo(const Object *obj) const
    {
        return (obj == objectA || obj == objectB);
    }

    void setID(int newID);
    int getID() const;
    std::string getIDString() const;
};

struct Spring : Connector
{
public:
    float stiffness;
    float damping;
    float restingLength;

    Spring(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB, float stiff, float damp, float restLen);
    ~Spring()
    {
        Connector::~Connector();
    };

    ForceSource getForceSource(bool isObjectA) const override;

    void draw(sf::RenderWindow *window) const override;

    float getCurrentLength() const;
    float getExtension() const;
    float getEnergy() const override;
    void applyEnergy() const override;
};

struct Rope : Connector
{
public:
    float totalLength;
    float dampingFactor;  // Controls oscillation damping

    Rope(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB, float length);
    ~Rope()
    {
        Connector::~Connector();
    };

    ForceSource getForceSource(bool isObjectA) const override;

    void draw(sf::RenderWindow *window) const override;

    float getEnergy() const override { return 0.0f; };
    void applyEnergy() const override { };
};

struct Strut : Connector
{
public:
    float fixedLength;      // Desired length (can be variable or fixed)
    float stiffness;        // Constraint stiffness (moderate value)
    float dampingFactor;    // Oscillation damping

    Strut(Object *objA, Vec2 ancA, Object *objB, Vec2 ancB, float length);
    ~Strut()
    {
        Connector::~Connector();
    };

    ForceSource getForceSource(bool isObjectA) const override;

    void draw(sf::RenderWindow *window) const override;

    float getCurrentLength() const;
    float getEnergy() const override { return 0.0f; };
    void applyEnergy() const override { };
};
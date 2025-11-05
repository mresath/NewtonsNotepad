#pragma once

#include <functional>
#include "math/Vec2.hpp"
#include "objects/Object.hpp"

struct Force
{
    Vec2 position;
    Vec2 force;

    Force() : position(0.0f, 0.0f), force(0.0f, 0.0f) {}

    Force(const Vec2 &pos, const Vec2 &f) : position(pos), force(f) {}
};

class ForceSource
{
private:
    bool isConstant = true;
    Force constantForce;
    std::function<Force(const Body &state)> variableForceFunc = nullptr;
    

public:
    std::string name;

    ForceSource(const std::string &name) : name(name), constantForce(Force()) {}
    ForceSource(const std::string &name, const Force &force) : name(name), constantForce(force) {}
    ForceSource(const std::string &name, const std::function<Force(const Body &state)> func) : name(name), variableForceFunc(func)
    {
        isConstant = false;
    }

    Force calculateForce(const Body &state) const
    {
        if (isConstant)
        {
            return constantForce;
        }
        else if (variableForceFunc != nullptr)
        {
            return variableForceFunc(state);
        }
        return Force();
    }

    void setForce(const Force &force)
    {
        constantForce = force;
        isConstant = true;
        variableForceFunc = nullptr;
    }

    void setForce(Force (*func)(const Body &state))
    {
        variableForceFunc = func;
        isConstant = false;
    }
};
#pragma once

#include <functional>
#include <tuple>
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

    std::tuple<Force, float> calculateForce(const Body &state) const
    {
        Force netForce;

        if (isConstant)
        {
            netForce = constantForce;
        }
        else if (variableForceFunc != nullptr)
        {
            netForce = variableForceFunc(state);
        }

        Vec2 dist = netForce.position - Vec2(0.0f, 0.0f);

        if (dist.lengthSquared() == 0.0f)
        {
            return std::make_tuple(netForce, 0.0f);
        }

        Vec2 radialNormal = dist.normalized();

        Vec2 radialForce = radialNormal * dot(netForce.force, radialNormal);
        Vec2 tangentialForce = netForce.force - radialForce;

        return std::make_tuple(Force(Vec2(0.0f, 0.0f), radialForce), dist.length() * tangentialForce.length());
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
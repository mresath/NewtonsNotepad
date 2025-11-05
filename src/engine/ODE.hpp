#pragma once

#include <string>
#include "objects/Object.hpp"

class Object;

enum SolverType : unsigned short
{
    EULER,
    RK2,
    RK4,
};

inline std::string solverTypeToString(SolverType type)
{
    switch (type)
    {
    case EULER:
        return "Euler";
    case RK2:
        return "RK2";
    case RK4:
        return "RK4";
    default:
        return "Unknown";
    }
}

class ODESolver
{
protected:
    Object *object;

public:
    ODESolver(Object *initialState) : object(initialState) {}
    virtual void step(float dt) = 0;
    virtual Body simulate(float dt) = 0;
    virtual ~ODESolver() = default;
};

class EulerSolver : public ODESolver
{
public:
    EulerSolver(Object *initialState) : ODESolver(initialState) {}
    void step(float dt) override;
    Body simulate(float dt) override;
};

class RK2Solver : public ODESolver
{
public:
    RK2Solver(Object *initialState) : ODESolver(initialState) {}
    void step(float dt) override;
    Body simulate(float dt) override;
};

class RK4Solver : public ODESolver
{
public:
    RK4Solver(Object *initialState) : ODESolver(initialState) {}
    void step(float dt) override;
    Body simulate(float dt) override;
};
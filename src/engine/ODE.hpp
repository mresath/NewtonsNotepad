#pragma once

#include <string>
#include "objects/Object.hpp"

class Object;

enum SolverType : unsigned short
{
    EULER,
    RK2,
    RK4,
    VERLET,
    DOPRI5,
    AB,
    AM
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
    case DOPRI5:
        return "DOPRI5";
    case VERLET:
        return "Verlet";
    case AB:
        return "AB";
    case AM:
        return "AM";
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

class VerletSolver : public ODESolver
{
private:
    Vec2 previousPosition;
    bool isFirstStep = true;
public:
    VerletSolver(Object *initialState) : ODESolver(initialState) {}
    void step(float dt) override;
    Body simulate(float dt) override;
};

class DOPRI5Solver : public ODESolver
{
public:
    DOPRI5Solver(Object *initialState) : ODESolver(initialState) {}
    void step(float dt) override;
    Body simulate(float dt) override;
};

class ABSolver : public ODESolver
{
private:
    std::vector<Body> previousStates;
public:
    ABSolver(Object *initialState) : ODESolver(initialState) {
        previousStates.reserve(5);
    }
    void step(float dt) override;
    Body simulate(float dt) override;
};

class AMSolver : public ODESolver
{
private:
    std::vector<Body> previousStates;
public:
    AMSolver(Object *initialState) : ODESolver(initialState) {
        previousStates.reserve(5);
    }
    void step(float dt) override;
    Body simulate(float dt) override;
};
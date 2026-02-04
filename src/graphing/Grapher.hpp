#pragma once

#include "logging/Logger.hpp"
#include <matplot/matplot.h>

class Grapher
{
private:
    Logger *logger;

public:
    Grapher(Logger *logger) : logger(logger) {}
};
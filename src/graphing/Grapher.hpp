#pragma once

#include "logging/Logger.hpp"
#include <matplot/matplot.h>

class Grapher
{
private:
    Logger *logger;
    std::map<std::string, std::map<std::string, std::map<double, double>>> graphData; // objectId -> property -> time -> value
    std::string graphFolder = "graphs/";

public:
    Grapher(Logger *logger) : logger(logger) {}
    ~Grapher();

    void addToGraph(std::string objectId, std::string property);

    void openGraph();
    void closeGraph();

    void saveGraph();

    void update();
};
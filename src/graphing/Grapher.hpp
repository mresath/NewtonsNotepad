#pragma once

#include "logging/Logger.hpp"
#include <matplot/matplot.h>

class Grapher
{
private:
    Logger *logger;
    ToGraph toGraph; // objectId -> properties
    std::string graphFolder = "graphs/";
    
    matplot::figure_handle fig{nullptr};

public:
    Grapher(Logger *logger) : logger(logger) {}
    ~Grapher();

    void addToGraph(std::string objectId, BodyKeys property);
    void removeFromGraph(std::string objectId, BodyKeys property);
    void toggleProperty(std::string objectId, BodyKeys property);
    void clearGraph();

    void openGraph();
    void closeGraph();
    void toggleGraph();
    bool isGraphOpen() const;
};
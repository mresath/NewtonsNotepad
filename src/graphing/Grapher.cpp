#include "Grapher.hpp"

Grapher::~Grapher()
{
    closeGraph();
}

void Grapher::addToGraph(std::string objectId, BodyKeys property)
{
    // If objectId is not in toGraph, add it with an empty vector
    if (toGraph.find(objectId) == toGraph.end())
    {
        toGraph[objectId] = std::vector<BodyKeys>();
    }
    // Add property to the vector if it's not already there
    auto &properties = toGraph[objectId];
    if (std::find(properties.begin(), properties.end(), property) == properties.end())
    {
        properties.push_back(property);
    }
}

void Grapher::removeFromGraph(std::string objectId, BodyKeys property)
{
    // Check if objectId is in toGraph
    if (toGraph.find(objectId) != toGraph.end())
    {
        auto &properties = toGraph[objectId];
        // Remove property from the vector
        properties.erase(std::remove(properties.begin(), properties.end(), property), properties.end());
        // If the vector is empty, remove the objectId from toGraph
        if (properties.empty())
        {
            toGraph.erase(objectId);
        }
    }
}

void Grapher::toggleProperty(std::string objectId, BodyKeys property)
{
    // Check if objectId is in toGraph
    bool found = false;
    if (toGraph.find(objectId) != toGraph.end())
    {
        auto &properties = toGraph[objectId];
        // If property is in the vector, remove it; otherwise, add it
        if (std::find(properties.begin(), properties.end(), property) != properties.end())
        {
            removeFromGraph(objectId, property);
            found = true;
        }
    }

    if (!found)
    {
        addToGraph(objectId, property);
    }
}

void Grapher::clearGraph()
{
    toGraph.clear();
}

void Grapher::openGraph()
{
    using namespace matplot;

    // Close existing graph if open
    closeGraph();

    // Return early if nothing to graph
    if (toGraph.empty())
    {
        return;
    }

    // Get graphable data from logger
    Graphable graphData = logger->getGraphable(toGraph);

    // Create new figure
    fig = figure(true);
    fig->quiet_mode(false); // Interactive mode

    // Prepare time data (x-axis)
    std::vector<float> times;
    for (const auto &[time, _] : graphData)
    {
        times.push_back(static_cast<float>(time));
    }

    // Plot each object and property
    int plotIndex = 0;
    for (const auto &[objectId, properties] : toGraph)
    {
        for (const auto &property : properties)
        {
            std::vector<float> values;

            // Extract values for this object and property over time
            for (const auto &[time, objects] : graphData)
            {
                if (objects.find(objectId) != objects.end() &&
                    objects.at(objectId).find(property) != objects.at(objectId).end())
                {
                    values.push_back(static_cast<float>(objects.at(objectId).at(property)));
                }
                else
                {
                    values.push_back(std::nan(""));
                }
            }

            // Only plot if we have data
            if (!times.empty() && !values.empty())
            {
                hold(on);
                auto line = plot(times, values);

                std::string objectLabel = "Object " + objectId;
                std::string unit = BodyKeyUnit(property);
                std::string unitLabel = unit.empty() ? "" : " (" + unit + ")";

                line->display_name(objectLabel + " - " + BodyKeyName(property) + unitLabel);
                plotIndex++;
            }
        }
    }

    if (plotIndex > 0)
    {
        xlabel("Time (s)");
        ylabel("Value");
        title("Graph");
        legend();
        grid(on);
        fig->draw();
    }
    else
    {
        // No data to plot, clean up the figure
        fig = nullptr;
    }
}

void Grapher::closeGraph()
{
    if (fig)
    {
        // Send exit command to gnuplot backend before destroying
        auto backend_ptr = std::dynamic_pointer_cast<matplot::backend::gnuplot>(fig->backend());
        if (backend_ptr)
        {
            backend_ptr->run_command("exit");
            backend_ptr->flush_commands();
        }
        fig = nullptr;
    }
}

bool Grapher::isGraphOpen() const
{
    // For gnuplot backend, we can't reliably detect window closure
    // Just check if we have a figure object
    return fig != nullptr;
}
void Grapher::toggleGraph()
{
    if (isGraphOpen())
    {
        closeGraph();
    }
    else
    {
        openGraph();
    }
}
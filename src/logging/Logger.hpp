#pragma once

#include "core/World.hpp"
#include <string>
#include <map>
#include <vector>
#include <fstream>

using WorldState = std::map<std::string, Body>;
using JSONLog = std::map<double, WorldState>;

enum LogLevel
{
    INFO,
    WARNING,
    ERROR
};

class Logger
{
private:
    World *world;
    std::string logFolder = "logs/";
    std::string logFilename = "log.txt";
    std::string jsonFilename = "world_log.json";

    std::vector<std::string> logMessages;
    JSONLog jsonLog;
public:
    Logger(World *world) : world(world) {
        if (!std::filesystem::exists(logFolder)) {
            std::filesystem::create_directory(logFolder);
        }
    };
    ~Logger();

    /* MESSAGE LOGGING */

    void logMessage(const std::string &message, LogLevel level = INFO);

    void clearLog();
    
    void setLogFile(const std::string &filename);
    

    void saveLog();

    /* WORLD STATE LOGGING */

    void logWorld(); // Logs the current state of the world in JSON format with time as the key

    void clearJSONLog();

    void setJSONFile(const std::string &filename);

    void saveJSON();

    /* GENERAL METHODS */
    void openLogFolder();

    void clearAll();
    
    void saveAll();
};
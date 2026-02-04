#include "Logger.h"

Logger::~Logger()
{
    saveAll();
}

/* MESSAGE LOGGING */

void Logger::logMessage(const std::string &message, LogLevel level)
{
    std::string prefix;
    switch (level)
    {
    case INFO:
        prefix = "[INFO] ";
        break;
    case WARNING:
        prefix = "[WARNING] ";
        break;
    case ERROR:
        prefix = "[ERROR] ";
        break;
    }
    logMessages.push_back(prefix + message);
}

void Logger::setLogFile(const std::string &filename)
{
    logFilename = filename;
}

void Logger::clearLog()
{
    logMessages.clear();
}

void Logger::saveLog()
{
    std::ofstream logFile(logFolder + logFilename, std::ios::out);
    if (logFile.is_open())
    {
        for (const auto &message : logMessages)
        {
            logFile << message << std::endl;
        }
        logFile.close();
    }
}

/* JSON LOGGING */

void Logger::logWorld()
{
    double currentTime = world->getTime();
    WorldState state;
    for (const auto &obj : world->getObjects())
    {
        if (obj->isStatic) continue;
        state.insert({ std::to_string(obj->getID()), *(obj->body) });
    }
    jsonLog[currentTime] = state;
}

void Logger::setJSONFile(const std::string &filename)
{
    jsonFilename = filename;
}

void Logger::clearJSONLog()
{
    jsonLog.clear();
}

void Logger::saveJSON() {
    nlohmann::json j;
    for (const auto &entry : jsonLog)
    {
        for (const auto &objState : entry.second)
        {
            j[std::to_string(entry.first)][objState.first] = objState.second.to_json();
        }
    }
    std::ofstream jsonFile(logFolder + jsonFilename);
    if (jsonFile.is_open())
    {
        jsonFile << j.dump(4); // Pretty print with 4 spaces indentation
        jsonFile.close();
    }
}

/* GENERAL METHODS */
void Logger::openLogFolder()
{
#ifdef _WIN32
    std::string command = "explorer " + logFolder;
    system(command.c_str());
#elif __APPLE__
    std::string command = "open " + logFolder;
    system(command.c_str());
#elif __linux__
    std::string command = "xdg-open " + logFolder;
    system(command.c_str());
#endif
}

void Logger::clearAll()
{
    clearLog();
    clearJSONLog();
}

void Logger::saveAll()
{
    saveLog();
    saveJSON();
}
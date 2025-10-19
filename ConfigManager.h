#pragma once
#include <string>

// Gets the config filename (e.g., MyDLL.json)
std::string GetConfigFilename();

// Gets the log filename (e.g., my_script_log.txt)
std::string GetLogFilename();

// Get the log level from config (0=ERROR, 1=WARNING, 2=INFO, 3=DEBUG)
int GetLogLevel();

#pragma once
#include <string>

// Prepares the log file for writing (clears any old content)
void InitLogger(const std::string& filename);

// Writes a message to the log file
void Log(const std::string& message);

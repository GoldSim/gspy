#pragma once
#include <string>

// Define logging levels
enum LogLevel {
    LOG_ERROR = 0,
    LOG_WARNING = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3
};

// Set the current logging level (default is LOG_INFO for production)
extern LogLevel current_log_level;

// Prepares the log file for writing (clears any old content)
void InitLogger(const std::string& filename, LogLevel level = LOG_INFO);

// Writes a message to the log file with specified level
void Log(const std::string& message, LogLevel level = LOG_INFO);

// Convenience functions for different log levels
void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogDebug(const std::string& message);

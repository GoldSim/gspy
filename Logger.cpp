#include "Logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>

// This static variable holds the connection to our log file
static std::ofstream log_file;

// Current logging level
LogLevel current_log_level = LOG_INFO;

void InitLogger(const std::string& filename, LogLevel level) {
    current_log_level = level;
    // Open the file, clearing its contents for a fresh log each run
    if (!log_file.is_open()) {
        log_file.open(filename, std::ios::out | std::ios::trunc);
    }
}

void Log(const std::string& message, LogLevel level) {
    // Skip logging if the message level is higher than current level
    if (level > current_log_level) {
        return;
    }
    
    if (log_file.is_open()) {
        // Get the current time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        tm buf;
        localtime_s(&buf, &in_time_t);

        // Write the timestamp and message to the file
        // Use '\n' instead of std::endl to avoid unnecessary flushing
        log_file << std::put_time(&buf, "%Y-%m-%d %X") << " - " << message << '\n';
    }
}

// Convenience functions
void LogError(const std::string& message) {
    Log("ERROR: " + message, LOG_ERROR);
}

void LogWarning(const std::string& message) {
    Log("WARNING: " + message, LOG_WARNING);
}

void LogInfo(const std::string& message) {
    Log("INFO: " + message, LOG_INFO);
}

void LogDebug(const std::string& message) {
    Log("DEBUG: " + message, LOG_DEBUG);
}
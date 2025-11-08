#pragma once
#include <string>
#include <atomic>
#include <mutex>

// Define logging levels
enum LogLevel {
    LOG_ERROR = 0,
    LOG_WARNING = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3
};

// Set the current logging level (default is LOG_INFO for production)
extern LogLevel current_log_level;

// Atomic log level for thread-safe access
extern std::atomic<int> current_log_level_atomic;

// Mutex for thread-safe file operations
extern std::mutex log_mutex;

// Flag to indicate if we're using stderr fallback
extern bool fallback_to_stderr;

// Set log level from integer with validation
void SetLogLevelFromInt(int level);

// Fast-path filtering function for performance optimization
inline bool ShouldLog(LogLevel level);

// Prepares the log file for writing (clears any old content)
void InitLogger(const std::string& filename, LogLevel level = LOG_INFO);

// Writes the intro header to the log file
void WriteLogHeader();

// Writes a message to the log file with specified level
void Log(const std::string& message, LogLevel level = LOG_INFO);

// Convenience functions for different log levels
void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);
void LogDebug(const std::string& message);

// Logs a message regardless of log_level setting (bypasses filtering)
// Used for critical diagnostic information that must always be written
void LogAlways(const std::string& message);

#include "Logger.h"
#include "GSPy.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <atomic>
#include <mutex>
#include <iostream>

// This static variable holds the connection to our log file
static std::ofstream log_file;

// Current logging level
LogLevel current_log_level = LOG_INFO;

// Atomic log level for thread-safe access (default LOG_INFO)
std::atomic<int> current_log_level_atomic{LOG_INFO};

// Mutex for thread-safe file operations
std::mutex log_mutex;

// Flag to indicate if we're using stderr fallback
bool fallback_to_stderr = false;

void SetLogLevelFromInt(int level) {
    // Validate the log level is within acceptable range
    if (level >= LOG_ERROR && level <= LOG_DEBUG) {
        current_log_level_atomic.store(level, std::memory_order_relaxed);
        current_log_level = static_cast<LogLevel>(level);
    }
    // If invalid level provided, keep current setting (no error thrown)
}

void InitLogger(const std::string& filename, LogLevel level) {
    current_log_level = level;
    current_log_level_atomic.store(level, std::memory_order_relaxed);
    
    // Reset fallback flag for new initialization
    fallback_to_stderr = false;
    
    // Open the file, clearing its contents for a fresh log each run
    if (!log_file.is_open()) {
        log_file.open(filename, std::ios::out | std::ios::trunc);
        
        // Detect file open failures and enable fallback handling
        if (!log_file.is_open() || log_file.fail()) {
            // Write single warning to stderr when file operations fail
            std::cerr << "WARNING: Failed to open log file '" << filename 
                      << "'. Redirecting all logging to stderr." << std::endl;
            
            // Redirect all subsequent logging to stderr without exceptions
            fallback_to_stderr = true;
            
            // Close the failed file stream to clean up
            if (log_file.is_open()) {
                log_file.close();
            }
        } else {
            // Write intro header to the log file
            WriteLogHeader();
        }
    }
}

void WriteLogHeader() {
    if (fallback_to_stderr) {
        std::cerr << "========================================\n";
        std::cerr << "GSPy: The GoldSim-Python Bridge\n";
        std::cerr << "Version: " << GSPY_VERSION << "\n";
        std::cerr << "Build Date: " << __DATE__ << " " << __TIME__ << "\n";
        std::cerr << "========================================\n\n";
        std::cerr.flush();
    } else if (log_file.is_open()) {
        log_file << "========================================\n";
        log_file << "GSPy: The GoldSim-Python Bridge\n";
        log_file << "Version: " << GSPY_VERSION << "\n";
        log_file << "Build Date: " << __DATE__ << " " << __TIME__ << "\n";
        log_file << "========================================\n\n";
        log_file.flush();
    }
}

inline bool ShouldLog(LogLevel level) {
    // Fast-path filtering using atomic read with memory_order_relaxed for performance
    return level <= current_log_level_atomic.load(std::memory_order_relaxed);
}

void Log(const std::string& message, LogLevel level) {
    // Fast-path filtering - early return if logging is disabled for this level
    if (!ShouldLog(level)) {
        return;
    }
    
    // Thread-safe operations using mutex - maintain thread safety during fallback operations
    std::lock_guard<std::mutex> lock(log_mutex);
    
    // Get the current time for timestamp
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    tm buf;
    localtime_s(&buf, &in_time_t);
    
    if (fallback_to_stderr) {
        // Redirect all subsequent logging to stderr without exceptions
        std::cerr << std::put_time(&buf, "%Y-%m-%d %X") << " - " << message << std::endl;
        
        // Flush immediately for ERROR and WARNING levels even in stderr mode
        if (level <= LOG_WARNING) {
            std::cerr.flush();
        }
    } else if (log_file.is_open()) {
        // Write the timestamp and message to the file
        // Use '\n' instead of std::endl to avoid unnecessary flushing
        log_file << std::put_time(&buf, "%Y-%m-%d %X") << " - " << message << '\n';
        
        // Hybrid flush policy: flush immediately for ERROR and WARNING levels
        // Use write-only behavior (no flush) for INFO and DEBUG levels
        if (level <= LOG_WARNING) {
            log_file.flush();
        }
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
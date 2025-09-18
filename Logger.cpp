#include "Logger.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>

// This static variable holds the connection to our log file
static std::ofstream log_file;

void InitLogger(const std::string& filename) {
    // Open the file, clearing its contents for a fresh log each run
    if (!log_file.is_open()) {
        log_file.open(filename, std::ios::out | std::ios::trunc);
    }
}

void Log(const std::string& message) {
    if (log_file.is_open()) {
        // Get the current time
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        tm buf;
        localtime_s(&buf, &in_time_t);

        // Write the timestamp and message to the file
        log_file << std::put_time(&buf, "%Y-%m-%d %X") << " - " << message << std::endl;
    }
}
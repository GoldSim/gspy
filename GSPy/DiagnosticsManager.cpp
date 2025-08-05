#include "DiagnosticsManager.h"
#include <fstream>
#include <iostream>

DiagnosticsManager& DiagnosticsManager::Instance() {
    static DiagnosticsManager instance;
    return instance;
}

DiagnosticsManager::DiagnosticsManager() 
    : logLevel_(LogLevel::LOG_ERROR), logFile_("GSPyDiagnostics.txt") {
}

DiagnosticsManager::~DiagnosticsManager() {
}

void DiagnosticsManager::SetLogLevel(LogLevel level) {
    logLevel_ = level;
}

void DiagnosticsManager::LogError(const std::string& msg) {
    if (static_cast<int>(logLevel_) <= static_cast<int>(LogLevel::LOG_ERROR)) {
        WriteLog("[ERROR] " + msg);
    }
}

void DiagnosticsManager::LogDebug(const std::string& msg) {
    if (static_cast<int>(logLevel_) <= static_cast<int>(LogLevel::DEBUG)) {
        WriteLog("[DEBUG] " + msg);
    }
}

void DiagnosticsManager::LogInfo(const std::string& msg) {
    if (static_cast<int>(logLevel_) <= static_cast<int>(LogLevel::INFO)) {
        WriteLog("[INFO] " + msg);
    }
}

void DiagnosticsManager::SetLogFile(const std::string& filename) {
    logFile_ = filename;
}

void DiagnosticsManager::WriteLog(const std::string& msg) {
    std::ofstream ofs(logFile_, std::ios::app);
    if (ofs.is_open()) {
        ofs << msg << std::endl;
    } else {
        std::cerr << "DiagnosticsManager: Failed to open log file: " << logFile_ << std::endl;
    }
}
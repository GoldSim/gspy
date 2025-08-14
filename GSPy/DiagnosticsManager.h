#pragma once

#include <string>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    LOG_ERROR
};

class DiagnosticsManager {
public:
    static DiagnosticsManager& Instance();
    
    void SetLogLevel(LogLevel level);
    void LogError(const std::string& msg);
    void LogDebug(const std::string& msg);
    void LogInfo(const std::string& msg);
    void SetLogFile(const std::string& filename);
    void Clear();
    std::string GetLastError() const;

private:
    DiagnosticsManager();
    ~DiagnosticsManager();
    DiagnosticsManager(const DiagnosticsManager&) = delete;
    DiagnosticsManager& operator=(const DiagnosticsManager&) = delete;
    
    void WriteLog(const std::string& msg);
    
    LogLevel logLevel_;
    std::string logFile_;
    std::string lastError_;
};
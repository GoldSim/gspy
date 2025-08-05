#pragma once

#include <mutex>

class PythonEnvManager {
public:
    static PythonEnvManager& Instance();
    
    void Initialize();
    void Finalize();
    bool IsInitialized() const;

private:
    PythonEnvManager();
    ~PythonEnvManager();
    PythonEnvManager(const PythonEnvManager&) = delete;
    PythonEnvManager& operator=(const PythonEnvManager&) = delete;
    
    bool initialized_;
    mutable std::mutex mutex_;
};
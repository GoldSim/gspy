#include "PythonEnvManager.h"
#include <Python.h>
#include <mutex>

PythonEnvManager& PythonEnvManager::Instance() {
    static PythonEnvManager instance;
    return instance;
}

PythonEnvManager::PythonEnvManager() : initialized_(false) {
}

PythonEnvManager::~PythonEnvManager() {
}

void PythonEnvManager::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        Py_InitializeEx(0); // 0 = don't install signal handlers
        initialized_ = true;
    }
}

void PythonEnvManager::Finalize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        Py_FinalizeEx();
        initialized_ = false;
    }
}

bool PythonEnvManager::IsInitialized() const {
    return initialized_;
}
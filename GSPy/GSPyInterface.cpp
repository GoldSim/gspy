#include "common.h"

#include <windows.h> // For the DLL entry point
#include <fstream>
#include <string>
#include <sstream>
#include <vector> // For std::vector
#include <filesystem>
#include "GSPyInterface.h"
#include "DiagnosticsManager.h"
#include "DataMarshaller.h"
#include "PythonEnvManager.h"
#include "UserScriptAdapter.h"

// Define the method IDs for clarity
enum XFMethodID {
    XF_INITIALIZE = 0, 
    XF_CALCULATE = 1, 
    XF_REP_VERSION = 2, 
    XF_REP_ARGUMENTS = 3,
    XF_CLEANUP = 99
};

// Dynamic I/O counts - determined by calling gspy_info() function in the Python script
static int g_numInputs = 1;   // Default values, will be updated during initialization
static int g_numOutputs = 1;

// Remove old LogMessage function - now using DiagnosticsManager

// Forward declaration removed - using bool version only

// Global UserScriptAdapter instance
static UserScriptAdapter g_scriptAdapter;

// Test interface functions
extern "C" bool __declspec(dllexport) gs_Initialize(const char* scriptPath, char* errorMsg, int errorMsgSize) {
    try {
        DiagnosticsManager::Instance().SetLogFile("gspy_log.txt");
        DiagnosticsManager::Instance().SetLogLevel(LogLevel::DEBUG);
        DiagnosticsManager::Instance().LogInfo("=== GSPy Test Initialize ===");
        
        if (!PythonEnvManager::Instance().IsInitialized()) {
            PythonEnvManager::Instance().Initialize();
            if (!PythonEnvManager::Instance().IsInitialized()) {
                strncpy_s(errorMsg, errorMsgSize, "Python initialization failed", errorMsgSize - 1);
                return false;
            }
        }
        
        errorMsg[0] = '\0';
        return true;
    } catch (const std::exception& e) {
        strncpy_s(errorMsg, errorMsgSize, e.what(), errorMsgSize - 1);
        return false;
    }
}

extern "C" bool __declspec(dllexport) gs_SetScriptPath(const char* scriptPath, char* errorMsg, int errorMsgSize) {
    try {
        std::string scriptName = std::filesystem::path(scriptPath).stem().string();
        if (!g_scriptAdapter.LoadScript(scriptName)) {
            strncpy_s(errorMsg, errorMsgSize, "Failed to load script", errorMsgSize - 1);
            return false;
        }
        
        errorMsg[0] = '\0';
        return true;
    } catch (const std::exception& e) {
        strncpy_s(errorMsg, errorMsgSize, e.what(), errorMsgSize - 1);
        return false;
    }
}

extern "C" bool __declspec(dllexport) gs_DoCalculation(gs_InputStruct* inputs, gs_OutputStruct* outputs) {
    try {
        if (!g_scriptAdapter.HasGoldSimCalculate()) {
            strncpy_s(outputs->errorMsg, sizeof(outputs->errorMsg), "Script not loaded", sizeof(outputs->errorMsg) - 1);
            return false;
        }
        
        DataMarshaller marshaller;
        double inargs[1] = { inputs->scalar };
        PyObject* inputDict = marshaller.PackArguments(inargs, 1);
        if (!inputDict) {
            strncpy_s(outputs->errorMsg, sizeof(outputs->errorMsg), "Failed to pack arguments", sizeof(outputs->errorMsg) - 1);
            return false;
        }
        
        std::string errorMsg;
        PyObject* result = g_scriptAdapter.CallGoldSimCalculate(inputDict, errorMsg);
        Py_DECREF(inputDict);
        
        if (!result) {
            strncpy_s(outputs->errorMsg, sizeof(outputs->errorMsg), errorMsg.c_str(), sizeof(outputs->errorMsg) - 1);
            return false;
        }
        
        double outargs[1];
        if (!marshaller.UnpackResult(result, outargs, 1)) {
            Py_DECREF(result);
            strncpy_s(outputs->errorMsg, sizeof(outputs->errorMsg), "Failed to unpack result", sizeof(outputs->errorMsg) - 1);
            return false;
        }
        
        outputs->scalar = outargs[0];
        outputs->errorMsg[0] = '\0';
        Py_DECREF(result);
        return true;
    } catch (const std::exception& e) {
        strncpy_s(outputs->errorMsg, sizeof(outputs->errorMsg), e.what(), sizeof(outputs->errorMsg) - 1);
        return false;
    }
}

extern "C" void __declspec(dllexport) gs_Finalize() {
    DiagnosticsManager::Instance().LogInfo("=== GSPy Test Finalize ===");
}

// I/O metadata functions
extern "C" int __declspec(dllexport) gs_GetNumberOfInputs() {
    return g_numInputs;
}

extern "C" bool __declspec(dllexport) gs_GetInputName(int idx, char* buffer, int buflen) {
    if (idx == 0) {
        strncpy_s(buffer, buflen, "input", buflen - 1);
        return true;
    }
    return false;
}

extern "C" int __declspec(dllexport) gs_GetInputDataType(int idx) {
    return 0; // 0 for double
}

extern "C" int __declspec(dllexport) gs_GetNumberOfOutputs() {
    return g_numOutputs;
}

extern "C" bool __declspec(dllexport) gs_GetOutputName(int idx, char* buffer, int buflen) {
    if (idx == 0) {
        strncpy_s(buffer, buflen, "output", buflen - 1);
        return true;
    }
    return false;
}

extern "C" int __declspec(dllexport) gs_GetOutputDataType(int idx) {
    return 0; // 0 for double
}



// Function to get the directory where this DLL is located
static std::string GetCurrentDllDirectory() {
    char dllPath[MAX_PATH];
    HMODULE hModule = NULL;
    
    // Get handle to this DLL using the address of our exported function
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&gs_DoCalculation, &hModule)) {
        
        // Get full path to the DLL
        if (GetModuleFileNameA(hModule, dllPath, MAX_PATH)) {
            std::string fullPath(dllPath);
            
            // Find the last backslash to get directory
            size_t lastSlash = fullPath.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                return fullPath.substr(0, lastSlash);
            }
        }
    }
    return ""; // Return empty string on failure
}

// Function to get I/O requirements by calling gspy_info() in the Python script
static bool GetScriptIOInfo(int& numInputs, int& numOutputs) {
    try {
        DiagnosticsManager::Instance().LogDebug("Getting I/O info from gspy_info() function...");
        
        // Ensure Python is initialized
        if (!PythonEnvManager::Instance().IsInitialized()) {
            DiagnosticsManager::Instance().LogDebug("Initializing Python for I/O detection...");
            PythonEnvManager::Instance().Initialize();
            if (!PythonEnvManager::Instance().IsInitialized()) {
                DiagnosticsManager::Instance().LogError("Python initialization failed during I/O detection");
                return false;
            }
        }
    } catch (...) {
        DiagnosticsManager::Instance().LogError("Exception during Python initialization for I/O detection");
        return false;
    }
    
    // Get DLL directory and add to Python path
    std::string dllDir = GetCurrentDllDirectory();
    if (dllDir.empty()) {
        DiagnosticsManager::Instance().LogError("Could not determine DLL directory for I/O detection");
        return false;
    }
    
    // Add DLL directory to Python path
    PyObject* sysPath = PySys_GetObject("path");
    PyObject* pyDllDir = PyUnicode_FromString(dllDir.c_str());
    PyList_Insert(sysPath, 0, pyDllDir);
    Py_DECREF(pyDllDir);
    
    // Load the gspy_script module
    PyObject* pModule = PyImport_ImportModule("gspy_script");
    if (!pModule) {
        DiagnosticsManager::Instance().LogError("Failed to import gspy_script module for I/O detection");
        PyErr_Clear();
        return false;
    }
    
    // Get the gspy_info function
    PyObject* pInfoFunc = PyObject_GetAttrString(pModule, "gspy_info");
    if (!pInfoFunc || !PyCallable_Check(pInfoFunc)) {
        DiagnosticsManager::Instance().LogError("gspy_info() function not found or not callable in script");
        Py_DECREF(pModule);
        if (pInfoFunc) Py_DECREF(pInfoFunc);
        PyErr_Clear();
        return false;
    }
    
    // Call gspy_info() function
    PyObject* pResult = PyObject_CallObject(pInfoFunc, NULL);
    Py_DECREF(pInfoFunc);
    Py_DECREF(pModule);
    
    if (!pResult) {
        DiagnosticsManager::Instance().LogError("Failed to call gspy_info() function");
        PyErr_Clear();
        return false;
    }
    
    // Check if result is a dictionary
    if (!PyDict_Check(pResult)) {
        DiagnosticsManager::Instance().LogError("gspy_info() must return a dictionary");
        Py_DECREF(pResult);
        return false;
    }
    
    // Extract 'inputs' value
    PyObject* pInputs = PyDict_GetItemString(pResult, "inputs");
    if (!pInputs || !PyLong_Check(pInputs)) {
        DiagnosticsManager::Instance().LogError("gspy_info() must return a dictionary with 'inputs' as an integer");
        Py_DECREF(pResult);
        return false;
    }
    
    // Extract 'outputs' value
    PyObject* pOutputs = PyDict_GetItemString(pResult, "outputs");
    if (!pOutputs || !PyLong_Check(pOutputs)) {
        DiagnosticsManager::Instance().LogError("gspy_info() must return a dictionary with 'outputs' as an integer");
        Py_DECREF(pResult);
        return false;
    }
    
    // Get the integer values
    numInputs = (int)PyLong_AsLong(pInputs);
    numOutputs = (int)PyLong_AsLong(pOutputs);
    
    Py_DECREF(pResult);
    
    // Validate the values
    if (numInputs < 1 || numOutputs < 1) {
        DiagnosticsManager::Instance().LogError("gspy_info() returned invalid I/O counts (must be >= 1)");
        return false;
    }
    
    DiagnosticsManager::Instance().LogInfo("Successfully retrieved I/O info: " + 
                                          std::to_string(numInputs) + " inputs, " + 
                                          std::to_string(numOutputs) + " outputs");
    return true;
}


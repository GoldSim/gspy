#include <windows.h> // For the DLL entry point
#include <fstream>
#include <string>
#include <sstream>
#include <vector> // For std::vector
#include <Python.h> // Python C API
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

// Forward declaration of our main exported function
extern "C" void __declspec(dllexport) gs_DoCalculation(int methodID, int* status, double* inargs, double* outargs);

// Global UserScriptAdapter instance
static UserScriptAdapter g_scriptAdapter;



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

extern "C" void __declspec(dllexport) gs_DoCalculation(
    int methodID, int* status, double* inargs, double* outargs)
{
    *status = 0; // Default to success

    switch (methodID)
    {
        case XF_INITIALIZE:
        {
            // Initialize the diagnostics manager
            DiagnosticsManager::Instance().SetLogFile("gspy_log.txt");
            DiagnosticsManager::Instance().SetLogLevel(LogLevel::DEBUG);
            DiagnosticsManager::Instance().LogInfo("=== GSPy DLL INITIALIZE ===");
            
            // Initialize Python interpreter using PythonEnvManager
            if (!PythonEnvManager::Instance().IsInitialized()) {
                DiagnosticsManager::Instance().LogInfo("Initializing Python interpreter...");
                PythonEnvManager::Instance().Initialize();
                if (!PythonEnvManager::Instance().IsInitialized()) {
                    DiagnosticsManager::Instance().LogError("Python initialization failed");
                    *status = 1;
                    break;
                }
                DiagnosticsManager::Instance().LogInfo("Python interpreter initialized successfully");
            } else {
                DiagnosticsManager::Instance().LogDebug("Python interpreter already initialized");
            }
            
            // Load the Python script using UserScriptAdapter
            DiagnosticsManager::Instance().LogInfo("Loading gspy_script.py...");
            
            // Log current working directory for debugging
            char currentDir[MAX_PATH];
            if (GetCurrentDirectoryA(MAX_PATH, currentDir)) {
                std::string dirMsg = "Current working directory: ";
                dirMsg += currentDir;
                DiagnosticsManager::Instance().LogDebug(dirMsg);
            }
            
            // Get DLL directory and add to Python path
            std::string dllDir = GetCurrentDllDirectory();
            if (dllDir.empty()) {
                DiagnosticsManager::Instance().LogError("Could not determine DLL directory");
                *status = 1;
                break;
            }
            
            std::string dllDirMsg = "DLL directory: " + dllDir;
            DiagnosticsManager::Instance().LogDebug(dllDirMsg);
            
            // Add DLL directory to Python path so it can find the script
            PyObject* sysPath = PySys_GetObject("path");
            PyObject* pyDllDir = PyUnicode_FromString(dllDir.c_str());
            PyList_Insert(sysPath, 0, pyDllDir);
            Py_DECREF(pyDllDir);
            
            DiagnosticsManager::Instance().LogDebug("Added DLL directory to Python path");
            
            // Use UserScriptAdapter to load the script
            if (!g_scriptAdapter.LoadScript("gspy_script")) {
                DiagnosticsManager::Instance().LogError("Failed to load gspy_script.py using UserScriptAdapter");
                *status = 1;
                break;
            }
            DiagnosticsManager::Instance().LogInfo("Python script loaded successfully");
            
            // Check if the script has the required function
            if (!g_scriptAdapter.HasGoldSimCalculate()) {
                DiagnosticsManager::Instance().LogError("goldsim_calculate function not found or not callable");
                *status = 1;
                break;
            }
            DiagnosticsManager::Instance().LogInfo("Python goldsim_calculate function found and ready");
            
            DiagnosticsManager::Instance().LogInfo("DLL initialization completed successfully");
            break;
        }
            
        case XF_CALCULATE:
        {
            DataMarshaller marshaller; // Create DataMarshaller instance
            
            if (g_scriptAdapter.HasGoldSimCalculate()) {
                // Use DataMarshaller to pack all input arguments
                PyObject* inputDict = marshaller.PackArguments(inargs, g_numInputs);
                if (!inputDict) {
                    DiagnosticsManager::Instance().LogError("Failed to pack input arguments");
                    for (int i = 0; i < g_numOutputs; i++) {
                        outargs[i] = -999.0;
                    }
                    *status = 1;
                    break;
                }
                
                // Use UserScriptAdapter to call the Python function
                std::string errorMsg;
                PyObject* pResult = g_scriptAdapter.CallGoldSimCalculate(inputDict, errorMsg);
                Py_DECREF(inputDict);
                
                if (pResult) {
                    // Choose the appropriate unpacking method based on output type
                    const int MAX_OUTPUT_SIZE = 1000;  // Maximum size for time series data
                    bool unpackSuccess = false;
                    bool hasTimeSeries = false;  // Moved to broader scope
                    
                    // First check if this is a time series output
                    if (PyDict_Check(pResult)) {
                        PyObject* key, * value;
                        Py_ssize_t pos = 0;
                        
                        while (PyDict_Next(pResult, &pos, &key, &value)) {
                            if (PyUnicode_Check(key)) {
                                const char* keyStr = PyUnicode_AsUTF8(key);
                                std::string keyName(keyStr);
                                if (keyName.find("goldsim_timeseries_") == 0) {
                                    hasTimeSeries = true;
                                    break;
                                }
                            }
                        }
                        
                        if (hasTimeSeries) {
                            // Use time series unpacking
                            unpackSuccess = marshaller.UnpackSingleTimeSeries(pResult, outargs, MAX_OUTPUT_SIZE);
                        } else {
                            // Use regular scalar unpacking
                            unpackSuccess = marshaller.UnpackResult(pResult, outargs, g_numOutputs);
                        }
                    }
                    
                    if (unpackSuccess) {
                        // Success! Log the input/output values
                        std::ostringstream oss;
                        oss << "CALCULATE: Python call successful - inputs=[";
                        for (int i = 0; i < g_numInputs; i++) {
                            if (i > 0) oss << ", ";
                            oss << inargs[i];
                        }
                        oss << "], outputs=[";
                        
                        // Log the appropriate number of outputs based on whether it's time series or regular
                        int outputsToLog = hasTimeSeries ? 15 : g_numOutputs;
                        for (int i = 0; i < outputsToLog; i++) {
                            if (i > 0) oss << ", ";
                            oss << outargs[i];
                        }
                        oss << "]";
                        
                        if (hasTimeSeries) {
                            oss << " (time series data)";
                        }
                        
                        DiagnosticsManager::Instance().LogInfo(oss.str());
                    } else {
                        DiagnosticsManager::Instance().LogError("Failed to unpack results from Python function");
                        for (int i = 0; i < g_numOutputs; i++) {
                            outargs[i] = -999.0;
                        }
                        *status = 1;
                    }
                    Py_DECREF(pResult);
                } else {
                    DiagnosticsManager::Instance().LogError("Python function call failed: " + errorMsg);
                    for (int i = 0; i < g_numOutputs; i++) {
                        outargs[i] = -999.0;
                    }
                    *status = 1;
                }
            } else {
                DiagnosticsManager::Instance().LogError("Python function not available");
                for (int i = 0; i < g_numOutputs; i++) {
                    outargs[i] = -999.0;
                }
                *status = 1;
            }
            
            break;
        }
            
        case XF_REP_VERSION:
            outargs[0] = 1.0;
            DiagnosticsManager::Instance().LogDebug("REP_VERSION: returning 1.0");
            break;
            
        case XF_REP_ARGUMENTS:
        {
            // Try to get I/O requirements from the Python script's gspy_info() function
            int detectedInputs = 1;   // Default fallback values
            int detectedOutputs = 1;
            
            // Only attempt dynamic detection if it's safe to do so
            bool dynamicDetectionSuccess = false;
            try {
                dynamicDetectionSuccess = GetScriptIOInfo(detectedInputs, detectedOutputs);
            } catch (...) {
                DiagnosticsManager::Instance().LogError("Exception during dynamic I/O detection");
                dynamicDetectionSuccess = false;
            }
            
            if (dynamicDetectionSuccess) {
                // Successfully got I/O info from script
                g_numInputs = detectedInputs;
                g_numOutputs = detectedOutputs;
                DiagnosticsManager::Instance().LogInfo("REP_ARGUMENTS: " + std::to_string(g_numInputs) + 
                                                      " inputs, " + std::to_string(g_numOutputs) + 
                                                      " outputs (from gspy_info)");
            } else {
                // Fallback to default values if gspy_info() fails
                g_numInputs = detectedInputs;
                g_numOutputs = detectedOutputs;
                DiagnosticsManager::Instance().LogError("Failed to get I/O info from gspy_info(), using defaults: " + 
                                                       std::to_string(g_numInputs) + " input, " + 
                                                       std::to_string(g_numOutputs) + " output");
            }
            
            outargs[0] = g_numInputs;
            outargs[1] = g_numOutputs;
            break;
        }
            
        case XF_CLEANUP:
        {
            DiagnosticsManager::Instance().LogInfo("CLEANUP: Starting DLL cleanup...");
            
            // UserScriptAdapter handles its own Python object cleanup in its destructor
            DiagnosticsManager::Instance().LogDebug("Python objects managed by UserScriptAdapter");
            
            // PythonEnvManager handles Python interpreter lifecycle
            DiagnosticsManager::Instance().LogDebug("Python environment managed by PythonEnvManager");
            
            DiagnosticsManager::Instance().LogInfo("CLEANUP: DLL cleanup completed");
            break;
        }
            
        default:
            DiagnosticsManager::Instance().LogError("Unknown method ID");
            *status = 1;
            break;
    }
}
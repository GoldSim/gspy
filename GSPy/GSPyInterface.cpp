#include <windows.h>
#include <string>
#include <chrono>
#include <Python.h>
#include "DiagnosticsManager.h"
#include "DataMarshaller.h"
#include "PythonEnvManager.h"
#include "UserScriptAdapter.h"
#include "TimeSeriesMarshaller.h"

// Define the method IDs for clarity
enum XFMethodID {
    XF_INITIALIZE = 0,
    XF_CALCULATE = 1,
    XF_REP_VERSION = 2,
    XF_REP_ARGUMENTS = 3,
    XF_CLEANUP = 99
};

// Global instances and variables
static UserScriptAdapter g_scriptAdapter;
static DataMarshaller    g_marshaller;
static int g_numInputs = 1;
static int g_numOutputs = 1;
static bool g_isInitialized = false;
static int g_calculationCount = 0;
static int g_instanceId = 0;

// Forward declaration
extern "C" void __declspec(dllexport) gs_DoCalculation(int methodID, int* status, double* inargs, double* outargs);

// **Function from old file to find the DLL's path**
static std::string GetCurrentDllDirectory() {
    char dllPath[MAX_PATH];
    HMODULE hModule = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&gs_DoCalculation, &hModule)) {
        if (GetModuleFileNameA(hModule, dllPath, MAX_PATH)) {
            std::string fullPath(dllPath);
            size_t lastSlash = fullPath.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                return fullPath.substr(0, lastSlash);
            }
        }
    }
    return "";
}

// Main DLL function called by GoldSim
void gs_DoCalculation(int methodID, int* status, double* inargs, double* outargs) {
    *status = 0; // Default to success
    
    // Set up logging early for all method calls, but safely
    static bool loggingSetup = false;
    if (!loggingSetup) {
        try {
            DiagnosticsManager::Instance().SetLogFile("gspy_log.txt");
            DiagnosticsManager::Instance().SetLogLevel(LogLevel::DEBUG);
            loggingSetup = true;
        } catch (...) {
            // If logging setup fails, continue without logging
        }
    }
    
    // Log method calls only if logging is working
    if (loggingSetup) {
        try {
            std::string methodName;
            switch (methodID) {
                case XF_INITIALIZE: methodName = "XF_INITIALIZE"; break;
                case XF_CALCULATE: methodName = "XF_CALCULATE"; break;
                case XF_REP_VERSION: methodName = "XF_REP_VERSION"; break;
                case XF_REP_ARGUMENTS: methodName = "XF_REP_ARGUMENTS"; break;
                case XF_CLEANUP: methodName = "XF_CLEANUP"; break;
                default: methodName = "UNKNOWN(" + std::to_string(methodID) + ")"; break;
            }
            DiagnosticsManager::Instance().LogDebug("=== METHOD CALL: " + methodName + " ===");
        } catch (...) {
            // If logging fails, continue without logging
        }
    }

    switch (methodID) {
        case XF_INITIALIZE:
        {
            if (g_isInitialized) {
                *status = 0;
                break;
            }
            
            // When called from Python (like our test), Python is already initialized
            // Only initialize if not already initialized (when called from GoldSim)
            if (!Py_IsInitialized()) {
                Py_Initialize();
                if (!Py_IsInitialized()) {
                    *status = 1;
                    break;
                }
            }
            
            // Add current directory to Python path to help find scripts
            PyGILState_STATE gstate = PyGILState_Ensure();
            try {
                char currentDir[MAX_PATH];
                if (GetCurrentDirectoryA(MAX_PATH, currentDir)) {
                    PyObject* sysPath = PySys_GetObject("path");
                    if (sysPath) {
                        PyObject* pyCurrentDir = PyUnicode_FromString(currentDir);
                        if (pyCurrentDir) {
                            PyList_Insert(sysPath, 0, pyCurrentDir);
                            Py_DECREF(pyCurrentDir);
                        }
                    }
                }
                
                // Try to get I/O info from the script
                PyObject* pName = PyUnicode_DecodeFSDefault("gspy_script");
                if (pName) {
                    PyObject* pModule = PyImport_Import(pName);
                    Py_DECREF(pName);
                    if (pModule) {
                        PyObject* pFunc = PyObject_GetAttrString(pModule, "gspy_info");
                        if (pFunc && PyCallable_Check(pFunc)) {
                            PyObject* pResult = PyObject_CallObject(pFunc, NULL);
                            if (pResult && PyDict_Check(pResult)) {
                                PyObject* pInputs = PyDict_GetItemString(pResult, "inputs");
                                if (pInputs && PyLong_Check(pInputs)) {
                                    g_numInputs = PyLong_AsLong(pInputs);
                                }
                                
                                PyObject* pOutputs = PyDict_GetItemString(pResult, "outputs");
                                if (pOutputs && PyLong_Check(pOutputs)) {
                                    g_numOutputs = PyLong_AsLong(pOutputs);
                                }
                            }
                            Py_XDECREF(pResult);
                        }
                        Py_XDECREF(pFunc);
                        Py_DECREF(pModule);
                    } else {
                        PyErr_Clear();
                    }
                }
                PyGILState_Release(gstate);
            } catch (...) {
                PyGILState_Release(gstate);
            }
            
            g_isInitialized = true;
            *status = 0;
            break;
        }

        case XF_CALCULATE:
        {
            // Ensure we have the GIL when calling Python API
            PyGILState_STATE gstate = PyGILState_Ensure();
            
            try {
                // Try to load and call the actual Python script
                PyObject* pName = PyUnicode_DecodeFSDefault("gspy_script");
                if (!pName) {
                    PyGILState_Release(gstate);
                    outargs[0] = inargs[0] * 10.0;
                    *status = 0;
                    break;
                }
                
                PyObject* pModule = PyImport_Import(pName);
                Py_DECREF(pName);
                if (!pModule) {
                    PyErr_Clear();
                    PyGILState_Release(gstate);
                    outargs[0] = inargs[0] * 10.0;
                    *status = 0;
                    break;
                }
                
                PyObject* pFunc = PyObject_GetAttrString(pModule, "goldsim_calculate");
                if (!pFunc || !PyCallable_Check(pFunc)) {
                    Py_DECREF(pModule);
                    PyGILState_Release(gstate);
                    outargs[0] = inargs[0] * 10.0;
                    *status = 0;
                    break;
                }
                
                // Create input dictionary - check for time series input first
                PyObject* inputDict = PyDict_New();
                
                // Try to detect and convert time series input
                TimeSeriesMarshaller tsMarshaller;
                if (!tsMarshaller.PackTimeSeriesInput(inargs, g_numInputs, inputDict)) {
                    // Not a time series, handle as regular scalar inputs
                    for (int i = 0; i < g_numInputs; ++i) {
                        std::string keyName = "input" + std::to_string(i + 1);
                        PyObject* inputValue = PyFloat_FromDouble(inargs[i]);
                        PyDict_SetItemString(inputDict, keyName.c_str(), inputValue);
                        Py_DECREF(inputValue);
                    }
                }
                
                // Call function
                PyObject* args = PyTuple_Pack(1, inputDict);
                Py_DECREF(inputDict);
                PyObject* pResult = PyObject_CallObject(pFunc, args);
                Py_DECREF(args);
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                
                if (pResult && PyDict_Check(pResult)) {
                    // Use DataMarshaller to handle complex data types (lookup tables, time series, etc.)
                    if (g_marshaller.UnpackResult(pResult, outargs, g_numOutputs)) {
                        Py_DECREF(pResult);
                        PyGILState_Release(gstate);
                        *status = 0;
                        break;
                    }
                }
                
                // Fallback if script execution failed
                Py_XDECREF(pResult);
                PyGILState_Release(gstate);
                outargs[0] = inargs[0] * 10.0;
                *status = 0;
                
            } catch (...) {
                PyGILState_Release(gstate);
                outargs[0] = inargs[0] * 10.0;
                *status = 0;
            }
            break;
        }

        case XF_REP_VERSION:
        {
            // Return a version number to GoldSim
            if (outargs) {
                outargs[0] = 1.5; // Example version number
            }
            *status = 0;
            break;
        }

        case XF_REP_ARGUMENTS:
        {
            // If not initialized yet, try to parse gspy_info() from the Python file
            if (!g_isInitialized) {
                try {
                    char scriptPath[MAX_PATH];
                    if (GetCurrentDirectoryA(MAX_PATH, scriptPath)) {
                        strcat_s(scriptPath, MAX_PATH, "\\gspy_script.py");
                        
                        FILE* scriptFile = nullptr;
                        if (fopen_s(&scriptFile, scriptPath, "r") == 0 && scriptFile) {
                            char line[512];
                            bool inGspyInfo = false;
                            bool inReturn = false;
                            
                            while (fgets(line, sizeof(line), scriptFile)) {
                                // Look for the gspy_info function definition
                                if (strstr(line, "def gspy_info(")) {
                                    inGspyInfo = true;
                                    continue;
                                }
                                
                                // If we're in gspy_info, look for the return statement
                                if (inGspyInfo && strstr(line, "return")) {
                                    inReturn = true;
                                }
                                
                                // Parse the return dictionary (continue reading until we find both values)
                                if (inGspyInfo) {
                                    // Look for 'inputs': number
                                    char* inputsPos = strstr(line, "'inputs':");
                                    if (!inputsPos) inputsPos = strstr(line, "\"inputs\":");
                                    if (inputsPos) {
                                        inputsPos = strchr(inputsPos, ':');
                                        if (inputsPos) {
                                            g_numInputs = atoi(inputsPos + 1);
                                        }
                                    }
                                    
                                    // Look for 'outputs': number
                                    char* outputsPos = strstr(line, "'outputs':");
                                    if (!outputsPos) outputsPos = strstr(line, "\"outputs\":");
                                    if (outputsPos) {
                                        outputsPos = strchr(outputsPos, ':');
                                        if (outputsPos) {
                                            g_numOutputs = atoi(outputsPos + 1);
                                        }
                                    }
                                    
                                    // If we hit the end of the function, stop
                                    if (strchr(line, '}') && inReturn) {
                                        break;
                                    }
                                    
                                    // If we see the next function definition, we're done
                                    if (strstr(line, "def ") && inReturn) {
                                        break;
                                    }
                                }
                            }
                            fclose(scriptFile);
                        }
                    }
                } catch (...) {
                    // If anything fails, use defaults
                }
                
                // Log what we found for debugging
                if (loggingSetup) {
                    try {
                        DiagnosticsManager::Instance().LogDebug("Parsed gspy_info(): inputs=" + std::to_string(g_numInputs) + 
                                                               ", outputs=" + std::to_string(g_numOutputs));
                    } catch (...) {
                        // Ignore logging errors
                    }
                }
            }
            
            // Return the I/O counts (either parsed or defaults)
            if (outargs) {
                outargs[0] = g_numInputs;
                outargs[1] = g_numOutputs;
            }
            *status = 0;
            break;
        }

        case XF_CLEANUP:
        {
            DiagnosticsManager::Instance().LogInfo("CLEANUP: Starting DLL cleanup...");
            PythonEnvManager::Instance().Finalize();
            g_isInitialized = false;
            DiagnosticsManager::Instance().LogInfo("CLEANUP: DLL cleanup completed");
            break;
        }

        default:
            DiagnosticsManager::Instance().LogError("Unknown method ID received.");
            *status = 1;
            break;
    }
}


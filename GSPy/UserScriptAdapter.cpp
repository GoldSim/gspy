#include "UserScriptAdapter.h"
#include "DiagnosticsManager.h"
#include "PythonEnvManager.h"
#include "DataMarshaller.h"

UserScriptAdapter::UserScriptAdapter() : pModule_(nullptr), pFunc_goldsim_calculate_(nullptr), pFunc_gspy_info_(nullptr) {
}

UserScriptAdapter::~UserScriptAdapter() {
    UnloadScript();
}

bool UserScriptAdapter::Initialize(const std::string& scriptPath) {
    if (!PythonEnvManager::Instance().IsInitialized()) {
        PythonEnvManager::Instance().Initialize();
        if (!PythonEnvManager::Instance().IsInitialized()) {
            DiagnosticsManager::Instance().LogError("Failed to initialize Python environment.");
            return false;
        }
    }
    return LoadScript(scriptPath);
}

PyObject* UserScriptAdapter::RunCalculation(const double* inargs, int input_count) {
    if (!pFunc_goldsim_calculate_) {
        DiagnosticsManager::Instance().LogError("'goldsim_calculate' function is not loaded.");
        return nullptr;
    }
    
    // Clear any pending Python exceptions before we start
    if (PyErr_Occurred()) {
        PyErr_Clear();
    }

    // Directly marshal C++ array to Python dict (cleanest, matches RunCalculation)
    PyObject* inputDict = PyDict_New();
    if (!inputDict) {
        DiagnosticsManager::Instance().LogError("Failed to create Python input dictionary.");
        return nullptr;
    }
    
    for (int i = 0; i < input_count; ++i) {
        std::string key = "input" + std::to_string(i + 1);
        PyObject* value = PyFloat_FromDouble(inargs[i]);
        if (!value) {
            DiagnosticsManager::Instance().LogError("Failed to marshal input value at index " + std::to_string(i) + 
                                                   " (value: " + std::to_string(inargs[i]) + ")");
            Py_DECREF(inputDict);
            return nullptr;
        }
        
        int result = PyDict_SetItemString(inputDict, key.c_str(), value);
        Py_DECREF(value);
        
        if (result < 0) {
            DiagnosticsManager::Instance().LogError("Failed to set dictionary item for key: " + key);
            Py_DECREF(inputDict);
            return nullptr;
        }
    }

    PyObject* args = PyTuple_Pack(1, inputDict);
    Py_DECREF(inputDict);
    
    if (!args) {
        DiagnosticsManager::Instance().LogError("Failed to create argument tuple for Python function call.");
        return nullptr;
    }
    
    PyObject* pResult = PyObject_CallObject(pFunc_goldsim_calculate_, args);
    Py_DECREF(args);

    if (!pResult) {
        DiagnosticsManager::Instance().LogError("Python function 'goldsim_calculate' execution failed.");
        
        // Capture and log the actual Python error
        if (PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            
            if (pvalue) {
                PyObject* pStr = PyObject_Str(pvalue);
                if (pStr) {
                    const char* errorMsg = PyUnicode_AsUTF8(pStr);
                    if (errorMsg) {
                        DiagnosticsManager::Instance().LogError("Python error details: " + std::string(errorMsg));
                    }
                    Py_DECREF(pStr);
                }
            }
            
            // Clean up error state
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);
        }
        return nullptr;
    }
    
    return pResult;
}

bool UserScriptAdapter::LoadScript(const std::string& scriptPath) {
    UnloadScript();
    std::string scriptName = scriptPath;
    size_t pos = scriptName.rfind(".py");
    if (pos != std::string::npos) {
        scriptName.erase(pos);
    }

    PyObject* pName = PyUnicode_DecodeFSDefault(scriptName.c_str());
    if (!pName) {
        DiagnosticsManager::Instance().LogError("Failed to decode script name: " + scriptName);
        return false;
    }

    pModule_ = PyImport_Import(pName);
    Py_DECREF(pName);

    if (!pModule_) {
        DiagnosticsManager::Instance().LogError("Failed to import Python module: " + scriptName);
        PyErr_Print();
        return false;
    }

    pFunc_goldsim_calculate_ = PyObject_GetAttrString(pModule_, "goldsim_calculate");
    if (!pFunc_goldsim_calculate_) {
        DiagnosticsManager::Instance().LogError("Failed to find 'goldsim_calculate' function in Python script.");
        return false;
    }
    DiagnosticsManager::Instance().LogInfo("Python goldsim_calculate function found and ready");
    
    pFunc_gspy_info_ = PyObject_GetAttrString(pModule_, "gspy_info");
    if (pFunc_gspy_info_) {
        DiagnosticsManager::Instance().LogInfo("Python gspy_info function found and ready");
    } else {
        DiagnosticsManager::Instance().LogDebug("Python gspy_info function not found - will use default I/O");
        PyErr_Clear(); // Clear the error since this is optional
    }

    return true;
}

void UserScriptAdapter::UnloadScript() {
    Py_XDECREF(pFunc_goldsim_calculate_);
    Py_XDECREF(pFunc_gspy_info_);
    Py_XDECREF(pModule_);
    pModule_ = nullptr;
    pFunc_goldsim_calculate_ = nullptr;
    pFunc_gspy_info_ = nullptr;
}

void UserScriptAdapter::GetIOArguments(int& inputs, int& outputs) {
    if (!pFunc_gspy_info_) {
        return;
    }

    PyObject* pResult = PyObject_CallObject(pFunc_gspy_info_, NULL);
    if (!pResult || !PyDict_Check(pResult)) {
        DiagnosticsManager::Instance().LogError("'gspy_info' did not return a valid dictionary.");
        Py_XDECREF(pResult);
        return;
    }

    PyObject* pInputs = PyDict_GetItemString(pResult, "inputs");
    if (pInputs && PyLong_Check(pInputs)) {
        inputs = PyLong_AsLong(pInputs);
    }

    PyObject* pOutputs = PyDict_GetItemString(pResult, "outputs");
    if (pOutputs && PyLong_Check(pOutputs)) {
        outputs = PyLong_AsLong(pOutputs);
    }

    Py_DECREF(pResult);
}
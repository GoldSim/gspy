#include "UserScriptAdapter.h"
#include "DiagnosticsManager.h"

UserScriptAdapter::UserScriptAdapter() : pModule_(nullptr), pFunc_(nullptr) {
}

UserScriptAdapter::~UserScriptAdapter() {
    if (pFunc_) Py_XDECREF(pFunc_);
    if (pModule_) Py_XDECREF(pModule_);
}

bool UserScriptAdapter::LoadScript(const std::string& scriptName) {
    DiagnosticsManager::Instance().LogDebug("Loading user script: " + scriptName);
    
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
    
    pFunc_ = PyObject_GetAttrString(pModule_, "goldsim_calculate");
    if (!pFunc_ || !PyCallable_Check(pFunc_)) {
        DiagnosticsManager::Instance().LogError("goldsim_calculate function not found or not callable in " + scriptName);
        if (pFunc_) Py_XDECREF(pFunc_);
        pFunc_ = nullptr;
        return false;
    }
    
    return true;
}

bool UserScriptAdapter::HasGoldSimCalculate() const {
    return pFunc_ && PyCallable_Check(pFunc_);
}

PyObject* UserScriptAdapter::CallGoldSimCalculate(PyObject* inputsDict, std::string& errorMsg) {
    if (!HasGoldSimCalculate()) {
        errorMsg = "goldsim_calculate function not loaded.";
        return nullptr;
    }
    
    PyObject* args = PyTuple_Pack(1, inputsDict);
    PyObject* result = PyObject_CallObject(pFunc_, args);
    Py_DECREF(args);
    
    if (!result) {
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
        
        PyObject* pyStr = PyObject_Str(pvalue);
        if (pyStr) {
            errorMsg = PyUnicode_AsUTF8(pyStr);
            Py_DECREF(pyStr);
        } else {
            errorMsg = "Unknown Python error.";
        }
        
        DiagnosticsManager::Instance().LogError("Exception in goldsim_calculate: " + errorMsg);
        Py_XDECREF(ptype);
        Py_XDECREF(pvalue);
        Py_XDECREF(ptraceback);
        return nullptr;
    }
    
    return result;
}
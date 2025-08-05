#pragma once

#include <string>
#include <Python.h>

class UserScriptAdapter {
public:
    UserScriptAdapter();
    ~UserScriptAdapter();
    
    bool LoadScript(const std::string& scriptName);
    bool HasGoldSimCalculate() const;
    PyObject* CallGoldSimCalculate(PyObject* inputsDict, std::string& errorMsg);
    
    // Exposed for UpdateIOInfo function
    PyObject* pFunc_;

private:
    PyObject* pModule_;
};
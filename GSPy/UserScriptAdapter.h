#pragma once

#include <string>
#include <Python.h>

class UserScriptAdapter {
public:
    UserScriptAdapter();
    ~UserScriptAdapter();

    bool Initialize(const std::string& scriptPath);
    void GetIOArguments(int& inputs, int& outputs);
    PyObject* RunCalculation(const double* inargs, int input_count);

private:
    // Helper methods
    bool LoadScript(const std::string& scriptPath);
    void UnloadScript();

    // Member variables use a trailing underscore
    PyObject* pModule_;
    PyObject* pFunc_goldsim_calculate_;
    PyObject* pFunc_gspy_info_;
};
#pragma once
#include <Python.h>
#include "json.hpp"
#include <string>

// This function takes GoldSim data and marshals it INTO a Python object.
PyObject* MarshalGoldSimTimeSeriesToPython(double*& current_inarg_pointer, const nlohmann::json& config);

// This is the single, correct declaration for the function.
bool MarshalPythonTimeSeriesToGoldSim(PyObject* py_object, const nlohmann::json& config, double*& current_outarg_pointer, std::string& errorMessage);
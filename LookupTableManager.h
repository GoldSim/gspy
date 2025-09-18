#pragma once
#include <Python.h>
#include "json.hpp"
#include <string>

// Takes a Python dictionary and writes the GoldSim Lookup Table format to the output buffer.
bool MarshalPythonLookupTableToGoldSim(PyObject* py_object, const nlohmann::json& config, double*& current_outarg_pointer, std::string& errorMessage);
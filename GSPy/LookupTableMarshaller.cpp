#include "LookupTableMarshaller.h"
#include "DiagnosticsManager.h"
#include <string>

bool LookupTableMarshaller::UnpackLookupTable1D(PyObject* tableDict, double* outargs, int& out_arg_count) {
    if (!PyDict_Check(tableDict)) {
        DiagnosticsManager::Instance().LogError("1D lookup table output is not a dictionary.");
        return false;
    }

    // --- FIX STARTS HERE ---
    // Extract the lists from the dictionary
    PyObject* independentObj = PyDict_GetItemString(tableDict, "independent_values");
    PyObject* dependentObj = PyDict_GetItemString(tableDict, "dependent_values");

    // Validate that the lists were found
    if (!independentObj || !dependentObj) {
        DiagnosticsManager::Instance().LogError("1D lookup table dictionary is missing 'independent_values' or 'dependent_values' keys.");
        return false;
    }
    // --- FIX ENDS HERE ---

    if (!PyList_Check(independentObj) || !PyList_Check(dependentObj)) {
        DiagnosticsManager::Instance().LogError("1D lookup table 'independent_values' or 'dependent_values' must be lists.");
        return false;
    }

    Py_ssize_t independentSize = PyList_Size(independentObj);
    Py_ssize_t dependentSize = PyList_Size(dependentObj);

    if (independentSize != dependentSize) {
        DiagnosticsManager::Instance().LogError("1D lookup table independent and dependent value lists must have the same size.");
        return false;
    }

    int index = 0;
    // 1. Number of dimensions (1.0 for a 1D table)
    outargs[index++] = 1.0;
    // 2. Number of points in the table
    outargs[index++] = static_cast<double>(independentSize);

    // 3. Independent values
    for (Py_ssize_t i = 0; i < independentSize; i++) {
        PyObject* item = PyList_GetItem(independentObj, i);
        if (!item || (!PyFloat_Check(item) && !PyLong_Check(item))) {
            DiagnosticsManager::Instance().LogError("Non-numeric independent value in lookup table at index " + std::to_string(i));
            return false;
        }
        outargs[index++] = PyFloat_AsDouble(item);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert independent value for lookup table at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
    }

    // 4. Dependent values
    for (Py_ssize_t i = 0; i < dependentSize; i++) {
        PyObject* item = PyList_GetItem(dependentObj, i);
        if (!item || (!PyFloat_Check(item) && !PyLong_Check(item))) {
            DiagnosticsManager::Instance().LogError("Non-numeric dependent value in lookup table at index " + std::to_string(i));
            return false;
        }
        outargs[index++] = PyFloat_AsDouble(item);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert dependent value for lookup table at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
    }

    out_arg_count = index;
    DiagnosticsManager::Instance().LogInfo("Successfully unpacked 1D lookup table with " + std::to_string(independentSize) + " points.");
    return true;
}
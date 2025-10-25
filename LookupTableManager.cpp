// Required includes for Python/NumPy
#ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG
#else
    #include <Python.h>
#endif

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/ndarrayobject.h"

#include "LookupTableManager.h"
#include "Logger.h"
#include <vector>


// Helper function to safely get a NumPy array from the Python dictionary
static PyArrayObject* get_numpy_array(PyObject* py_dict, const char* key, std::string& errorMessage) {
    PyObject* py_item = PyDict_GetItemString(py_dict, key);
    if (!py_item) {
        errorMessage = "Error: Python dictionary is missing required key: " + std::string(key);
        return nullptr;
    }
    return (PyArrayObject*)PyArray_FROM_OTF(py_item, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
}

bool MarshalPythonLookupTableToGoldSim(PyObject* py_object, const nlohmann::json& config, double*& current_outarg_pointer, std::string& errorMessage) {
    Log("--- LookupTableManager: Marshalling Python Lookup Table to GoldSim ---");

    // Initialize NumPy API if it hasn't been already
    static bool numpy_initialized = false;
    if (!numpy_initialized) {
        if (_import_array() < 0) {
            errorMessage = "Error: Could not initialize NumPy C-API in LookupTableManager.";
            LogError(errorMessage); PyErr_Print(); return false;
        }
        numpy_initialized = true;
    }

    if (!PyDict_Check(py_object)) {
        errorMessage = "Error: Python script was expected to return a dictionary for a Lookup Table output.";
        LogError(errorMessage); return false;
    }

    // Extract table dimension
    PyObject* py_dim = PyDict_GetItemString(py_object, "table_dim");
    if (!py_dim) {
        errorMessage = "Error: Python dictionary for Table is missing required key 'table_dim'.";
        LogError(errorMessage); return false;
    }
    long table_dim = PyLong_AsLong(py_dim);

    // Use a switch to handle 1D, 2D, or 3D cases
    switch (table_dim) {
    case 1: {
        LogDebug("  Marshalling 1D Lookup Table.");
        PyArrayObject* row_labels = get_numpy_array(py_object, "row_labels", errorMessage);
        PyArrayObject* data = get_numpy_array(py_object, "data", errorMessage);
        if (!row_labels || !data) { if (row_labels) Py_DECREF(row_labels); if (data) Py_DECREF(data); return false; }

        npy_intp num_rows = PyArray_SIZE(row_labels);

        // Write 1D table sequence to GoldSim buffer
        *current_outarg_pointer++ = 1.0; // Number of dimensions
        *current_outarg_pointer++ = static_cast<double>(num_rows);
        memcpy(current_outarg_pointer, PyArray_DATA(row_labels), num_rows * sizeof(double));
        current_outarg_pointer += num_rows;
        memcpy(current_outarg_pointer, PyArray_DATA(data), num_rows * sizeof(double));
        current_outarg_pointer += num_rows;

        Py_DECREF(row_labels);
        Py_DECREF(data);
        break;
    }
    case 2: {
        LogDebug("  Marshalling 2D Lookup Table.");
        PyArrayObject* row_labels = get_numpy_array(py_object, "row_labels", errorMessage);
        PyArrayObject* col_labels = get_numpy_array(py_object, "col_labels", errorMessage);
        PyArrayObject* data = get_numpy_array(py_object, "data", errorMessage);
        if (!row_labels || !col_labels || !data) { /* Cleanup and return */ return false; }

        npy_intp num_rows = PyArray_SIZE(row_labels);
        npy_intp num_cols = PyArray_SIZE(col_labels);
        npy_intp data_size = PyArray_SIZE(data);

        // Write 2D table sequence to GoldSim buffer
        *current_outarg_pointer++ = 2.0; // Number of dimensions
        *current_outarg_pointer++ = static_cast<double>(num_rows);
        *current_outarg_pointer++ = static_cast<double>(num_cols);
        memcpy(current_outarg_pointer, PyArray_DATA(row_labels), num_rows * sizeof(double));
        current_outarg_pointer += num_rows;
        memcpy(current_outarg_pointer, PyArray_DATA(col_labels), num_cols * sizeof(double));
        current_outarg_pointer += num_cols;
        memcpy(current_outarg_pointer, PyArray_DATA(data), data_size * sizeof(double));
        current_outarg_pointer += data_size;

        Py_DECREF(row_labels);
        Py_DECREF(col_labels);
        Py_DECREF(data);
        break;
    }
    case 3: {
        LogDebug("  Marshalling 3D Lookup Table.");
        PyArrayObject* row_labels = get_numpy_array(py_object, "row_labels", errorMessage);
        PyArrayObject* col_labels = get_numpy_array(py_object, "col_labels", errorMessage);
        PyArrayObject* layer_labels = get_numpy_array(py_object, "layer_labels", errorMessage);
        PyArrayObject* data = get_numpy_array(py_object, "data", errorMessage);
        if (!row_labels || !col_labels || !layer_labels || !data) { /* Cleanup and return */ return false; }

        npy_intp num_rows = PyArray_SIZE(row_labels);
        npy_intp num_cols = PyArray_SIZE(col_labels);
        npy_intp num_layers = PyArray_SIZE(layer_labels);
        npy_intp data_size = PyArray_SIZE(data);

        // Write 3D table sequence to GoldSim buffer
        *current_outarg_pointer++ = 3.0; // Number of dimensions
        *current_outarg_pointer++ = static_cast<double>(num_rows);
        *current_outarg_pointer++ = static_cast<double>(num_cols);
        *current_outarg_pointer++ = static_cast<double>(num_layers);
        memcpy(current_outarg_pointer, PyArray_DATA(row_labels), num_rows * sizeof(double));
        current_outarg_pointer += num_rows;
        memcpy(current_outarg_pointer, PyArray_DATA(col_labels), num_cols * sizeof(double));
        current_outarg_pointer += num_cols;
        memcpy(current_outarg_pointer, PyArray_DATA(layer_labels), num_layers * sizeof(double));
        current_outarg_pointer += num_layers;
        memcpy(current_outarg_pointer, PyArray_DATA(data), data_size * sizeof(double));
        current_outarg_pointer += data_size;

        Py_DECREF(row_labels);
        Py_DECREF(col_labels);
        Py_DECREF(layer_labels);
        Py_DECREF(data);
        break;
    }
    default:
        errorMessage = "Error: Invalid 'table_dim' provided. Must be 1, 2, or 3.";
        LogError(errorMessage);
        return false;
    }

    return true;
}
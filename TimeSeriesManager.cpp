#ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG
#else
    #include <Python.h>
#endif
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include "numpy/ndarrayobject.h"

#include "TimeSeriesManager.h"
#include "Logger.h"
#include <vector>
#include <numpy/arrayobject.h>
#include <sstream>


PyObject* MarshalGoldSimTimeSeriesToPython(double*& current_inarg_pointer, const nlohmann::json& config) {
    // Initialize NumPy API, but only once.
    static bool numpy_initialized = false;
    if (!numpy_initialized) {
        if (_import_array() < 0) {
            LogError("Error: Could not initialize NumPy C-API in TimeSeriesManager.");
            PyErr_Print();
            // We can't send an error here easily, so we'll return None and let the Python call fail.
            Py_INCREF(Py_None);
            return Py_None;
        }
        numpy_initialized = true;
    }
    Log("--- TimeSeriesManager: Marshalling GoldSim Time Series to Python ---");

    // We will now log every piece of metadata we read from the data stream.
    double ts_id = *current_inarg_pointer++;
    LogDebug("  TS Metadata: ts_id = " + std::to_string(ts_id));

    double format_version = *current_inarg_pointer++;
    LogDebug("  TS Metadata: format_version = " + std::to_string(format_version));

    double time_basis = *current_inarg_pointer++;
    LogDebug("  TS Metadata: time_basis = " + std::to_string(time_basis));

    double data_type = *current_inarg_pointer++;
    LogDebug("  TS Metadata: data_type = " + std::to_string(data_type));

    long num_rows = static_cast<long>(*current_inarg_pointer++);
    LogDebug("  TS Metadata: num_rows = " + std::to_string(num_rows));

    long num_cols = static_cast<long>(*current_inarg_pointer++);
    LogDebug("  TS Metadata: num_cols = " + std::to_string(num_cols));

    long num_series = static_cast<long>(*current_inarg_pointer++);
    LogDebug("  TS Metadata: num_series = " + std::to_string(num_series));

    long num_time_points = static_cast<long>(*current_inarg_pointer++);
    LogDebug("  TS Metadata: num_time_points = " + std::to_string(num_time_points));

    // Create a NumPy array for the timestamps by wrapping the data pointer (no copy)
    npy_intp time_dims[] = { num_time_points };
    PyObject* py_timestamps = PyArray_SimpleNewFromData(1, time_dims, NPY_FLOAT64, current_inarg_pointer);

    // Advance the pointer PAST the timestamp data BEFORE reading the value data.
    current_inarg_pointer += num_time_points;

    // Create a NumPy array for the data values
    long data_size = num_time_points;
    std::vector<npy_intp> data_dims_vec;
    data_dims_vec.push_back(num_time_points);

    // Insert dimensions to match the expected shape in MarshalPythonTimeSeriesToGoldSim
    // Final shape should be (num_rows, num_cols, num_time_points)
    if (num_cols > 0) {
        data_dims_vec.insert(data_dims_vec.begin(), num_cols);
        data_size *= num_cols;
    }
    if (num_rows > 0) {
        data_dims_vec.insert(data_dims_vec.begin(), num_rows);
        data_size *= num_rows;
    }

    PyObject* py_data = PyArray_SimpleNewFromData(static_cast<int>(data_dims_vec.size()), data_dims_vec.data(), NPY_FLOAT64, current_inarg_pointer);

    current_inarg_pointer += data_size;

    // Create a Python dictionary to hold all the information
    PyObject* py_dict = PyDict_New();
    PyDict_SetItemString(py_dict, "time_basis", PyFloat_FromDouble(time_basis));
    PyDict_SetItemString(py_dict, "data_type", PyFloat_FromDouble(data_type));
    PyDict_SetItemString(py_dict, "timestamps", py_timestamps);
    PyDict_SetItemString(py_dict, "data", py_data);

    LogDebug("  TS Marshalling: Successfully created Python dictionary.");
    return py_dict;
}

// Marshal a Python dictionary (time series) to GoldSim outargs buffer
bool MarshalPythonTimeSeriesToGoldSim(PyObject* py_object, const nlohmann::json& config, double*& current_outarg_pointer, std::string& errorMessage) {
    Log("--- TimeSeriesManager: Marshalling Python Time Series to GoldSim ---");

    if (!PyDict_Check(py_object)) {
        errorMessage = "Error: Python script was expected to return a dictionary for a Time Series output.";
        return false;
    }

    // Extract values from the Python dictionary
    PyObject* py_timestamps = PyDict_GetItemString(py_object, "timestamps");
    PyObject* py_data = PyDict_GetItemString(py_object, "data");
    PyObject* py_time_basis = PyDict_GetItemString(py_object, "time_basis");
    PyObject* py_data_type = PyDict_GetItemString(py_object, "data_type");

    if (!py_timestamps || !py_data || !py_time_basis || !py_data_type) {
        errorMessage = "Error: Python dictionary for Time Series is missing required keys ('timestamps', 'data', 'time_basis', 'data_type').";
        return false;
    }

    PyArrayObject* timestamps_array = (PyArrayObject*)py_timestamps;
    PyArrayObject* data_array = (PyArrayObject*)py_data;

    // Get the dimensions directly from the shape of the returned NumPy data array
    int ndim = PyArray_NDIM(data_array);
    npy_intp* shape = PyArray_SHAPE(data_array);
    double num_rows = 0;
    double num_cols = 0;

    LogDebug("  Python->GoldSim: Array ndim = " + std::to_string(ndim));
    std::string shape_str = "(";
    for (int i = 0; i < ndim; ++i) {
        if (i > 0) shape_str += ", ";
        shape_str += std::to_string(shape[i]);
    }
    shape_str += ")";
    LogDebug("  Python->GoldSim: Array shape = " + shape_str);

    if (ndim == 1) { // Scalar Time Series
        LogDebug("  Python->GoldSim: Scalar time series, num_rows = 0, num_cols = 0");
    }
    else if (ndim == 2) { // Vector Time Series
        num_rows = static_cast<double>(shape[0]);
        LogDebug("  Python->GoldSim: Vector time series, num_rows = " + std::to_string(num_rows));
    }
    else if (ndim == 3) { // Matrix Time Series
        num_rows = static_cast<double>(shape[0]);
        num_cols = static_cast<double>(shape[1]);
        LogDebug("  Python->GoldSim: Matrix time series, num_rows = " + std::to_string(num_rows) + ", num_cols = " + std::to_string(num_cols));
    }
    else {
        errorMessage = "Error: Unsupported array dimensions: " + std::to_string(ndim);
        return false;
    }

    // Write the GoldSim Time Series header
    LogDebug("  Python->GoldSim: Writing header - ts_id=20.0, format=-3.0, time_basis=" + std::to_string(PyFloat_AsDouble(py_time_basis)) + ", data_type=" + std::to_string(PyFloat_AsDouble(py_data_type)));
    LogDebug("  Python->GoldSim: Writing header - num_rows=" + std::to_string(num_rows) + ", num_cols=" + std::to_string(num_cols) + ", num_series=1.0");
    
    *current_outarg_pointer++ = 20.0;
    *current_outarg_pointer++ = -3.0;
    *current_outarg_pointer++ = PyFloat_AsDouble(py_time_basis);
    *current_outarg_pointer++ = PyFloat_AsDouble(py_data_type);
    *current_outarg_pointer++ = num_rows;
    *current_outarg_pointer++ = num_cols;
    *current_outarg_pointer++ = 1.0; // num_series (always 1 for now)

    // Write timestamps
    npy_intp ts_array_size = PyArray_SIZE(timestamps_array);
    LogDebug("  Python->GoldSim: Writing " + std::to_string(ts_array_size) + " timestamps");
    *current_outarg_pointer++ = static_cast<double>(ts_array_size);
    
    if (PyArray_DATA(timestamps_array) == nullptr) {
        errorMessage = "Error: Timestamps array data pointer is null";
        return false;
    }
    
    memcpy(current_outarg_pointer, PyArray_DATA(timestamps_array), ts_array_size * sizeof(double));
    current_outarg_pointer += ts_array_size;

    // Write data
    npy_intp data_array_size = PyArray_SIZE(data_array);
    LogDebug("  Python->GoldSim: Writing " + std::to_string(data_array_size) + " data values");
    
    if (PyArray_DATA(data_array) == nullptr) {
        errorMessage = "Error: Data array data pointer is null";
        return false;
    }
    
    memcpy(current_outarg_pointer, PyArray_DATA(data_array), data_array_size * sizeof(double));
    current_outarg_pointer += data_array_size;

    LogDebug("  Python->GoldSim: Successfully marshalled time series to GoldSim");
    return true;
}
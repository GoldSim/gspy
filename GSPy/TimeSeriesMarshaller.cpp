#include "TimeSeriesMarshaller.h"
#include "DiagnosticsManager.h"
#include <string>

bool TimeSeriesMarshaller::UnpackTimeSeriesOutput(PyObject* pTimeSeriesDict, double* outargs, int& out_arg_count) {
    // 1. Validate that the top-level object is a dictionary
    if (!PyDict_Check(pTimeSeriesDict)) {
        DiagnosticsManager::Instance().LogError("Time series output must be a dictionary.");
        return false;
    }

    // 2. Extract and validate all required fields from the dictionary
    PyObject* pIsCalendar = PyDict_GetItemString(pTimeSeriesDict, "is_calendar");
    if (!pIsCalendar || !PyBool_Check(pIsCalendar)) {
        DiagnosticsManager::Instance().LogError("Time series dictionary missing or invalid 'is_calendar' boolean.");
        return false;
    }

    PyObject* pDataType = PyDict_GetItemString(pTimeSeriesDict, "data_type");
    if (!pDataType || !PyUnicode_Check(pDataType)) {
        DiagnosticsManager::Instance().LogError("Time series dictionary missing or invalid 'data_type' string.");
        return false;
    }

    PyObject* pTimes = PyDict_GetItemString(pTimeSeriesDict, "times");
    if (!pTimes || !PyList_Check(pTimes)) {
        DiagnosticsManager::Instance().LogError("Time series dictionary missing or invalid 'times' list.");
        return false;
    }

    PyObject* pValues = PyDict_GetItemString(pTimeSeriesDict, "values");
    if (!pValues || !PyList_Check(pValues)) {
        DiagnosticsManager::Instance().LogError("Time series dictionary missing or invalid 'values' list.");
        return false;
    }

    // 3. Validate that the time and value lists have the same length
    Py_ssize_t num_points = PyList_Size(pTimes);
    if (num_points != PyList_Size(pValues)) {
        DiagnosticsManager::Instance().LogError("'times' and 'values' lists must have the same length.");
        return false;
    }

    // 4. Convert Python data types to C++ types
    bool is_calendar = (pIsCalendar == Py_True);
    const char* data_type_str = PyUnicode_AsUTF8(pDataType);
    double data_type_num = 0.0; // Default to 'instantaneous'
    if (strcmp(data_type_str, "constant") == 0) data_type_num = 1.0;
    else if (strcmp(data_type_str, "change") == 0) data_type_num = 2.0; // Deprecated, but supported
    else if (strcmp(data_type_str, "discrete") == 0) data_type_num = 3.0; // Deprecated, but supported

    // 5. Build the 'outargs' array in the correct GoldSim format
    int index = 0;
    outargs[index++] = 20.0;         // Magic number for Time Series
    outargs[index++] = -3.0;         // Format version
    outargs[index++] = is_calendar ? 1.0 : 0.0;
    outargs[index++] = data_type_num;
    outargs[index++] = 0.0;          // Number of rows (0 for scalar)
    outargs[index++] = 0.0;          // Number of columns (0 for scalar)
    outargs[index++] = 1.0;          // Number of series (we support 1 for now)
    outargs[index++] = static_cast<double>(num_points);

    // 6. Add time points with robust checking
    for (Py_ssize_t i = 0; i < num_points; ++i) {
        PyObject* time_item = PyList_GetItem(pTimes, i);
        if (!time_item || (!PyFloat_Check(time_item) && !PyLong_Check(time_item))) {
            DiagnosticsManager::Instance().LogError("Non-numeric value found in 'times' list at index " + std::to_string(i));
            return false;
        }
        outargs[index++] = PyFloat_AsDouble(time_item);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert time value to double at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
    }

    // 7. Add data values with robust checking
    for (Py_ssize_t i = 0; i < num_points; ++i) {
        PyObject* value_item = PyList_GetItem(pValues, i);
        if (!value_item || (!PyFloat_Check(value_item) && !PyLong_Check(value_item))) {
            DiagnosticsManager::Instance().LogError("Non-numeric value found in 'values' list at index " + std::to_string(i));
            return false;
        }
        outargs[index++] = PyFloat_AsDouble(value_item);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert data value to double at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
    }

    out_arg_count = index;
    DiagnosticsManager::Instance().LogInfo("Successfully unpacked time series with " + std::to_string(num_points) + " points.");
    return true;
}

bool TimeSeriesMarshaller::PackTimeSeriesInput(const double* inargs, int input_count, PyObject* inputDict) {
    // Check if this looks like time series input data
    if (input_count < 8) {
        return false; // Not enough data for time series header
    }
    
    // Check magic number
    if (inargs[0] != 20.0) {
        return false; // Not a time series
    }
    
    DiagnosticsManager::Instance().LogDebug("Detected potential time series input with " + std::to_string(input_count) + " data points");
    
    // Extract header information
    double magic_number = inargs[0];
    double format_version = inargs[1];
    bool is_calendar = (inargs[2] != 0.0);
    double data_type = inargs[3];
    int rows = static_cast<int>(inargs[4]);
    int cols = static_cast<int>(inargs[5]);
    int series_count = static_cast<int>(inargs[6]);
    int point_count = static_cast<int>(inargs[7]);
    
    // Validate data
    if (point_count <= 0 || 8 + (2 * point_count) > input_count) {
        DiagnosticsManager::Instance().LogError("Invalid time series data: point_count=" + std::to_string(point_count) + 
                                               ", input_count=" + std::to_string(input_count));
        return false;
    }
    
    // Create time series dictionary
    PyObject* timeSeriesDict = PyDict_New();
    if (!timeSeriesDict) {
        return false;
    }
    
    // Add header information
    PyDict_SetItemString(timeSeriesDict, "is_calendar", is_calendar ? Py_True : Py_False);
    
    const char* data_type_str = "instantaneous";
    if (data_type == 1.0) data_type_str = "constant";
    else if (data_type == 2.0) data_type_str = "change";
    else if (data_type == 3.0) data_type_str = "discrete";
    PyDict_SetItemString(timeSeriesDict, "data_type", PyUnicode_FromString(data_type_str));
    
    // Create times list
    PyObject* timesList = PyList_New(point_count);
    for (int i = 0; i < point_count; ++i) {
        PyList_SetItem(timesList, i, PyFloat_FromDouble(inargs[8 + i]));
    }
    PyDict_SetItemString(timeSeriesDict, "times", timesList);
    
    // Create values list
    PyObject* valuesList = PyList_New(point_count);
    for (int i = 0; i < point_count; ++i) {
        PyList_SetItem(valuesList, i, PyFloat_FromDouble(inargs[8 + point_count + i]));
    }
    PyDict_SetItemString(timeSeriesDict, "values", valuesList);
    
    // Create header list (for compatibility)
    PyObject* headerList = PyList_New(8);
    for (int i = 0; i < 8; ++i) {
        PyList_SetItem(headerList, i, PyFloat_FromDouble(inargs[i]));
    }
    PyDict_SetItemString(timeSeriesDict, "header", headerList);
    
    // Add the time series dictionary as input1
    PyDict_SetItemString(inputDict, "input1", timeSeriesDict);
    
    DiagnosticsManager::Instance().LogInfo("Successfully packed time series input with " + std::to_string(point_count) + " data points");
    return true;
}
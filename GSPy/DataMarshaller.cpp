#include "DataMarshaller.h"
#include "DiagnosticsManager.h"
#include <cstring>

DataMarshaller::DataMarshaller() {
}

int DataMarshaller::InitNumPy() {
    static bool numpy_initialized = false;
    if (!numpy_initialized) {
        import_array();
        numpy_initialized = true;
    }
    return 0;
}

PyObject* DataMarshaller::MarshalDouble(double value) {
    return PyFloat_FromDouble(value);
}

bool DataMarshaller::UnmarshalDouble(PyObject* obj, double& out, std::string& errorMsg) {
    if (!PyFloat_Check(obj) && !PyLong_Check(obj)) {
        errorMsg = "Expected a float or int for scalar output.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    out = PyFloat_AsDouble(obj);
    if (PyErr_Occurred()) {
        errorMsg = "Failed to convert Python object to double.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        PyErr_Clear();
        return false;
    }
    return true;
}

// Enhanced method to handle both scalars and arrays
bool DataMarshaller::UnmarshalValue(PyObject* obj, double* outargs, int& index, std::string& errorMsg) {
    if (PyFloat_Check(obj) || PyLong_Check(obj)) {
        // Handle scalar value
        outargs[index++] = PyFloat_AsDouble(obj);
        if (PyErr_Occurred()) {
            errorMsg = "Failed to convert Python scalar to double.";
            PyErr_Clear();
            return false;
        }
        return true;
    }
    else if (PyList_Check(obj)) {
        // Handle list/array value
        Py_ssize_t listSize = PyList_Size(obj);
        DiagnosticsManager::Instance().LogDebug("Unpacking list with " + std::to_string(listSize) + " elements");
        
        for (Py_ssize_t i = 0; i < listSize; i++) {
            PyObject* item = PyList_GetItem(obj, i);
            if (PyFloat_Check(item) || PyLong_Check(item)) {
                outargs[index++] = PyFloat_AsDouble(item);
                if (PyErr_Occurred()) {
                    errorMsg = "Failed to convert Python list item to double.";
                    PyErr_Clear();
                    return false;
                }
            } else {
                errorMsg = "List contains non-numeric value at index " + std::to_string(i);
                return false;
            }
        }
        return true;
    }
    else {
        errorMsg = "Expected a scalar number or list of numbers.";
        return false;
    }
}

PyObject* DataMarshaller::Marshal1DArray(const double* data, npy_intp size) {
    PyObject* arr = PyArray_SimpleNew(1, &size, NPY_DOUBLE);
    if (!arr) {
        DiagnosticsManager::Instance().LogError("Failed to allocate NumPy 1D array.");
        return nullptr;
    }
    void* arr_data = PyArray_DATA((PyArrayObject*)arr);
    memcpy(arr_data, data, size * sizeof(double));
    return arr;
}

PyObject* DataMarshaller::Marshal2DArray(const double* data, npy_intp rows, npy_intp cols) {
    npy_intp dims[2] = {rows, cols};
    PyObject* arr = PyArray_SimpleNew(2, dims, NPY_DOUBLE);
    if (!arr) {
        DiagnosticsManager::Instance().LogError("Failed to allocate NumPy 2D array.");
        return nullptr;
    }
    void* arr_data = PyArray_DATA((PyArrayObject*)arr);
    memcpy(arr_data, data, rows * cols * sizeof(double));
    return arr;
}

bool DataMarshaller::Unmarshal1DArray(PyObject* obj, double* out, npy_intp expected_size, std::string& errorMsg) {
    if (!PyArray_Check(obj)) {
        errorMsg = "Expected a NumPy array for 1D array output.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    PyArrayObject* arr = (PyArrayObject*)obj;
    if (PyArray_NDIM(arr) != 1 || PyArray_TYPE(arr) != NPY_DOUBLE) {
        errorMsg = "NumPy array is not 1D or not of type double.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    npy_intp size = PyArray_DIM(arr, 0);
    if (size != expected_size) {
        errorMsg = "NumPy array size mismatch.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    void* arr_data = PyArray_DATA(arr);
    memcpy(out, arr_data, size * sizeof(double));
    return true;
}

bool DataMarshaller::Unmarshal2DArray(PyObject* obj, double* out, npy_intp expected_rows, npy_intp expected_cols, std::string& errorMsg) {
    if (!PyArray_Check(obj)) {
        errorMsg = "Expected a NumPy array for 2D array output.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    PyArrayObject* arr = (PyArrayObject*)obj;
    if (PyArray_NDIM(arr) != 2 || PyArray_TYPE(arr) != NPY_DOUBLE) {
        errorMsg = "NumPy array is not 2D or not of type double.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    npy_intp rows = PyArray_DIM(arr, 0);
    npy_intp cols = PyArray_DIM(arr, 1);
    if (rows != expected_rows || cols != expected_cols) {
        errorMsg = "NumPy array shape mismatch.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    void* arr_data = PyArray_DATA(arr);
    memcpy(out, arr_data, rows * cols * sizeof(double));
    return true;
}

PyObject* DataMarshaller::MarshalTimeSeries(const double* time, const double* value, npy_intp size, std::string& errorMsg) {
    PyObject* pandasMod = PyImport_ImportModule("pandas");
    if (pandasMod) {
        PyObject* pyTime = PyList_New(size);
        PyObject* pyValue = PyList_New(size);
        for (npy_intp i = 0; i < size; ++i) {
            PyList_SET_ITEM(pyTime, i, PyFloat_FromDouble(time[i]));
            PyList_SET_ITEM(pyValue, i, PyFloat_FromDouble(value[i]));
        }
        PyObject* pdSeriesClass = PyObject_GetAttrString(pandasMod, "Series");
        if (pdSeriesClass && PyCallable_Check(pdSeriesClass)) {
            PyObject* args = PyTuple_Pack(1, pyValue);
            PyObject* kwargs = PyDict_New();
            PyDict_SetItemString(kwargs, "index", pyTime);
            PyObject* series = PyObject_Call(pdSeriesClass, args, kwargs);
            Py_DECREF(args);
            Py_DECREF(kwargs);
            Py_DECREF(pyTime);
            Py_DECREF(pyValue);
            Py_DECREF(pdSeriesClass);
            Py_DECREF(pandasMod);
            if (!series) {
                errorMsg = "Failed to construct pandas.Series for time series.";
                DiagnosticsManager::Instance().LogError(errorMsg);
                PyErr_Clear();
                return nullptr;
            }
            return series;
        } else {
            errorMsg = "pandas.Series not found or not callable.";
            DiagnosticsManager::Instance().LogError(errorMsg);
            Py_XDECREF(pdSeriesClass);
            Py_DECREF(pyTime);
            Py_DECREF(pyValue);
            Py_DECREF(pandasMod);
        }
    } else {
        DiagnosticsManager::Instance().LogDebug("pandas not available, falling back to dict for time series marshalling.");
        PyErr_Clear();
    }
    
    // Fallback to dict
    PyObject* pyTime = PyList_New(size);
    PyObject* pyValue = PyList_New(size);
    for (npy_intp i = 0; i < size; ++i) {
        PyList_SET_ITEM(pyTime, i, PyFloat_FromDouble(time[i]));
        PyList_SET_ITEM(pyValue, i, PyFloat_FromDouble(value[i]));
    }
    PyObject* dict = PyDict_New();
    PyDict_SetItemString(dict, "time", pyTime);
    PyDict_SetItemString(dict, "value", pyValue);
    Py_DECREF(pyTime);
    Py_DECREF(pyValue);
    return dict;
}

bool DataMarshaller::UnmarshalTimeSeries(PyObject* obj, double* time, double* value, npy_intp expected_size, std::string& errorMsg) {
    // Try pandas Series/DataFrame first
    if (obj && PyObject_HasAttrString(obj, "index") && PyObject_HasAttrString(obj, "values")) {
        PyObject* pyIndex = PyObject_GetAttrString(obj, "index");
        PyObject* pyValues = PyObject_GetAttrString(obj, "values");
        PyObject* indexArr = PyArray_FROM_OTF(pyIndex, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
        PyObject* valueArr = PyArray_FROM_OTF(pyValues, NPY_DOUBLE, NPY_ARRAY_IN_ARRAY);
        Py_DECREF(pyIndex);
        Py_DECREF(pyValues);
        
        if (!indexArr || !valueArr) {
            errorMsg = "Failed to convert pandas index/values to NumPy arrays.";
            DiagnosticsManager::Instance().LogError(errorMsg);
            Py_XDECREF(indexArr);
            Py_XDECREF(valueArr);
            return false;
        }
        
        PyArrayObject* idxArr = (PyArrayObject*)indexArr;
        PyArrayObject* valArr = (PyArrayObject*)valueArr;
        npy_intp nidx = PyArray_SIZE(idxArr);
        npy_intp nval = PyArray_SIZE(valArr);
        
        if (nidx != expected_size || nval != expected_size) {
            errorMsg = "Pandas time series length mismatch.";
            DiagnosticsManager::Instance().LogError(errorMsg);
            Py_DECREF(indexArr);
            Py_DECREF(valueArr);
            return false;
        }
        
        memcpy(time, PyArray_DATA(idxArr), expected_size * sizeof(double));
        memcpy(value, PyArray_DATA(valArr), expected_size * sizeof(double));
        Py_DECREF(indexArr);
        Py_DECREF(valueArr);
        return true;
    }
    
    // Try dict format
    if (PyDict_Check(obj)) {
        PyObject* pyTime = PyDict_GetItemString(obj, "time");
        PyObject* pyValue = PyDict_GetItemString(obj, "value");
        if (!pyTime || !pyValue || !PyList_Check(pyTime) || !PyList_Check(pyValue)) {
            errorMsg = "Dict time series missing 'time' or 'value' lists.";
            DiagnosticsManager::Instance().LogError(errorMsg);
            return false;
        }
        
        npy_intp ntime = (npy_intp)PyList_Size(pyTime);
        npy_intp nvalue = (npy_intp)PyList_Size(pyValue);
        if (ntime != expected_size || nvalue != expected_size) {
            errorMsg = "Dict time/value list length mismatch.";
            DiagnosticsManager::Instance().LogError(errorMsg);
            return false;
        }
        
        for (npy_intp i = 0; i < expected_size; ++i) {
            PyObject* t = PyList_GetItem(pyTime, i);
            PyObject* v = PyList_GetItem(pyValue, i);
            if (!PyFloat_Check(t) && !PyLong_Check(t)) {
                errorMsg = "Non-numeric time value in time series.";
                DiagnosticsManager::Instance().LogError(errorMsg);
                return false;
            }
            if (!PyFloat_Check(v) && !PyLong_Check(v)) {
                errorMsg = "Non-numeric value in time series.";
                DiagnosticsManager::Instance().LogError(errorMsg);
                return false;
            }
            time[i] = PyFloat_AsDouble(t);
            value[i] = PyFloat_AsDouble(v);
        }
        return true;
    }
    
    errorMsg = "Unsupported time series format: expected pandas Series/DataFrame or dict with 'time' and 'value'.";
    DiagnosticsManager::Instance().LogError(errorMsg);
    return false;
}

PyObject* DataMarshaller::MarshalInputs(const GoldSimInputs& inputs) {
    PyObject* dict = PyDict_New();
    PyDict_SetItemString(dict, "scalar", MarshalDouble(inputs.scalar));
    PyDict_SetItemString(dict, "array", Marshal1DArray(inputs.array, inputs.array_size));
    return dict;
}

bool DataMarshaller::UnmarshalOutputs(PyObject* pyDict, GoldSimOutputs& outputs, std::string& errorMsg) {
    if (!PyDict_Check(pyDict)) {
        errorMsg = "Expected Python dict for outputs.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    
    PyObject* pyScalar = PyDict_GetItemString(pyDict, "scalar");
    PyObject* pyArray = PyDict_GetItemString(pyDict, "array");
    if (!pyScalar || !pyArray) {
        errorMsg = "Missing 'scalar' or 'array' in Python output dict.";
        DiagnosticsManager::Instance().LogError(errorMsg);
        return false;
    }
    
    if (!UnmarshalDouble(pyScalar, outputs.scalar, errorMsg)) return false;
    if (!Unmarshal1DArray(pyArray, outputs.array, outputs.array_size, errorMsg)) return false;
    
    return true;
}

// Phase 3a: Multiple I/O marshalling implementations
PyObject* DataMarshaller::PackArguments(const double* inargs, int input_count) {
    PyObject* inputDict = PyDict_New();
    if (!inputDict) {
        DiagnosticsManager::Instance().LogError("Failed to create input dictionary");
        return nullptr;
    }
    
    // Check if this might be a time series input
    // Time series format: 8 header values + N pairs of (time, value)
    // So total count should be 8 + 2*N, where N >= 1
    if (input_count >= 10 && (input_count - 8) % 2 == 0) {
        int num_points = (input_count - 8) / 2;
        DiagnosticsManager::Instance().LogDebug("Detected potential time series input with " + 
                                               std::to_string(num_points) + " data points");
        
        // Try to parse as time series
        if (PackTimeSeriesInput(inargs, input_count, inputDict)) {
            return inputDict;
        } else {
            DiagnosticsManager::Instance().LogDebug("Time series parsing failed, falling back to scalar inputs");
        }
    }
    
    // Default behavior: pack as individual scalar inputs
    for (int i = 0; i < input_count; i++) {
        std::string keyName = "input" + std::to_string(i + 1); // input1, input2, input3
        PyObject* value = MarshalDouble(inargs[i]);
        if (value) {
            PyDict_SetItemString(inputDict, keyName.c_str(), value);
            Py_DECREF(value);
        } else {
            DiagnosticsManager::Instance().LogError("Failed to marshal input " + std::to_string(i + 1));
            Py_DECREF(inputDict);
            return nullptr;
        }
    }
    
    return inputDict;
}

bool DataMarshaller::UnpackResult(PyObject* pResult, double* outargs, int output_count) {
    if (!PyDict_Check(pResult)) {
        DiagnosticsManager::Instance().LogError("Python function did not return a dictionary");
        return false;
    }
    
    int outputIndex = 0;
    
    // Iterate through all keys in the result dictionary
    PyObject* key, * value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(pResult, &pos, &key, &value)) {
        if (!PyUnicode_Check(key)) {
            DiagnosticsManager::Instance().LogError("Non-string key found in Python output dictionary");
            return false;
        }
        
        const char* keyStr = PyUnicode_AsUTF8(key);
        std::string keyName(keyStr);
        
        // Check if this is a 1D lookup table output
        if (keyName.find("goldsim_lookup_table_1d_") == 0) {
            DiagnosticsManager::Instance().LogDebug("Processing 1D lookup table output: " + keyName);
            if (!UnpackLookupTable1D(value, outargs, outputIndex)) {
                DiagnosticsManager::Instance().LogError("Failed to unpack 1D lookup table: " + keyName);
                return false;
            }
        }
        // Check if this is a time series output
        else if (keyName.find("goldsim_timeseries_") == 0) {
            DiagnosticsManager::Instance().LogDebug("Processing time series output: " + keyName);
            
            // Debug: Check what fields are present
            if (PyDict_Check(value)) {
                PyObject* timesField = PyDict_GetItemString(value, "times");
                PyObject* valuesField = PyDict_GetItemString(value, "values");
                PyObject* isCalendarField = PyDict_GetItemString(value, "is_calendar");
                PyObject* dataTypeField = PyDict_GetItemString(value, "data_type");
                PyObject* seriesDataField = PyDict_GetItemString(value, "series_data");
                DiagnosticsManager::Instance().LogDebug("Dict check - times: " + 
                    std::string(timesField ? "present" : "missing") + 
                    ", values: " + std::string(valuesField ? "present" : "missing") +
                    ", is_calendar: " + std::string(isCalendarField ? "present" : "missing") +
                    ", data_type: " + std::string(dataTypeField ? "present" : "missing") +
                    ", series_data: " + std::string(seriesDataField ? "present" : "missing"));
            }
            
            // Use the new UnpackTimeSeriesOutput function for the flat structure
            // This function expects 'times', 'values', 'is_calendar', 'data_type' at top level
            DiagnosticsManager::Instance().LogDebug("Using UnpackTimeSeriesOutput for flat structure");
            int temp_count = 0;
            if (!UnpackTimeSeriesOutput(value, &outargs[outputIndex], temp_count)) {
                DiagnosticsManager::Instance().LogError("Failed to unpack time series: " + keyName);
                return false;
            }
            outputIndex += temp_count;
        }
        // Check if this is a regular output (scalar or array)
        else if (keyName.find("output") == 0) {
            std::string errorMsg;
            if (!UnmarshalValue(value, outargs, outputIndex, errorMsg)) {
                DiagnosticsManager::Instance().LogError("Failed to unmarshal " + keyName + ": " + errorMsg);
                return false;
            }
        }
        else {
            DiagnosticsManager::Instance().LogDebug("Ignoring unknown output key: " + keyName);
        }
    }
    
    return true;
}

// Phase 3b: Time series marshalling implementations
bool DataMarshaller::UnpackTimeSeries(PyObject* tsDict, double* outargs, int& index) {
    if (!PyDict_Check(tsDict)) {
        DiagnosticsManager::Instance().LogError("Time series output is not a dictionary");
        return false;
    }
    
    // 1. Start with magic number 20 for time series
    outargs[index++] = 20.0;
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": Magic number 20");
    
    // 2. Add format number -3 (required by GoldSim manual)
    outargs[index++] = -3.0;
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": Format number -3");
    
    // Extract header information
    PyObject* isCalendarObj = PyDict_GetItemString(tsDict, "is_calendar");
    PyObject* dataTypeObj = PyDict_GetItemString(tsDict, "data_type");
    PyObject* seriesDataObj = PyDict_GetItemString(tsDict, "series_data");
    PyObject* numRowsObj = PyDict_GetItemString(tsDict, "num_rows");
    PyObject* numColsObj = PyDict_GetItemString(tsDict, "num_columns");
    
    if (!isCalendarObj || !dataTypeObj || !seriesDataObj) {
        DiagnosticsManager::Instance().LogError("Time series missing required fields: is_calendar, data_type, or series_data");
        return false;
    }
    
    // Extract header values
    double isCalendar = PyFloat_AsDouble(isCalendarObj);
    double dataType = PyFloat_AsDouble(dataTypeObj);
    double numRows = numRowsObj ? PyFloat_AsDouble(numRowsObj) : 0.0;  // Default to 0 for scalar
    double numCols = numColsObj ? PyFloat_AsDouble(numColsObj) : 0.0;  // Default to 0 for scalar/vector
    
    if (PyErr_Occurred()) {
        DiagnosticsManager::Instance().LogError("Failed to extract time series header values");
        PyErr_Clear();
        return false;
    }
    
    // 3. Calendar-based index
    outargs[index++] = isCalendar;
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": Calendar=" + std::to_string(isCalendar));
    
    // 4. Data type index  
    outargs[index++] = dataType;
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": DataType=" + std::to_string(dataType));
    
    // 5. Number of rows (0 for scalar time series)
    outargs[index++] = numRows;
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": Rows=" + std::to_string(numRows));
    
    // 6. Number of columns (0 for scalar and vector time series)
    outargs[index++] = numCols;
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": Cols=" + std::to_string(numCols));
    
    DiagnosticsManager::Instance().LogDebug("Added header: is_calendar=" + std::to_string(isCalendar) + 
                                          ", data_type=" + std::to_string(dataType) +
                                          ", rows=" + std::to_string(numRows) +
                                          ", cols=" + std::to_string(numCols));
    
    // Process series data
    if (!UnpackSeriesData(seriesDataObj, outargs, index)) {
        return false;
    }
    
    return true;
}

bool DataMarshaller::UnpackSeriesData(PyObject* seriesArray, double* outargs, int& index) {
    if (!PyList_Check(seriesArray)) {
        DiagnosticsManager::Instance().LogError("series_data is not a list");
        return false;
    }
    
    Py_ssize_t numSeries = PyList_Size(seriesArray);
    outargs[index++] = static_cast<double>(numSeries);
    DiagnosticsManager::Instance().LogInfo("Index " + std::to_string(index - 1) + ": NumSeries=" + std::to_string(numSeries));
    
    for (Py_ssize_t i = 0; i < numSeries; i++) {
        PyObject* series = PyList_GetItem(seriesArray, i);
        if (!PyDict_Check(series)) {
            DiagnosticsManager::Instance().LogError("Series " + std::to_string(i) + " is not a dictionary");
            return false;
        }
        
        PyObject* timesObj = PyDict_GetItemString(series, "times");
        PyObject* valuesObj = PyDict_GetItemString(series, "values");
        
        if (!timesObj || !valuesObj) {
            DiagnosticsManager::Instance().LogError("Series " + std::to_string(i) + " missing 'times' or 'values'");
            return false;
        }
        
        if (!PyList_Check(timesObj)) {
            DiagnosticsManager::Instance().LogError("Series " + std::to_string(i) + " 'times' is not a list");
            return false;
        }
        
        Py_ssize_t numTimes = PyList_Size(timesObj);
        
        // Determine if values is a simple list (scalar) or list of lists (vector)
        bool isVector = false;
        Py_ssize_t vectorSize = 1;
        
        if (PyList_Check(valuesObj)) {
            Py_ssize_t numValues = PyList_Size(valuesObj);
            if (numValues > 0) {
                PyObject* firstValue = PyList_GetItem(valuesObj, 0);
                if (PyList_Check(firstValue)) {
                    // This is a vector (list of lists)
                    // vectorSize = number of vector elements (outer list size)
                    // Each inner list should have numTimes elements
                    isVector = true;
                    vectorSize = numValues;  // Number of vector elements
                }
            }
        }
        
        // Add series metadata
        outargs[index++] = static_cast<double>(numTimes);
        
        // Only add vector size for vector time series, not for scalar
        if (isVector) {
            outargs[index++] = static_cast<double>(vectorSize);
            DiagnosticsManager::Instance().LogDebug("Series " + std::to_string(i) + ": " + 
                                                  std::to_string(numTimes) + " times, vector size " + 
                                                  std::to_string(vectorSize));
        } else {
            DiagnosticsManager::Instance().LogDebug("Series " + std::to_string(i) + ": " + 
                                                  std::to_string(numTimes) + " times, scalar data");
        }
        
        // Add time values
        for (Py_ssize_t t = 0; t < numTimes; t++) {
            PyObject* timeValue = PyList_GetItem(timesObj, t);
            double time = PyFloat_AsDouble(timeValue);
            if (PyErr_Occurred()) {
                DiagnosticsManager::Instance().LogError("Failed to extract time value at index " + std::to_string(t));
                PyErr_Clear();
                return false;
            }
            outargs[index++] = time;
        }
        
        // Add data values
        if (isVector) {
            // Vector data: values[vector_element][time]
            DiagnosticsManager::Instance().LogDebug("Processing vector data: " + std::to_string(vectorSize) + 
                                                   " elements, " + std::to_string(numTimes) + " times each");
            for (Py_ssize_t v = 0; v < vectorSize; v++) {
                PyObject* vectorAtTime = PyList_GetItem(valuesObj, v);
                if (!PyList_Check(vectorAtTime)) {
                    DiagnosticsManager::Instance().LogError("Vector element " + std::to_string(v) + " is not a list");
                    return false;
                }
                
                Py_ssize_t innerSize = PyList_Size(vectorAtTime);
                if (innerSize != numTimes) {
                    DiagnosticsManager::Instance().LogError("Vector element " + std::to_string(v) + 
                                                           " has " + std::to_string(innerSize) + 
                                                           " values, expected " + std::to_string(numTimes));
                    return false;
                }
                
                for (Py_ssize_t t = 0; t < numTimes; t++) {
                    PyObject* valueAtTime = PyList_GetItem(vectorAtTime, t);
                    double value = PyFloat_AsDouble(valueAtTime);
                    if (PyErr_Occurred()) {
                        DiagnosticsManager::Instance().LogError("Failed to extract vector value at [" + 
                                                              std::to_string(v) + "][" + std::to_string(t) + "]");
                        PyErr_Clear();
                        return false;
                    }
                    if (index >= 100) {  // Safety check to prevent array overflow
                        DiagnosticsManager::Instance().LogError("Output array index exceeded maximum (100)");
                        return false;
                    }
                    outargs[index++] = value;
                    DiagnosticsManager::Instance().LogDebug("Added vector value [" + std::to_string(v) + 
                                                           "][" + std::to_string(t) + "] = " + std::to_string(value) + 
                                                           " at index " + std::to_string(index - 1));
                }
            }
        } else {
            // Scalar data: values[time]
            DiagnosticsManager::Instance().LogDebug("Processing scalar data: " + std::to_string(numTimes) + " times");
            for (Py_ssize_t t = 0; t < numTimes; t++) {
                PyObject* valueAtTime = PyList_GetItem(valuesObj, t);
                double value = PyFloat_AsDouble(valueAtTime);
                if (PyErr_Occurred()) {
                    DiagnosticsManager::Instance().LogError("Failed to extract scalar value at time " + std::to_string(t));
                    PyErr_Clear();
                    return false;
                }
                if (index >= 100) {  // Safety check to prevent array overflow
                    DiagnosticsManager::Instance().LogError("Output array index exceeded maximum (100)");
                    return false;
                }
                outargs[index++] = value;
                DiagnosticsManager::Instance().LogDebug("Added scalar value [" + std::to_string(t) + 
                                                       "] = " + std::to_string(value) + 
                                                       " at index " + std::to_string(index - 1));
            }
        }
    }
    
    return true;
}

int DataMarshaller::CalculateTimeSeriesSize(PyObject* tsDict) {
    // This method can be used to pre-calculate the required outargs array size
    // For now, we'll implement a conservative estimate
    // In practice, GoldSim should allocate sufficient space based on the output type
    return 1000; // Conservative estimate - can be refined later
}

// Specialized method for single time series output
bool DataMarshaller::UnpackSingleTimeSeries(PyObject* pResult, double* outargs, int maxSize) {
    if (!PyDict_Check(pResult)) {
        DiagnosticsManager::Instance().LogError("Python function did not return a dictionary");
        return false;
    }
    
    // Look for the time series output
    PyObject* key, * value;
    Py_ssize_t pos = 0;
    
    while (PyDict_Next(pResult, &pos, &key, &value)) {
        if (!PyUnicode_Check(key)) {
            continue;
        }
        
        const char* keyStr = PyUnicode_AsUTF8(key);
        std::string keyName(keyStr);
        
        // Check if this is a time series output
        if (keyName.find("goldsim_timeseries_") == 0) {
            DiagnosticsManager::Instance().LogDebug("Processing single time series output: " + keyName);
            int index = 0;
            bool result = UnpackTimeSeries(value, outargs, index);
            if (result) {
                DiagnosticsManager::Instance().LogInfo("Successfully unpacked time series with " + 
                                                      std::to_string(index) + " elements");
            }
            return result;
        }
    }
    
    DiagnosticsManager::Instance().LogError("No time series output found in Python result");
    return false;
}

// 1D Lookup Table unpacking implementation
bool DataMarshaller::UnpackLookupTable1D(PyObject* tableDict, double* outargs, int& index) {
    if (!PyDict_Check(tableDict)) {
        DiagnosticsManager::Instance().LogError("1D lookup table output is not a dictionary");
        return false;
    }
    
    // Extract independent_values and dependent_values from the dictionary
    PyObject* independentObj = PyDict_GetItemString(tableDict, "independent_values");
    PyObject* dependentObj = PyDict_GetItemString(tableDict, "dependent_values");
    
    if (!independentObj || !dependentObj) {
        DiagnosticsManager::Instance().LogError("1D lookup table missing 'independent_values' or 'dependent_values'");
        return false;
    }
    
    if (!PyList_Check(independentObj) || !PyList_Check(dependentObj)) {
        DiagnosticsManager::Instance().LogError("1D lookup table values must be lists");
        return false;
    }
    
    Py_ssize_t independentSize = PyList_Size(independentObj);
    Py_ssize_t dependentSize = PyList_Size(dependentObj);
    
    if (independentSize != dependentSize) {
        DiagnosticsManager::Instance().LogError("1D lookup table independent and dependent value lists must have the same size");
        return false;
    }
    
    // Build the GoldSim 1D table format: [1.0, N, ...independent_values..., ...dependent_values...]
    
    // 1. Number of dimensions (must be 1.0 for 1D table)
    outargs[index++] = 1.0;
    
    // 2. Number of points in the table
    outargs[index++] = static_cast<double>(independentSize);
    
    // 3. Independent values (e.g., row headers)
    for (Py_ssize_t i = 0; i < independentSize; i++) {
        PyObject* item = PyList_GetItem(independentObj, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            DiagnosticsManager::Instance().LogError("Non-numeric independent value at index " + std::to_string(i));
            return false;
        }
        outargs[index++] = PyFloat_AsDouble(item);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert independent value at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
    }
    
    // 4. Dependent values (the table results)
    for (Py_ssize_t i = 0; i < dependentSize; i++) {
        PyObject* item = PyList_GetItem(dependentObj, i);
        if (!PyFloat_Check(item) && !PyLong_Check(item)) {
            DiagnosticsManager::Instance().LogError("Non-numeric dependent value at index " + std::to_string(i));
            return false;
        }
        outargs[index++] = PyFloat_AsDouble(item);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert dependent value at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
    }
    
    DiagnosticsManager::Instance().LogInfo("Successfully unpacked 1D lookup table with " + 
                                          std::to_string(independentSize) + " points");
    
    return true;
}

// Time series input packing implementation
bool DataMarshaller::PackTimeSeriesInput(const double* inargs, int input_count, PyObject* inputDict) {
    // GoldSim time series input format:
    // [0-7]: 8 header values (metadata)
    // [8+]: N pairs of (time, value) data points
    
    if (input_count < 10) {
        return false; // Need at least 8 header + 1 data pair
    }
    
    int num_points = (input_count - 8) / 2;
    if (8 + 2 * num_points != input_count) {
        return false; // Invalid format
    }
    
    // Extract header information (first 8 values)
    // Note: The exact meaning of these header values would need to be documented
    // For now, we'll store them for potential future use
    PyObject* headerList = PyList_New(8);
    if (!headerList) {
        return false;
    }
    
    for (int i = 0; i < 8; i++) {
        PyObject* headerValue = PyFloat_FromDouble(inargs[i]);
        if (!headerValue) {
            Py_DECREF(headerList);
            return false;
        }
        PyList_SetItem(headerList, i, headerValue); // Steals reference
    }
    
    // Extract time and value data
    PyObject* timesList = PyList_New(num_points);
    PyObject* valuesList = PyList_New(num_points);
    
    if (!timesList || !valuesList) {
        Py_DECREF(headerList);
        Py_XDECREF(timesList);
        Py_XDECREF(valuesList);
        return false;
    }
    
    // Parse time series data: all times first, then all values
    // Times: indices 8 to 8+num_points-1
    // Values: indices 8+num_points to 8+2*num_points-1
    
    // Extract times
    for (int i = 0; i < num_points; i++) {
        int time_index = 8 + i;
        PyObject* timeValue = PyFloat_FromDouble(inargs[time_index]);
        
        if (!timeValue) {
            Py_DECREF(headerList);
            Py_DECREF(timesList);
            Py_DECREF(valuesList);
            return false;
        }
        
        PyList_SetItem(timesList, i, timeValue); // Steals reference
    }
    
    // Extract values
    for (int i = 0; i < num_points; i++) {
        int value_index = 8 + num_points + i;
        PyObject* dataValue = PyFloat_FromDouble(inargs[value_index]);
        
        if (!dataValue) {
            Py_DECREF(headerList);
            Py_DECREF(timesList);
            Py_DECREF(valuesList);
            return false;
        }
        
        PyList_SetItem(valuesList, i, dataValue); // Steals reference
    }
    
    // Create the time series dictionary
    PyObject* timeSeriesDict = PyDict_New();
    if (!timeSeriesDict) {
        Py_DECREF(headerList);
        Py_DECREF(timesList);
        Py_DECREF(valuesList);
        return false;
    }
    
    // Add the data to the time series dictionary
    PyDict_SetItemString(timeSeriesDict, "header", headerList);
    PyDict_SetItemString(timeSeriesDict, "times", timesList);
    PyDict_SetItemString(timeSeriesDict, "values", valuesList);
    
    // Add the time series as input1
    PyDict_SetItemString(inputDict, "input1", timeSeriesDict);
    
    // Clean up references
    Py_DECREF(headerList);
    Py_DECREF(timesList);
    Py_DECREF(valuesList);
    Py_DECREF(timeSeriesDict);
    
    DiagnosticsManager::Instance().LogInfo("Successfully packed time series input with " + 
                                          std::to_string(num_points) + " data points");
    
    return true;
}

// This is the complete, corrected function for DataMarshaller.cpp
bool DataMarshaller::UnpackTimeSeriesOutput(PyObject* pTimeSeriesDict, double* outargs, int& out_arg_count) {
    // --- 1. Extract required fields from the Python dictionary ---
    PyObject* pIsCalendar = PyDict_GetItemString(pTimeSeriesDict, "is_calendar");
    PyObject* pDataType = PyDict_GetItemString(pTimeSeriesDict, "data_type");
    PyObject* pTimes = PyDict_GetItemString(pTimeSeriesDict, "times");
    PyObject* pValues = PyDict_GetItemString(pTimeSeriesDict, "values");

    if (!pIsCalendar || !pDataType || !pTimes || !pValues) {
        DiagnosticsManager::Instance().LogError("Time series dictionary missing required keys ('is_calendar', 'data_type', 'times', or 'values').");
        return false;
    }

    // --- 2. Convert Python objects to C++ types ---
    bool is_calendar = (pIsCalendar == Py_True);
    const char* data_type_str = PyUnicode_AsUTF8(pDataType);

    // Convert data_type string to its corresponding number for GoldSim
    double data_type_num = 0.0; // Default to 'instantaneous'
    if (strcmp(data_type_str, "constant") == 0) data_type_num = 1.0;
    else if (strcmp(data_type_str, "change") == 0) data_type_num = 2.0;
    else if (strcmp(data_type_str, "discrete") == 0) data_type_num = 3.0;

    Py_ssize_t num_points = PyList_Size(pTimes);

    // --- 3. Build the 'outargs' array in the correct GoldSim format ---
    int index = 0;
    outargs[index++] = 20.0;         // Magic number for Time Series
    outargs[index++] = -3.0;         // Format version
    outargs[index++] = is_calendar ? 1.0 : 0.0;
    outargs[index++] = data_type_num;
    outargs[index++] = 0.0;          // Number of rows (0 for scalar)
    outargs[index++] = 0.0;          // Number of columns (0 for scalar)
    outargs[index++] = 1.0;          // Number of series (we support 1 for now)
    outargs[index++] = static_cast<double>(num_points);

    // Add time points
    for (Py_ssize_t i = 0; i < num_points; ++i) {
        outargs[index++] = PyFloat_AsDouble(PyList_GetItem(pTimes, i));
    }

    // Add data values
    for (Py_ssize_t i = 0; i < num_points; ++i) {
        outargs[index++] = PyFloat_AsDouble(PyList_GetItem(pValues, i));
    }

    out_arg_count = index; // The total number of values written
    return true;
}

// Simple time series output packing (for scalar-to-timeseries example)
bool DataMarshaller::UnpackSimpleTimeSeries(PyObject* tsDict, double* outargs, int& index) {
    if (!PyDict_Check(tsDict)) {
        DiagnosticsManager::Instance().LogError("Simple time series output is not a dictionary");
        return false;
    }
    
    // 1. Check for and extract the 'is_calendar' boolean
    PyObject* isCalendarObj = PyDict_GetItemString(tsDict, "is_calendar");
    if (!isCalendarObj || !PyBool_Check(isCalendarObj)) {
        DiagnosticsManager::Instance().LogError("'is_calendar' key is missing or not a boolean");
        return false;
    }
    bool is_calendar = (isCalendarObj == Py_True);
    
    // 2. Check for and extract the 'data_type' string
    PyObject* dataTypeObj = PyDict_GetItemString(tsDict, "data_type");
    if (!dataTypeObj || !PyUnicode_Check(dataTypeObj)) {
        DiagnosticsManager::Instance().LogError("'data_type' key is missing or not a string");
        return false;
    }
    
    // 3. Check for and extract the 'times' list
    PyObject* timesObj = PyDict_GetItemString(tsDict, "times");
    if (!timesObj || !PyList_Check(timesObj)) {
        DiagnosticsManager::Instance().LogError("'times' key is missing or not a list");
        return false;
    }
    
    // 4. Check for and extract the 'values' list
    PyObject* valuesObj = PyDict_GetItemString(tsDict, "values");
    if (!valuesObj || !PyList_Check(valuesObj)) {
        DiagnosticsManager::Instance().LogError("'values' key is missing or not a list");
        return false;
    }
    
    // 5. Validate that times and values have the same length
    Py_ssize_t numPoints = PyList_Size(timesObj);
    Py_ssize_t numValues = PyList_Size(valuesObj);
    
    if (numPoints != numValues) {
        DiagnosticsManager::Instance().LogError("Simple time series times and values lists must have the same length");
        return false;
    }
    
    // Convert data_type string to numeric value
    double dataTypeValue = 0.0; // Default to instantaneous
    const char* dataTypeStr = PyUnicode_AsUTF8(dataTypeObj);
    if (strcmp(dataTypeStr, "instantaneous") == 0) {
        dataTypeValue = 0.0;
    } else if (strcmp(dataTypeStr, "constant") == 0) {
        dataTypeValue = 1.0;
    } else if (strcmp(dataTypeStr, "linear") == 0) {
        dataTypeValue = 2.0;
    } else if (strcmp(dataTypeStr, "step") == 0) {
        dataTypeValue = 3.0;
    } else {
        DiagnosticsManager::Instance().LogInfo("Unknown data_type: " + std::string(dataTypeStr) + ", using instantaneous");
    }
    
    // Build the GoldSim Time Series format according to the specification
    // Index 0: Magic number 20.0
    outargs[index++] = 20.0;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": Magic number 20.0");
    
    // Index 1: Format version -3.0
    outargs[index++] = -3.0;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": Format version -3.0");
    
    // Index 2: is_calendar (0.0 for elapsed time, 1.0 for dates)
    outargs[index++] = is_calendar ? 1.0 : 0.0;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": is_calendar=" + std::to_string(is_calendar ? 1.0 : 0.0));
    
    // Index 3: data_type (0 for instantaneous, 1 for constant, etc.)
    outargs[index++] = dataTypeValue;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": data_type=" + std::to_string(dataTypeValue));
    
    // Index 4: Number of rows (0 for scalar time series)
    outargs[index++] = 0.0;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": rows=0.0");
    
    // Index 5: Number of columns (0 for scalar time series)
    outargs[index++] = 0.0;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": columns=0.0");
    
    // Index 6: Number of series (1 for single series)
    outargs[index++] = 1.0;
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": num_series=1.0");
    
    // Index 7: Number of time points
    outargs[index++] = static_cast<double>(numPoints);
    DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": num_points=" + std::to_string(numPoints));
    
    // Indices 8 to 8+N-1: Time points
    for (Py_ssize_t i = 0; i < numPoints; i++) {
        PyObject* timeValue = PyList_GetItem(timesObj, i);
        if (!PyFloat_Check(timeValue) && !PyLong_Check(timeValue)) {
            DiagnosticsManager::Instance().LogError("Non-numeric time value at index " + std::to_string(i));
            return false;
        }
        double time = PyFloat_AsDouble(timeValue);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert time value at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
        outargs[index++] = time;
        DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": time[" + std::to_string(i) + "]=" + std::to_string(time));
    }
    
    // Indices 8+N to 8+2N-1: Data values
    for (Py_ssize_t i = 0; i < numPoints; i++) {
        PyObject* dataValue = PyList_GetItem(valuesObj, i);
        if (!PyFloat_Check(dataValue) && !PyLong_Check(dataValue)) {
            DiagnosticsManager::Instance().LogError("Non-numeric data value at index " + std::to_string(i));
            return false;
        }
        double value = PyFloat_AsDouble(dataValue);
        if (PyErr_Occurred()) {
            DiagnosticsManager::Instance().LogError("Failed to convert data value at index " + std::to_string(i));
            PyErr_Clear();
            return false;
        }
        outargs[index++] = value;
        DiagnosticsManager::Instance().LogDebug("Index " + std::to_string(index - 1) + ": value[" + std::to_string(i) + "]=" + std::to_string(value));
    }
    
    DiagnosticsManager::Instance().LogInfo("Successfully unpacked simple time series with " + 
                                          std::to_string(numPoints) + " data points, total elements: " + 
                                          std::to_string(index));
    
    return true;
}
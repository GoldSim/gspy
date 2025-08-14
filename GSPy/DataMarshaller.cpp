#include "DataMarshaller.h"
#include "TimeSeriesMarshaller.h"
#include "LookupTableMarshaller.h"
#include "DiagnosticsManager.h"
#include <string>

// Constructor
DataMarshaller::DataMarshaller() {
    // Helper classes are initialized automatically as member variables.
}

// Destructor
DataMarshaller::~DataMarshaller() {
    // No manual cleanup needed.
}

/**
 * @brief Unpacks the result dictionary from a Python script into the GoldSim output array.
 * This function acts as a dispatcher, delegating the unpacking of complex types
 * to specialized helper classes based on key prefixes in the Python dictionary.
 * @param pResult The dictionary object returned from the Python script.
 * @param outargs Pointer to the double array where GoldSim expects the output.
 * @param output_count The maximum size of the outargs array.
 * @return True on success, false on failure.
 */
bool DataMarshaller::UnpackResult(PyObject* pResult, double* outargs, int output_count) {
    if (!PyDict_Check(pResult)) {
        DiagnosticsManager::Instance().LogError("GSPy script must return a dictionary.");
        return false;
    }

    PyObject* key, * value;
    Py_ssize_t pos = 0;
    int current_output_index = 0;

    // Iterate through all items in the returned dictionary
    while (PyDict_Next(pResult, &pos, &key, &value)) {
        if (!PyUnicode_Check(key)) {
            // CORRECTED LINE: Replaced LogWarning with LogDebug
            DiagnosticsManager::Instance().LogDebug("Ignoring non-string key in Python output dictionary.");
            continue;
        }

        const char* keyStr = PyUnicode_AsUTF8(key);
        std::string keyName(keyStr);

        // Delegate to the appropriate marshaller based on the key prefix
        if (keyName.rfind("goldsim_timeseries_", 0) == 0) {
            DiagnosticsManager::Instance().LogDebug("Delegating to TimeSeriesMarshaller for key: " + keyName);
            int written_count = 0;
            if (!tsMarshaller.UnpackTimeSeriesOutput(value, &outargs[current_output_index], written_count)) {
                DiagnosticsManager::Instance().LogError("Failed to unpack time series for key: " + keyName);
                return false;
            }
            current_output_index += written_count;

        } else if (keyName.rfind("goldsim_lookup_table_1d_", 0) == 0) {
            DiagnosticsManager::Instance().LogDebug("Delegating to LookupTableMarshaller for key: " + keyName);
            int written_count = 0;
            if (!ltMarshaller.UnpackLookupTable1D(value, &outargs[current_output_index], written_count)) {
                 DiagnosticsManager::Instance().LogError("Failed to unpack 1D lookup table for key: " + keyName);
                 return false;
            }
            current_output_index += written_count;

        } else {
            // Handle simple scalar outputs (like output1, output2, etc.) or vector outputs
            if (PyFloat_Check(value) || PyLong_Check(value)) {
                // Handle single scalar value
                int outputIndex = -1;
                if (keyName.rfind("output", 0) == 0 && keyName.length() > 6) {
                    try {
                        outputIndex = std::stoi(keyName.substr(6)) - 1; // Convert "output1" to index 0
                    } catch (...) {
                        outputIndex = -1;
                    }
                }
                
                if (outputIndex >= 0 && outputIndex < output_count) {
                    double outputValue = PyFloat_AsDouble(value);
                    if (PyErr_Occurred()) {
                        DiagnosticsManager::Instance().LogError("Failed to convert output value to double for key: " + keyName);
                        PyErr_Clear();
                        return false;
                    }
                    
                    outargs[outputIndex] = outputValue;
                    DiagnosticsManager::Instance().LogDebug("Set " + keyName + " = " + std::to_string(outputValue) + " at index " + std::to_string(outputIndex));
                } else {
                    DiagnosticsManager::Instance().LogDebug("Invalid output index for key: " + keyName);
                }
            } else if (PyList_Check(value)) {
                // Handle vector output (list of values)
                Py_ssize_t listSize = PyList_Size(value);
                DiagnosticsManager::Instance().LogDebug("Unpacking list with " + std::to_string(listSize) + " elements");
                
                if (current_output_index + listSize > output_count) {
                    DiagnosticsManager::Instance().LogError("Vector output exceeds allocated size");
                    return false;
                }
                
                for (Py_ssize_t i = 0; i < listSize; ++i) {
                    PyObject* listItem = PyList_GetItem(value, i);
                    if (listItem && (PyFloat_Check(listItem) || PyLong_Check(listItem))) {
                        double itemValue = PyFloat_AsDouble(listItem);
                        if (PyErr_Occurred()) {
                            DiagnosticsManager::Instance().LogError("Failed to convert list item to double");
                            PyErr_Clear();
                            return false;
                        }
                        
                        outargs[current_output_index] = itemValue;
                        DiagnosticsManager::Instance().LogDebug("Set " + keyName + "[" + std::to_string(i) + "] = " + 
                                                               std::to_string(itemValue) + " at index " + std::to_string(current_output_index));
                        current_output_index++;
                    } else {
                        DiagnosticsManager::Instance().LogError("Non-numeric value in output list");
                        return false;
                    }
                }
            } else {
                DiagnosticsManager::Instance().LogDebug("Ignoring unrecognized output key: " + keyName);
            }
        }

        // Safety check to prevent buffer overflow
        if (current_output_index > output_count) {
            DiagnosticsManager::Instance().LogError("Output data exceeds the allocated size for GoldSim element.");
            return false;
        }
    }

    return true;
}
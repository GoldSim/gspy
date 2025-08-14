#pragma once

#include <string>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include <Python.h>

class DataMarshaller {
public:
    DataMarshaller();
    
    static int InitNumPy();
    
    // Scalar marshalling
    PyObject* MarshalDouble(double value);
    bool UnmarshalDouble(PyObject* obj, double& out, std::string& errorMsg);
    
    // Enhanced marshalling for scalars and arrays
    bool UnmarshalValue(PyObject* obj, double* outargs, int& index, std::string& errorMsg);
    
    // Phase 3a: Multiple I/O marshalling
    PyObject* PackArguments(const double* inargs, int input_count);
    bool UnpackResult(PyObject* pResult, double* outargs, int output_count);
    
    // Time series input marshalling
    bool PackTimeSeriesInput(const double* inargs, int input_count, PyObject* inputDict);
    
    // Phase 3b: Time series marshalling
    bool UnpackTimeSeries(PyObject* tsDict, double* outargs, int& index);
    bool UnpackSeriesData(PyObject* seriesArray, double* outargs, int& index);
    int CalculateTimeSeriesSize(PyObject* tsDict);
    
    // Specialized method for single time series output
    bool UnpackSingleTimeSeries(PyObject* pResult, double* outargs, int maxSize);
    
    // Simple time series output packing (for scalar-to-timeseries example)
    bool UnpackSimpleTimeSeries(PyObject* tsDict, double* outargs, int& index);
    
    // Corrected time series output unpacking function
    bool UnpackTimeSeriesOutput(PyObject* pTimeSeriesDict, double* outargs, int& out_arg_count);
    
    // 1D Lookup table marshalling
    bool UnpackLookupTable1D(PyObject* tableDict, double* outargs, int& index);
    
    // Array marshalling
    PyObject* Marshal1DArray(const double* data, npy_intp size);
    PyObject* Marshal2DArray(const double* data, npy_intp rows, npy_intp cols);
    bool Unmarshal1DArray(PyObject* obj, double* out, npy_intp expected_size, std::string& errorMsg);
    bool Unmarshal2DArray(PyObject* obj, double* out, npy_intp expected_rows, npy_intp expected_cols, std::string& errorMsg);
    
    // Time series marshalling
    PyObject* MarshalTimeSeries(const double* time, const double* value, npy_intp size, std::string& errorMsg);
    bool UnmarshalTimeSeries(PyObject* obj, double* time, double* value, npy_intp expected_size, std::string& errorMsg);
    
    // Helper structs for GoldSim I/O
    struct GoldSimInputs {
        double scalar;
        const double* array;
        npy_intp array_size;
    };
    
    struct GoldSimOutputs {
        double scalar;
        double* array;
        npy_intp array_size;
    };
    
    // High-level marshalling for GoldSim
    PyObject* MarshalInputs(const GoldSimInputs& inputs);
    bool UnmarshalOutputs(PyObject* pyDict, GoldSimOutputs& outputs, std::string& errorMsg);
};
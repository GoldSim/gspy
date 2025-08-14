#pragma once
#include <Python.h>
#include <string>
#include "TimeSeriesMarshaller.h"
#include "LookupTableMarshaller.h"

// Forward declaration of structs if they are not included from another header
struct GoldSimInputs;
struct GoldSimOutputs;

class DataMarshaller {
public:
    DataMarshaller();
    ~DataMarshaller();

    // Main dispatcher function
    bool UnpackResult(PyObject* pResult, double* outargs, int output_count);
    
    // You can also declare other simple marshalling functions here if needed
    // PyObject* MarshalDouble(double value);
    // bool UnmarshalDouble(PyObject* obj, double& out, std::string& errorMsg);

private:
    // Member variables for the helper classes
    TimeSeriesMarshaller tsMarshaller;
    LookupTableMarshaller ltMarshaller;
};
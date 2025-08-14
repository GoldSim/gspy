#pragma once
#include <Python.h>

class TimeSeriesMarshaller {
public:
    bool UnpackTimeSeriesOutput(PyObject* pTimeSeriesDict, double* outargs, int& out_arg_count);
    bool PackTimeSeriesInput(const double* inargs, int input_count, PyObject* inputDict);
};

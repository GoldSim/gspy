#pragma once
#include <Python.h>

class LookupTableMarshaller {
public:
    bool UnpackLookupTable1D(PyObject* tableDict, double* outargs, int& index);
};

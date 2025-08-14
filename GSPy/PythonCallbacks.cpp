#include "common.h"
#include <Python.h>
#include <cstdio>

// Global error message storage for Python module
static const char* pyModuleErrorMessage = "Python Module Error! Details were not specified.";

// Python-callable C functions
static PyObject* GSPy_setErrorMessage(PyObject *self, PyObject *msg) {
    int ok = PyArg_ParseTuple(msg, "s", &pyModuleErrorMessage);
    if (ok) {
        printf("An error message was relayed from the Python module\n");
    }
    Py_RETURN_NONE;
}

static PyObject* GSPy_add(PyObject *self, PyObject *args) {
    double num1, num2;
    int ok = PyArg_ParseTuple(args, "dd", &num1, &num2);
    if (ok) {
        double result = num1 + num2;
        return PyFloat_FromDouble(result);
    }
    Py_RETURN_NONE;
}

// PyMethodDef array - defines the methods available to Python
static PyMethodDef GSPy_methods[] = {
    {"setErrorMessage", (PyCFunction) GSPy_setErrorMessage, METH_VARARGS, 
     "Allows a Python module to return an error message"},
    {"add", (PyCFunction) GSPy_add, METH_VARARGS, 
     "Adds two numbers together"},
    {NULL, NULL, 0, NULL}  // Sentinel
};

// PyModuleDef struct - defines the module
static struct PyModuleDef GSPyModule = {
    PyModuleDef_HEAD_INIT,
    "GSPy",           // module name
    NULL,             // module documentation
    -1,               // size of per-interpreter state of the module
    GSPy_methods      // method table
};

// Module initialization function - this is called when Python imports the module
PyMODINIT_FUNC PyInit_GSPy(void) {
    return PyModule_Create(&GSPyModule);
}
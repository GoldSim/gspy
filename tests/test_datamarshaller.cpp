#include "common.h"
#include "gtest/gtest.h"
#include "DataMarshaller.h"
#include "DiagnosticsManager.h"
#include <Python.h>
#include <numpy/arrayobject.h>
#include <string>
#include <vector>

class DataMarshallerTest : public ::testing::Test {
protected:
    void SetUp() override {
        Py_Initialize();
        DataMarshaller::InitNumPy();
    }
    void TearDown() override {
        Py_Finalize();
    }
    DataMarshaller marshaller;
};

// 1. Scalar and Array Marshalling
TEST_F(DataMarshallerTest, ScalarRoundTrip) {
    double value = 42.5;
    PyObject* pyVal = marshaller.MarshalDouble(value);
    double out = 0.0;
    std::string errorMsg;
    ASSERT_TRUE(marshaller.UnmarshalDouble(pyVal, out, errorMsg));
    EXPECT_DOUBLE_EQ(out, value);
    Py_DECREF(pyVal);
}

TEST_F(DataMarshallerTest, Array1DRoundTrip) {
    std::vector<double> arr = {1.0, 2.0, 3.0};
    PyObject* pyArr = marshaller.Marshal1DArray(arr.data(), arr.size());
    std::vector<double> out(arr.size());
    std::string errorMsg;
    ASSERT_TRUE(marshaller.Unmarshal1DArray(pyArr, out.data(), arr.size(), errorMsg));
    for (size_t i = 0; i < arr.size(); ++i) {
        EXPECT_DOUBLE_EQ(out[i], arr[i]);
    }
    Py_DECREF(pyArr);
}

TEST_F(DataMarshallerTest, Array2DRoundTrip) {
    double arr[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
    PyObject* pyArr = marshaller.Marshal2DArray(&arr[0][0], 2, 2);
    double out[4] = {0};
    std::string errorMsg;
    ASSERT_TRUE(marshaller.Unmarshal2DArray(pyArr, out, 2, 2, errorMsg));
    for (int i = 0; i < 4; ++i) {
        EXPECT_DOUBLE_EQ(out[i], ((double*)&arr[0][0])[i]);
    }
    Py_DECREF(pyArr);
}

TEST_F(DataMarshallerTest, ArrayShapeMismatch) {
    std::vector<double> arr = {1.0, 2.0, 3.0};
    PyObject* pyArr = marshaller.Marshal1DArray(arr.data(), arr.size());
    std::vector<double> out(arr.size() + 1); // Wrong size
    std::string errorMsg;
    EXPECT_FALSE(marshaller.Unmarshal1DArray(pyArr, out.data(), arr.size() + 1, errorMsg));
    EXPECT_FALSE(errorMsg.empty());
    Py_DECREF(pyArr);
}

// 2. Time Series Marshalling
TEST_F(DataMarshallerTest, TimeSeriesUnpackSuccess) {
    PyObject* tsDict = PyDict_New();
    PyDict_SetItemString(tsDict, "is_calendar", Py_True);
    PyDict_SetItemString(tsDict, "data_type", PyUnicode_FromString("instantaneous"));
    PyObject* times = PyList_New(2);
    PyObject* values = PyList_New(2);
    PyList_SetItem(times, 0, PyFloat_FromDouble(1.0));
    PyList_SetItem(times, 1, PyFloat_FromDouble(2.0));
    PyList_SetItem(values, 0, PyFloat_FromDouble(10.0));
    PyList_SetItem(values, 1, PyFloat_FromDouble(20.0));
    PyDict_SetItemString(tsDict, "times", times);
    PyDict_SetItemString(tsDict, "values", values);
    double outargs[12] = {0};
    int out_arg_count = 0;
    ASSERT_TRUE(marshaller.UnpackTimeSeriesOutput(tsDict, outargs, out_arg_count));
    EXPECT_EQ(outargs[0], 20.0); // Magic number
    EXPECT_EQ(outargs[1], -3.0); // Version
    EXPECT_EQ(outargs[2], 1.0); // is_calendar
    EXPECT_EQ(outargs[3], 0.0); // data_type instantaneous
    EXPECT_EQ(outargs[7], 2.0); // num_times
    EXPECT_EQ(outargs[8], 1.0); // time 1
    EXPECT_EQ(outargs[9], 2.0); // time 2
    EXPECT_EQ(outargs[10], 10.0); // value 1
    EXPECT_EQ(outargs[11], 20.0); // value 2
    Py_DECREF(tsDict);
}

TEST_F(DataMarshallerTest, TimeSeriesHeaderFlags) {
    const char* types[] = {"instantaneous", "constant", "change", "discrete"};
    double expected[] = {0.0, 1.0, 2.0, 3.0};
    for (int i = 0; i < 4; ++i) {
        PyObject* tsDict = PyDict_New();
        PyDict_SetItemString(tsDict, "is_calendar", Py_False);
        PyDict_SetItemString(tsDict, "data_type", PyUnicode_FromString(types[i]));
        PyObject* times = PyList_New(1);
        PyObject* values = PyList_New(1);
        PyList_SetItem(times, 0, PyFloat_FromDouble(1.0));
        PyList_SetItem(values, 0, PyFloat_FromDouble(10.0));
        PyDict_SetItemString(tsDict, "times", times);
        PyDict_SetItemString(tsDict, "values", values);
        double outargs[10] = {0};
        int out_arg_count = 0;
        ASSERT_TRUE(marshaller.UnpackTimeSeriesOutput(tsDict, outargs, out_arg_count));
        EXPECT_EQ(outargs[2], 0.0); // is_calendar
        EXPECT_EQ(outargs[3], expected[i]); // data_type
        Py_DECREF(tsDict);
    }
}

// 3. Lookup Table Marshalling
TEST_F(DataMarshallerTest, LookupTable1DUnpack) {
    PyObject* tableDict = PyDict_New();
    PyDict_SetItemString(tableDict, "dimension", PyLong_FromLong(1));
    PyDict_SetItemString(tableDict, "num_points", PyLong_FromLong(3));
    PyObject* indep = PyList_New(3);
    PyObject* dep = PyList_New(3);
    PyList_SetItem(indep, 0, PyFloat_FromDouble(1.0));
    PyList_SetItem(indep, 1, PyFloat_FromDouble(2.0));
    PyList_SetItem(indep, 2, PyFloat_FromDouble(3.0));
    PyList_SetItem(dep, 0, PyFloat_FromDouble(10.0));
    PyList_SetItem(dep, 1, PyFloat_FromDouble(20.0));
    PyList_SetItem(dep, 2, PyFloat_FromDouble(30.0));
    PyDict_SetItemString(tableDict, "independent", indep);
    PyDict_SetItemString(tableDict, "dependent", dep);
    double outargs[10] = {0};
    int index = 0;
    ASSERT_TRUE(marshaller.UnpackLookupTable1D(tableDict, outargs, index));
    EXPECT_EQ(outargs[0], 1.0); // dimension
    EXPECT_EQ(outargs[1], 3.0); // num_points
    EXPECT_EQ(outargs[2], 1.0); // indep 1
    EXPECT_EQ(outargs[3], 2.0); // indep 2
    EXPECT_EQ(outargs[4], 3.0); // indep 3
    EXPECT_EQ(outargs[5], 10.0); // dep 1
    EXPECT_EQ(outargs[6], 20.0); // dep 2
    EXPECT_EQ(outargs[7], 30.0); // dep 3
    Py_DECREF(tableDict);
}

class DataMarshallerErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        Py_Initialize();
        DataMarshaller::InitNumPy();
        DiagnosticsManager::Instance().Clear();
    }
    void TearDown() override {
        Py_Finalize();
    }
    DataMarshaller marshaller;
};

TEST_F(DataMarshallerErrorTest, ScriptException) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    ASSERT_NE(module, nullptr);
    PyObject* func = PyObject_GetAttrString(module, "fail_with_exception");
    ASSERT_NE(func, nullptr);
    PyObject* result = PyObject_CallObject(func, nullptr);
    EXPECT_EQ(result, nullptr); // Should be null due to exception
    std::string lastError = DiagnosticsManager::Instance().GetLastError();
    EXPECT_NE(lastError.find("ValueError"), std::string::npos);
    EXPECT_NE(lastError.find("This is a test exception"), std::string::npos);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

TEST_F(DataMarshallerErrorTest, ReturnStringInsteadOfDouble) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    PyObject* func = PyObject_GetAttrString(module, "return_string");
    PyObject* result = PyObject_CallObject(func, nullptr);
    double out = 0.0;
    std::string errorMsg;
    EXPECT_FALSE(marshaller.UnmarshalDouble(result, out, errorMsg));
    EXPECT_FALSE(errorMsg.empty());
    Py_XDECREF(result);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

TEST_F(DataMarshallerErrorTest, ReturnScalarInsteadOfArray) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    PyObject* func = PyObject_GetAttrString(module, "return_scalar_for_array");
    PyObject* result = PyObject_CallObject(func, nullptr);
    std::vector<double> out(3);
    std::string errorMsg;
    EXPECT_FALSE(marshaller.Unmarshal1DArray(result, out.data(), 3, errorMsg));
    EXPECT_FALSE(errorMsg.empty());
    Py_XDECREF(result);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

TEST_F(DataMarshallerErrorTest, NonNumericArrayElement) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    PyObject* func = PyObject_GetAttrString(module, "return_non_numeric_array");
    PyObject* result = PyObject_CallObject(func, nullptr);
    std::vector<double> out(3);
    std::string errorMsg;
    EXPECT_FALSE(marshaller.Unmarshal1DArray(result, out.data(), 3, errorMsg));
    EXPECT_FALSE(errorMsg.empty());
    Py_XDECREF(result);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

TEST_F(DataMarshallerErrorTest, TimeSeriesMissingKey) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    PyObject* func = PyObject_GetAttrString(module, "malformed_timeseries_missing_key");
    PyObject* result = PyObject_CallObject(func, nullptr);
    double outargs[10] = {0};
    int out_arg_count = 0;
    EXPECT_FALSE(marshaller.UnpackTimeSeriesOutput(result, outargs, out_arg_count));
    std::string lastError = DiagnosticsManager::Instance().GetLastError();
    EXPECT_NE(lastError.find("missing required keys"), std::string::npos);
    Py_XDECREF(result);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

TEST_F(DataMarshallerErrorTest, TimeSeriesMismatchedLength) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    PyObject* func = PyObject_GetAttrString(module, "malformed_timeseries_mismatched_length");
    PyObject* result = PyObject_CallObject(func, nullptr);
    double outargs[10] = {0};
    int out_arg_count = 0;
    EXPECT_FALSE(marshaller.UnpackTimeSeriesOutput(result, outargs, out_arg_count));
    std::string lastError = DiagnosticsManager::Instance().GetLastError();
    EXPECT_NE(lastError.find("must have the same length"), std::string::npos);
    Py_XDECREF(result);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

TEST_F(DataMarshallerErrorTest, LookupTableMismatchedLength) {
    PyObject* module = PyImport_ImportModule("fail_with_exception");
    PyObject* func = PyObject_GetAttrString(module, "malformed_lookup_table_mismatched_length");
    PyObject* result = PyObject_CallObject(func, nullptr);
    double outargs[10] = {0};
    int out_arg_count = 0;
    EXPECT_FALSE(marshaller.UnpackLookupTable1D(result, outargs, out_arg_count));
    std::string lastError = DiagnosticsManager::Instance().GetLastError();
    EXPECT_NE(lastError.find("must have the same length"), std::string::npos);
    Py_XDECREF(result);
    Py_XDECREF(func);
    Py_XDECREF(module);
}

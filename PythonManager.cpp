#include <cstdlib>

// Includes and macro definitions for Python and NumPy
#ifdef _DEBUG
    #undef _DEBUG
    #include <Python.h>
    #define _DEBUG
#else
    #include <Python.h>
#endif
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/ndarrayobject.h>

#include "PythonManager.h"
#include "GSPY_Error.h"
#include <fstream>
#include <vector>
#include "json.hpp"
#include "Logger.h"
#include "TimeSeriesManager.h"
#include "ConfigManager.h"
#include "LookupTableManager.h"

using json = nlohmann::json;

static json config;
static PyObject* pModule = nullptr;
static PyObject* pFunc = nullptr;

// =================================================================
// Python-Callable Logging Function
// =================================================================

// Python-callable logging function (static - internal use only)
static PyObject* PythonLog(PyObject* self, PyObject* args) {
    const char* message;
    int level = 2; // Default to INFO level
    
    // Parse arguments: message (required), level (optional)
    if (!PyArg_ParseTuple(args, "s|i", &message, &level)) {
        return nullptr;
    }
    
    // Call the appropriate logging function based on level
    switch (level) {
        case 0:
            LogError(std::string(message));
            break;
        case 1:
            LogWarning(std::string(message));
            break;
        case 2:
            LogInfo(std::string(message));
            break;
        case 3:
            LogDebug(std::string(message));
            break;
        default:
            LogInfo(std::string(message)); // Default to INFO
            break;
    }
    
    Py_RETURN_NONE;
}

// Method definition for the gspy module
static PyMethodDef GSPyMethods[] = {
    {"log", PythonLog, METH_VARARGS, "Write a message to the GSPy log file"},
    {nullptr, nullptr, 0, nullptr} // Sentinel
};

// Module definition
static struct PyModuleDef gspymodule = {
    PyModuleDef_HEAD_INIT,
    "gspy",     // module name
    nullptr,    // module documentation
    -1,         // size of per-interpreter state, -1 if global
    GSPyMethods
};

// Module initialization function
PyObject* PyInit_gspy(void) {
    return PyModule_Create(&gspymodule);
}

// =================================================================
// ## Specialized Cohorts (Private Helper Functions) ##
// =================================================================
static std::string read_config() {
    std::string config_path = GetConfigFilename();
    LogDebug("Reading config file: " + config_path);
    std::ifstream f(config_path);
    if (f.is_open()) {
        LogDebug("Config file opened successfully.");
        std::string file_contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.clear();
        f.seekg(0, std::ios::beg); // Reset file pointer for parsing
        try {
            config = json::parse(file_contents);
            LogDebug("Config file parsed successfully.");
            return ""; // Success
        }
        catch (json::parse_error& e) {
            LogError("JSON parse error: " + std::string(e.what()));
            return "JSON parse error: " + std::string(e.what()); // Failure
        }
    }
    else {
        LogError("Error: Could not open config.json at path: " + config_path);
        return "Error: Could not open config.json"; // Failure
    }
}

// Helper function to calculate total elements from dimensions array
static int calculate_total_elements(const json& dimensions) {
    if (dimensions.empty()) return 1; // Scalar
    int total = 1;
    for (int dim : dimensions) {
        total *= dim;
    }
    return total;
}

// --- Initializes the NumPy C-API ---
static bool initialize_numpy(std::string& errorMessage) {
    LogDebug("Initializing NumPy C-API...");
    if (_import_array() < 0) {
        errorMessage = "Error: Could not initialize NumPy C-API.";
        LogError(errorMessage);
        PyErr_Print();
        return false;
    }
    LogDebug("NumPy C-API initialized successfully.");
    return true;
}

// --- Adds the current directory to Python's search path ---
static bool add_script_path_to_sys() {
    LogDebug("Adding current directory to Python sys.path...");
    PyObject* sys = PyImport_ImportModule("sys");
    PyObject* path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("."));
    Py_DECREF(path);
    Py_DECREF(sys);
    LogDebug("Current directory added to path.");
    return true;
}

// --- Loads the user's script and gets the target function ---
static bool load_script_and_function(std::string& errorMessage) {
    std::string script_path_full = config["script_path"];
    std::string function_name = config["function_name"];
    LogDebug("Script path from config: " + script_path_full);
    LogDebug("Function name from config: " + function_name);

    std::string script_path_module = script_path_full;
    size_t dot_pos = script_path_module.find(".py");
    if (dot_pos != std::string::npos) {
        script_path_module = script_path_module.substr(0, dot_pos);
    }

    LogDebug("Attempting to import Python module: " + script_path_module);
    pModule = PyImport_ImportModule(script_path_module.c_str());

    if (pModule != nullptr) {
        LogDebug("Module imported successfully.");
        LogDebug("Attempting to get function '" + function_name + "' from module...");
        pFunc = PyObject_GetAttrString(pModule, function_name.c_str());

        if (pFunc && PyCallable_Check(pFunc)) {
            LogDebug("Function found successfully.");
            return true;
        }
        else {
            errorMessage = "Error: Cannot find function '" + function_name + "' in script '" + script_path_full + "'.";
            LogError(errorMessage);
            return false;
        }
    }
    else {
        PyErr_Print();
        errorMessage = "Error: Failed to load Python script '" + script_path_full + "'. Check for syntax errors or missing dependencies in the script.";
        LogError(errorMessage);
        return false;
    }
}

// This function prepares the tuple of arguments to be sent to Python.
static PyObject* MarshalInputsToPython(const json& inputs_config, double* inargs) {
    LogDebug("Preparing " + std::to_string(inputs_config.size()) + " input argument(s) for Python.");
    PyObject* pArgs = PyTuple_New(inputs_config.size());
    double* current_inarg_pointer = inargs; // Use a pointer we can advance

    for (int i = 0; i < inputs_config.size(); ++i) {
        const auto& input = inputs_config[i];
        std::string type = input["type"];
        PyObject* pValue = nullptr;

        LogDebug("  Input #" + std::to_string(i) + ": Type='" + type + "'");

        if (type == "timeseries") {
            // Delegate to our specialist
            pValue = MarshalGoldSimTimeSeriesToPython(current_inarg_pointer, input);
        }
        else if (type == "scalar") {
            pValue = PyFloat_FromDouble(*current_inarg_pointer);
            current_inarg_pointer += 1; // Advance pointer by 1
        }
        else { // Vector or Matrix
            int num_elements = calculate_total_elements(input["dimensions"]);
            const auto& dims_json = input["dimensions"];
            std::vector<npy_intp> dims_vec;
            for (const auto& dim : dims_json) {
                dims_vec.push_back(dim.get<npy_intp>());
            }
            pValue = PyArray_SimpleNewFromData(static_cast<int>(dims_vec.size()), dims_vec.data(), NPY_FLOAT64, current_inarg_pointer);
            current_inarg_pointer += num_elements; // Advance pointer by the size of the array
        }
        PyTuple_SetItem(pArgs, i, pValue); // Steals reference to pValue
    }
    return pArgs;
}

// This function unpacks the tuple of results from Python and copies the data back.
static bool MarshalOutputsToCpp(PyObject* pResultTuple, const json& outputs_config, double* outargs, std::string& errorMessage) {
    if (!pResultTuple || !PyTuple_Check(pResultTuple)) {
        PyErr_Print();
        errorMessage = "Error: Python call failed or did not return a tuple.";
        LogError(errorMessage);
        return false;
    }

    LogDebug("Python call successful. Processing " + std::to_string(PyTuple_Size(pResultTuple)) + " result(s).");
    double* current_outarg_pointer = outargs;

    for (Py_ssize_t i = 0; i < PyTuple_Size(pResultTuple); ++i) {
        PyObject* pItem = PyTuple_GetItem(pResultTuple, i);
        const auto& output_config = outputs_config[i];
        std::string type = output_config["type"];
        LogDebug("  Output #" + std::to_string(i) + ": Type='" + type + "'");

        if (type == "timeseries") {
            if (!MarshalPythonTimeSeriesToGoldSim(pItem, output_config, current_outarg_pointer, errorMessage)) {
                Py_DECREF(pResultTuple);
                return false;
            }
        }
        else if (type == "table") {
            if (!MarshalPythonLookupTableToGoldSim(pItem, output_config, current_outarg_pointer, errorMessage)) {
                Py_DECREF(pResultTuple);
                return false;
            }
        }
        else if (PyArray_Check(pItem)) { // Handle Vector or Matrix
            int expected_size = calculate_total_elements(output_config["dimensions"]);
            memcpy(current_outarg_pointer, PyArray_DATA((PyArrayObject*)pItem), expected_size * sizeof(double));
            current_outarg_pointer += expected_size;
        }
        else { // Handle Scalar
            *current_outarg_pointer = PyFloat_AsDouble(pItem);
            current_outarg_pointer += 1;
        }
    }
    Py_DECREF(pResultTuple);
    return true;
}

// =================================================================
// ## The Commander (Public Functions) ##
// =================================================================

// --- The main InitializePython function ---
bool InitializePython(std::string& errorMessage) {
    Log("--- Initializing Python Manager ---");

    if (config.empty()) {
        errorMessage = read_config();
        if (!errorMessage.empty()) {
            LogError("Error reading config: " + errorMessage);
            return false;
        }
        LogInfo("Config read successfully.");
    }

    if (!Py_IsInitialized()) {
        LogInfo("Python interpreter is not initialized. Initializing now...");

        PyConfig py_config;
        PyConfig_InitPythonConfig(&py_config);

        // --- REVERTED LOGIC: Get Python Home from the config file ---
        if (config.contains("python_path")) {
            std::string python_home = config["python_path"];
            LogDebug("Using python_path from config: " + python_home);
            PyStatus status = PyConfig_SetBytesString(&py_config, &py_config.home, python_home.c_str());
            if (PyStatus_Exception(status)) {
                errorMessage = "Error: Failed to set Python Home from config path.";
                LogError(errorMessage);
                PyConfig_Clear(&py_config);
                return false;
            }
        }
        else {
            errorMessage = "Error: 'python_path' key is missing from the config file.";
            LogError(errorMessage);
            PyConfig_Clear(&py_config);
            return false;
        }

        // Register the gspy module before initializing Python
        if (PyImport_AppendInittab("gspy", PyInit_gspy) == -1) {
            errorMessage = "Error: Failed to register gspy module with Python.";
            LogError(errorMessage);
            return false;
        }

        PyStatus status = Py_InitializeFromConfig(&py_config);
        PyConfig_Clear(&py_config);
        if (PyStatus_Exception(status)) {
            errorMessage = "Error: Py_InitializeFromConfig failed.";
            LogError(errorMessage);
            return false;
        }

        if (!initialize_numpy(errorMessage)) return false;
        if (!add_script_path_to_sys()) return false;
        if (!load_script_and_function(errorMessage)) return false;
    }
    else {
        LogInfo("Python interpreter is already initialized.");
    }

    Log("--- Python Manager initialization successful ---");
    return true;
}

void FinalizePython() {
    // LOGGING: Announce the start of the cleanup process.
    LogInfo("--- Finalizing Python Manager ---");

    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);

    if (Py_IsInitialized()) {
        // LOGGING: Confirm that we are shutting down the interpreter.
        LogInfo("Shutting down Python interpreter.");
        Py_Finalize();
    }
    else {
        // LOGGING: Note if no shutdown was necessary.
        LogInfo("Python interpreter was not initialized. No cleanup needed.");
    }
}

int GetNumberOfInputs() {
    if (config.empty()) return 0;

    int total_inputs = 0;
    for (const auto& input : config["inputs"]) {
        // If any input is a dynamic type, we must return -1.
        if (input["type"] == "timeseries") {
            LogDebug("GetNumberOfInputs detected a dynamic time series. Returning -1.");
            return -1;
        }
        total_inputs += calculate_total_elements(input["dimensions"]);
    }

    LogDebug("GetNumberOfInputs calculated a total of: " + std::to_string(total_inputs));
    return total_inputs;
}

int GetNumberOfOutputs() {
    if (config.empty()) return 0;

    int total_outputs = 0;
    for (const auto& output : config["outputs"]) {
        std::string type = output["type"];

        if (type == "timeseries") {
            // Your correct, detailed calculation for Time Series
            int max_points = output.value("max_points", 1);
            const auto& dims = output["dimensions"];
            int data_multiplier = 1;
            // Note: A scalar TS has 0 dims, a vector has 1, a matrix has 2.
            // The data array shape is (rows, cols, time_points).
            // data_multiplier here will be rows * cols.
            if (!dims.empty()) data_multiplier *= dims[0].get<int>(); // rows
            if (dims.size() > 1) data_multiplier *= dims[1].get<int>(); // cols

            // Total size = 8 metadata doubles + N timestamps + (N * rows * cols) data values
            total_outputs += 8 + max_points + (max_points * data_multiplier);
        }
        else if (type == "table") {
            // For tables, the structure is more complex, so we rely on the user's max_elements estimate.
            total_outputs += output.value("max_elements", 1);
        }
        else {
            // Standard calculation for fixed-size arrays (scalar, vector, matrix)
            total_outputs += calculate_total_elements(output["dimensions"]);
        }
    }

    LogDebug("GetNumberOfOutputs calculated a total of: " + std::to_string(total_outputs));
    return total_outputs;
}

// --- The ExecuteCalculation function is now a clean, high-level commander ---
void ExecuteCalculation(double* inargs, double* outargs, std::string& errorMessage) {
    LogInfo("--- Executing Calculation Cycle ---");
    if (!pFunc) {
        errorMessage = "Error: Python function not loaded.";
        LogError(errorMessage);
        return;
    }

    // 1. Delegate argument preparation
    PyObject* pArgs = MarshalInputsToPython(config["inputs"], inargs);
    if (!pArgs) {
        errorMessage = "Error: Failed to marshal inputs for Python.";
        LogError(errorMessage);
        return;
    }

    // 2. Call the Python function
    LogDebug("Calling Python function...");
    PyObject* pResultTuple = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    // 3. Delegate result processing
    if (!MarshalOutputsToCpp(pResultTuple, config["outputs"], outargs, errorMessage)) {
        // MarshalOutputsToCpp handles its own error logging and Py_DECREF
        return;
    }

    LogInfo("--- Calculation Cycle Complete ---");
}
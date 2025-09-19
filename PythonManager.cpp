#include <cstdlib>
#include <cmath>

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
#include <map>
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
// ## Specialized Cohorts (Private Helper Functions) ##
// =================================================================
static std::string read_config() {
    std::string config_path = GetConfigFilename();
    Log("Reading config file: " + config_path);
    std::ifstream f(config_path);
    if (f.is_open()) {
        Log("Config file opened successfully.");
        std::string file_contents((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.clear();
        f.seekg(0, std::ios::beg); // Reset file pointer for parsing
        try {
            config = json::parse(file_contents);
            Log("Config file parsed successfully.");
            return ""; // Success
        }
        catch (json::parse_error& e) {
            Log("JSON parse error: " + std::string(e.what()));
            return "JSON parse error: " + std::string(e.what()); // Failure
        }
    }
    else {
        Log("Error: Could not open config.json at path: " + config_path);
        return "Error: Could not open config.json"; // Failure
    }
}

// Validates the dimensions_from configuration
static std::string validate_dimensions_from() {
    Log("Validating dimensions_from configuration...");
    
    if (!config.contains("inputs")) {
        return ""; // No inputs to validate
    }

    const auto& inputs = config["inputs"];
    
    // Create a map of input names to their types for dependency validation
    std::map<std::string, std::string> input_name_to_type;
    for (const auto& input : inputs) {
        if (input.contains("name") && input.contains("type")) {
            input_name_to_type[input["name"]] = input["type"];
        }
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        const auto& input = inputs[i];
        
        if (!input.contains("dimensions_from")) {
            continue; // No dimensions_from to validate
        }

        std::string input_name = input.value("name", "unnamed_input_" + std::to_string(i));
        std::string input_type = input.value("type", "");

        // Rule 1: dimensions_from is only valid for vector and matrix inputs
        if (input_type != "vector" && input_type != "matrix") {
            return "Error: 'dimensions_from' key is only valid for 'vector' and 'matrix' input types. Found on input '" + 
                   input_name + "' of type '" + input_type + "'.";
        }

        const auto& dimensions_from = input["dimensions_from"];
        
        // Rule 2: dimensions_from must be an array
        if (!dimensions_from.is_array()) {
            return "Error: 'dimensions_from' must be an array of strings for input '" + input_name + "'.";
        }

        // Rule 3: Array size must match type requirements
        size_t expected_size = (input_type == "vector") ? 1 : 2;
        if (dimensions_from.size() != expected_size) {
            return "Error: 'dimensions_from' array for " + input_type + " input '" + input_name + 
                   "' must contain exactly " + std::to_string(expected_size) + " element(s). Found " + 
                   std::to_string(dimensions_from.size()) + ".";
        }

        // Rule 4: All elements must be strings and reference valid scalar inputs
        for (size_t j = 0; j < dimensions_from.size(); ++j) {
            if (!dimensions_from[j].is_string()) {
                return "Error: Element " + std::to_string(j) + " in 'dimensions_from' array for input '" + 
                       input_name + "' must be a string.";
            }

            std::string referenced_name = dimensions_from[j];
            
            // Check if the referenced input exists
            auto it = input_name_to_type.find(referenced_name);
            if (it == input_name_to_type.end()) {
                return "Error: Dimension source '" + referenced_name + "' for input '" + input_name + 
                       "' does not exist in the inputs array.";
            }

            // Check if the referenced input is a scalar
            if (it->second != "scalar") {
                return "Error: Dimension source '" + referenced_name + "' for input '" + input_name + 
                       "' must be a scalar input. Found type '" + it->second + "'.";
            }
        }
    }

    // Check that dimensions_from doesn't appear in outputs
    if (config.contains("outputs")) {
        const auto& outputs = config["outputs"];
        for (size_t i = 0; i < outputs.size(); ++i) {
            const auto& output = outputs[i];
            if (output.contains("dimensions_from")) {
                std::string output_name = output.value("name", "unnamed_output_" + std::to_string(i));
                return "Error: 'dimensions_from' key is not valid for outputs. Found on output '" + output_name + "'.";
            }
        }
    }

    Log("dimensions_from validation completed successfully.");
    return ""; // Success
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
    Log("Initializing NumPy C-API...");
    if (_import_array() < 0) {
        errorMessage = "Error: Could not initialize NumPy C-API.";
        Log(errorMessage);
        PyErr_Print();
        return false;
    }
    Log("NumPy C-API initialized successfully.");
    return true;
}

// --- Adds the current directory to Python's search path ---
static bool add_script_path_to_sys() {
    Log("Adding current directory to Python sys.path...");
    PyObject* sys = PyImport_ImportModule("sys");
    PyObject* path = PyObject_GetAttrString(sys, "path");
    PyList_Append(path, PyUnicode_FromString("."));
    Py_DECREF(path);
    Py_DECREF(sys);
    Log("Current directory added to path.");
    return true;
}

// --- Loads the user's script and gets the target function ---
static bool load_script_and_function(std::string& errorMessage) {
    std::string script_path_full = config["script_path"];
    std::string function_name = config["function_name"];
    Log("Script path from config: " + script_path_full);
    Log("Function name from config: " + function_name);

    std::string script_path_module = script_path_full;
    size_t dot_pos = script_path_module.find(".py");
    if (dot_pos != std::string::npos) {
        script_path_module = script_path_module.substr(0, dot_pos);
    }

    Log("Attempting to import Python module: " + script_path_module);
    pModule = PyImport_ImportModule(script_path_module.c_str());

    if (pModule != nullptr) {
        Log("Module imported successfully.");
        Log("Attempting to get function '" + function_name + "' from module...");
        pFunc = PyObject_GetAttrString(pModule, function_name.c_str());

        if (pFunc && PyCallable_Check(pFunc)) {
            Log("Function found successfully.");
            return true;
        }
        else {
            errorMessage = "Error: Cannot find function '" + function_name + "' in script '" + script_path_full + "'.";
            Log(errorMessage);
            return false;
        }
    }
    else {
        PyErr_Print();
        errorMessage = "Error: Failed to load Python script '" + script_path_full + "'. Check for syntax errors or missing dependencies in the script.";
        Log(errorMessage);
        return false;
    }
}

// Helper function to calculate dynamic dimensions for an input
static std::vector<npy_intp> calculate_dynamic_dimensions(const json& input, const std::map<std::string, double>& scalar_values) {
    std::vector<npy_intp> dims_vec;
    
    if (!input.contains("dimensions_from")) {
        // Static dimensions
        const auto& dims_json = input["dimensions"];
        for (const auto& dim : dims_json) {
            dims_vec.push_back(dim.get<npy_intp>());
        }
        return dims_vec;
    }
    
    // Dynamic dimensions
    const auto& dimensions_from = input["dimensions_from"];
    std::string input_name = input.value("name", "unnamed");
    
    Log("    Calculating dynamic dimensions for '" + input_name + "':");
    
    for (const auto& scalar_name : dimensions_from) {
        std::string scalar_name_str = scalar_name.get<std::string>();
        
        auto it = scalar_values.find(scalar_name_str);
        if (it != scalar_values.end()) {
            double dim_value = it->second;
            
            // Safeguard: Handle invalid size values
            if (dim_value < 0 || dim_value != std::floor(dim_value)) {
                Log("      WARNING: Invalid dimension value " + std::to_string(dim_value) + 
                    " for scalar '" + scalar_name_str + "'. Treating as 0.");
                dims_vec.push_back(0);
            }
            else {
                npy_intp dim_size = static_cast<npy_intp>(dim_value);
                dims_vec.push_back(dim_size);
                Log("      '" + scalar_name_str + "' = " + std::to_string(dim_size));
            }
        }
        else {
            Log("      ERROR: Referenced scalar '" + scalar_name_str + "' not found! Using 0.");
            dims_vec.push_back(0);
        }
    }
    
    return dims_vec;
}

// Helper function to calculate total elements from dimensions vector
static int calculate_total_elements_from_vector(const std::vector<npy_intp>& dims_vec) {
    if (dims_vec.empty()) return 1; // Scalar
    int total = 1;
    for (npy_intp dim : dims_vec) {
        total *= static_cast<int>(dim);
    }
    return total;
}

// This function prepares the tuple of arguments to be sent to Python.
static PyObject* MarshalInputsToPython(const json& inputs_config, double* inargs) {
    Log("Preparing " + std::to_string(inputs_config.size()) + " input argument(s) for Python.");
    PyObject* pArgs = PyTuple_New(inputs_config.size());

    // Step 1: Collect all scalar values first (dependency-aware processing)
    std::map<std::string, double> scalar_values;
    double* read_pointer = inargs;
    
    Log("  Step 1: Collecting scalar values for dynamic sizing...");
    for (int i = 0; i < inputs_config.size(); ++i) {
        const auto& input = inputs_config[i];
        std::string type = input["type"];
        std::string name = input.value("name", "input_" + std::to_string(i));
        
        if (type == "scalar") {
            scalar_values[name] = *read_pointer;
            Log("    Scalar '" + name + "' = " + std::to_string(*read_pointer));
            read_pointer += 1;
        }
        else if (type == "timeseries") {
            // Timeseries size is handled by the specialist function, skip data reading here
            // The specialist will handle advancing the pointer internally
        }
        else { // Vector or Matrix
            // Calculate how many elements this array will consume
            int num_elements = 0;
            if (input.contains("dimensions_from")) {
                // We can't calculate this yet since we're still collecting scalars
                // We'll handle this in the second pass
            }
            else {
                // Static sizing
                num_elements = calculate_total_elements(input["dimensions"]);
                read_pointer += num_elements;
            }
        }
    }
    
    // Step 2: Calculate all array sizes now that we have scalar values
    Log("  Step 2: Calculating array sizes...");
    std::vector<int> input_sizes(inputs_config.size(), 0);
    
    for (int i = 0; i < inputs_config.size(); ++i) {
        const auto& input = inputs_config[i];
        std::string type = input["type"];
        
        if (type == "scalar") {
            input_sizes[i] = 1;
        }
        else if (type == "timeseries") {
            input_sizes[i] = -1; // Special marker for timeseries (handled separately)
        }
        else { // Vector or Matrix
            std::vector<npy_intp> dims_vec = calculate_dynamic_dimensions(input, scalar_values);
            input_sizes[i] = calculate_total_elements_from_vector(dims_vec);
            Log("    Input " + std::to_string(i) + " size: " + std::to_string(input_sizes[i]) + " elements");
        }
    }

    // Step 3: Process all inputs with known sizes
    Log("  Step 3: Creating Python objects...");
    double* current_inarg_pointer = inargs;
    
    for (int i = 0; i < inputs_config.size(); ++i) {
        const auto& input = inputs_config[i];
        std::string type = input["type"];
        std::string name = input.value("name", "input_" + std::to_string(i));
        PyObject* pValue = nullptr;

        Log("    Processing '" + name + "' (Type: " + type + ")");

        if (type == "timeseries") {
            // Delegate to specialist - it handles its own pointer advancement
            pValue = MarshalGoldSimTimeSeriesToPython(current_inarg_pointer, input);
        }
        else if (type == "scalar") {
            pValue = PyFloat_FromDouble(*current_inarg_pointer);
            current_inarg_pointer += 1;
        }
        else { // Vector or Matrix
            std::vector<npy_intp> dims_vec = calculate_dynamic_dimensions(input, scalar_values);
            int num_elements = input_sizes[i];
            
            if (num_elements == 0) {
                Log("      Creating empty array (zero dimensions detected)");
                // Create empty array with proper shape
                if (dims_vec.empty()) dims_vec.push_back(0);
                pValue = PyArray_ZEROS(static_cast<int>(dims_vec.size()), dims_vec.data(), NPY_FLOAT64, 0);
            }
            else {
                Log("      Creating array with " + std::to_string(num_elements) + " elements");
                pValue = PyArray_SimpleNewFromData(static_cast<int>(dims_vec.size()), dims_vec.data(), NPY_FLOAT64, current_inarg_pointer);
            }
            
            current_inarg_pointer += num_elements;
        }
        
        PyTuple_SetItem(pArgs, i, pValue); // Steals reference to pValue
    }
    
    Log("  Marshalling complete.");
    return pArgs;
}

// This function unpacks the tuple of results from Python and copies the data back.
static bool MarshalOutputsToCpp(PyObject* pResultTuple, const json& outputs_config, double* outargs, std::string& errorMessage) {
    if (!pResultTuple || !PyTuple_Check(pResultTuple)) {
        PyErr_Print();
        errorMessage = "Error: Python call failed or did not return a tuple.";
        Log(errorMessage);
        return false;
    }

    Log("Python call successful. Processing " + std::to_string(PyTuple_Size(pResultTuple)) + " result(s).");
    double* current_outarg_pointer = outargs;

    for (Py_ssize_t i = 0; i < PyTuple_Size(pResultTuple); ++i) {
        PyObject* pItem = PyTuple_GetItem(pResultTuple, i);
        const auto& output_config = outputs_config[i];
        std::string type = output_config["type"];
        Log("  Output #" + std::to_string(i) + ": Type='" + type + "'");

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
            Log("Error reading config: " + errorMessage);
            return false;
        }
        Log("Config read successfully.");
        
        // Validate the dimensions_from configuration
        errorMessage = validate_dimensions_from();
        if (!errorMessage.empty()) {
            Log("Config validation failed: " + errorMessage);
            return false;
        }
        Log("Config validation completed successfully.");
    }

    if (!Py_IsInitialized()) {
        Log("Python interpreter is not initialized. Initializing now...");

        PyConfig py_config;
        PyConfig_InitPythonConfig(&py_config);

        // --- REVERTED LOGIC: Get Python Home from the config file ---
        if (config.contains("python_path")) {
            std::string python_home = config["python_path"];
            Log("Using python_path from config: " + python_home);
            PyStatus status = PyConfig_SetBytesString(&py_config, &py_config.home, python_home.c_str());
            if (PyStatus_Exception(status)) {
                errorMessage = "Error: Failed to set Python Home from config path.";
                Log(errorMessage);
                PyConfig_Clear(&py_config);
                return false;
            }
        }
        else {
            errorMessage = "Error: 'python_path' key is missing from the config file.";
            Log(errorMessage);
            PyConfig_Clear(&py_config);
            return false;
        }

        PyStatus status = Py_InitializeFromConfig(&py_config);
        PyConfig_Clear(&py_config);
        if (PyStatus_Exception(status)) {
            errorMessage = "Error: Py_InitializeFromConfig failed.";
            Log(errorMessage);
            return false;
        }

        if (!initialize_numpy(errorMessage)) return false;
        if (!add_script_path_to_sys()) return false;
        if (!load_script_and_function(errorMessage)) return false;
    }
    else {
        Log("Python interpreter is already initialized.");
    }

    Log("--- Python Manager initialization successful ---");
    return true;
}

void FinalizePython() {
    // LOGGING: Announce the start of the cleanup process.
    Log("--- Finalizing Python Manager ---");

    Py_XDECREF(pFunc);
    Py_XDECREF(pModule);

    if (Py_IsInitialized()) {
        // LOGGING: Confirm that we are shutting down the interpreter.
        Log("Shutting down Python interpreter.");
        Py_Finalize();
    }
    else {
        // LOGGING: Note if no shutdown was necessary.
        Log("Python interpreter was not initialized. No cleanup needed.");
    }
}

int GetNumberOfInputs() {
    if (config.empty()) return 0;

    int total_inputs = 0;
    for (const auto& input : config["inputs"]) {
        // If any input is a dynamic type, we must return -1.
        if (input["type"] == "timeseries") {
            Log("GetNumberOfInputs detected a dynamic time series. Returning -1.");
            return -1;
        }
        
        // Check for dimensions_from (dynamic sizing)
        if (input.contains("dimensions_from")) {
            Log("GetNumberOfInputs detected dynamic dimensions_from. Returning -1.");
            return -1;
        }
        
        total_inputs += calculate_total_elements(input["dimensions"]);
    }

    Log("GetNumberOfInputs calculated a total of: " + std::to_string(total_inputs));
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

    Log("GetNumberOfOutputs calculated a total of: " + std::to_string(total_outputs));
    return total_outputs;
}

// --- The ExecuteCalculation function is now a clean, high-level commander ---
void ExecuteCalculation(double* inargs, double* outargs, std::string& errorMessage) {
    Log("--- Executing Calculation Cycle ---");
    if (!pFunc) {
        errorMessage = "Error: Python function not loaded.";
        Log(errorMessage);
        return;
    }

    // 1. Delegate argument preparation
    PyObject* pArgs = MarshalInputsToPython(config["inputs"], inargs);
    if (!pArgs) {
        errorMessage = "Error: Failed to marshal inputs for Python.";
        Log(errorMessage);
        return;
    }

    // 2. Call the Python function
    Log("Calling Python function...");
    PyObject* pResultTuple = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    // 3. Delegate result processing
    if (!MarshalOutputsToCpp(pResultTuple, config["outputs"], outargs, errorMessage)) {
        // MarshalOutputsToCpp handles its own error logging and Py_DECREF
        return;
    }

    Log("--- Calculation Cycle Complete ---");
}
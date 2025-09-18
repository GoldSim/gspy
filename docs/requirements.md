# v1.6: Requirements Specification

## 1. Vision and Guiding Principles

The primary goal of GSPy v2.0 is to provide a **simple, robust, and maintainable bridge** between GoldSim and Python. It will enable users to perform specialized calculations in Python using GoldSim's External element.

The core principles for this redesign are:

* **Python-Centric User Experience:** The end-user will only write Python code and a simple configuration file. No knowledge of C/C++ or DLL compilation will be required.
* **Simplicity over Complexity:** We will implement only the essential features required to pass data back and forth.
* **Stateless Operation:** The DLL's primary job is to be a stateless data marshaler. It translates data from GoldSim's format to a Python-friendly format, runs the user's script, and translates the results back.
* **Clear and Actionable Error Handling:** When the user's Python script fails, the error message must be passed clearly back to the GoldSim user to facilitate debugging.

---

## 2. Core Architecture

The system will consist of three main components:

1.  **GSPy.dll (The C++ Shim):** A pre-compiled C++ DLL that implements the required interface for a GoldSim External element. Users will rename this base DLL for each specific task (e.g., `inflows.dll`, `calculations.dll`), and the DLL will automatically look for a matching configuration file with the same name (e.g., `inflows.json`, `calculations.json`). Its sole responsibilities are to read the configuration file, manage the Python interpreter, marshal data between GoldSim and Python, and handle errors. The DLL is linked into GoldSim at run time.
2.  **Configuration File (e.g., `inflows.json`):** A human-readable text file that defines the "contract" between GoldSim and the Python script. It must have the same name as the renamed DLL and will specify the path to the Python script and the structure of all inputs and outputs.
3.  **User Python Script (e.g., `main.py`):** A standard Python script containing a specific function that GSPy.dll will call. This script will receive inputs from GoldSim as standard Python data structures and will return outputs in the same manner.

---

## 3. Data Marshalling Requirements

All data is passed between GoldSim and the DLL via arrays of double-precision floating-point numbers. The C++ shim is responsible for structuring and destructuring this data for Python.

### 3.1 GoldSim to Python (Inputs)

* **Mechanism:** GSPy.dll will read the `inargs` array provided by GoldSim. Based on the `inputs` section of the `config.json` file, it will parse this flat array into distinct data objects.
* **Data Types:**
    * **Scalar:** A single scalar input is mapped to a single array argument. This will be passed as a standard Python float.
    * **Vector/Matrix:** Vector and matrix items will be specified item-by-item in the `inargs` array. These will be reshaped into a **NumPy array** with dimensions specified in the config file.
    * **Time Series:** The DLL will parse the specific Time Series Definition format from the `inargs` array. It will then pass two NumPy arrays to the Python script: one for the time points and one for the corresponding values (which could be 1D, 2D, or 3D).

### 3.2 Python to GoldSim (Outputs)

* **Mechanism:** The user's Python function will return a dictionary where keys match the output names defined in `config.json`.
* **Data Types:**
    * **Scalar/Vector/Matrix:** The C++ shim will take the returned number or NumPy array and flatten it into the `outargs` array in the correct order.
    * **Lookup Table:** The user will return NumPy arrays representing the table's data. The C++ shim will format these into the precise sequence required by GoldSim for 1-D, 2-D, or 3-D tables.
    * **Time Series:** The user will return NumPy arrays for time and values, along with necessary metadata. The shim will format this into the exact Time Series Definition sequence GoldSim expects.

---

## 4. Configuration File (.json)

This file is the key to decoupling the C++ shim from the user's logic. It must be present in the same directory as the GoldSim model file and have the same name as the renamed DLL (e.g., my_test.dll and my_test.json).

Required Fields:
* python_path: The absolute path to the Python installation directory (e.g., C:\\Python313).
* script_name: The name of the user's Python script, without the .py extension.
* function_name: The name of the function within the script to execute.
* inputs: A list of objects defining the data sent from GoldSim to Python.
* outputs: A list of objects defining the data returned from Python to GoldSim.

Data Object Keys:
| Key | Description | Example |
| :--- | :--- | :--- |
| `name` | A descriptive name for reference. | `"flow_rate"` |
| `type` | Data type. Can be `"scalar"`, `"vector"`, `"matrix"`, `"timeseries"`, or `"table"`. | `"scalar"` |
| `dimensions` | The shape of the data. Use `[]` for scalars. | `[10]`, `[5, 3]` |
| `max_points` | **Required for `timeseries` outputs.** Specifies max buffer size. | `1000` |
| `max_elements` | **Required for `table` outputs.** Specifies max buffer size. | `500` |

---

### Simple Example (Scalar In, Scalar Out)
This is the most basic configuration.

```json
{
  "python_path": "C:\\Users\\YourName\\AppData\\Local\\Programs\\Python\\Python313",
  "script_name": "scalar_test",
  "function_name": "process_data",
  "inputs": [
    {
      "name": "input_scalar",
      "type": "scalar",
      "dimensions": []
    }
  ],
  "outputs": [
    {
      "name": "output_scalar",
      "type": "scalar",
      "dimensions": []
    }
  ]
}
```
### Advanced Data Type Examples
Here are examples of how to define more complex data types in the inputs or outputs lists.

Vector (1D Array of 10 elements):

```json
{
  "name": "my_vector",
  "type": "vector",
  "dimensions": [ 10 ]
}
```
Matrix (2D Array of 5 rows and 3 columns):

```json
{
  "name": "my_matrix",
  "type": "matrix",
  "dimensions": [ 5, 3 ]
}
```
Time Series Input (Scalar Time Series):

```json
{
  "name": "my_input_ts",
  "type": "timeseries",
  "dimensions": []
}
```
Time Series Output (Vector Time Series with 5 rows):

```json
{
  "name": "my_output_ts",
  "type": "timeseries",
  "dimensions": [ 5 ],
  "max_points": 1000
}
```
Lookup Table Output:

```json
{
  "name": "my_lookup_table",
  "type": "table",
  "max_elements": 500
}
```

---

## 5. Error Handling and Logging

* If the user's Python script raises an exception, the C++ shim must catch it.
* The shim will then return a status code of **-1** to GoldSim to indicate a fatal error where an error message is being returned.
* The Python exception message and traceback will be captured and formatted into a static, NULL-terminated character buffer.
* A pointer to this error message buffer will be placed in `outargs[0]`, which GoldSim will then interpret and display to the user.

---

## 6. DLL Interface and Calling Sequence

The GSPy.dll must correctly implement the standard GoldSim calling function and respond to requests from GoldSim.

### 6.1 Function Signature

The DLL will export a function with the C-style signature GoldSim expects.

### 6.2 Calling Sequence

GoldSim calls the DLL at different times with specific requests (`XFMethod`). The DLL's response must be correct for each phase:

* **Before the Simulation Starts:** To check model validity, GoldSim will:
    1.  Request the version number (`XFMethod = 2`).
    2.  Request the number of input/output arguments (`XFMethod = 3`).
    3.  Request cleanup (`XFMethod = 99`) and unload the DLL.

* **During a Realization:** When the External element needs to be updated, and if the DLL is not already loaded, GoldSim will:
    1.  Request the version number (`XFMethod = 2`).
    2.  Request the number of input/output arguments (`XFMethod = 3`).
    3.  Request initialization (`XFMethod = 0`).
    4.  Request a normal calculation (`XFMethod = 1`).

* **Between/After Realizations:**
    * GoldSim will request initialization (`XFMethod = 0`) before each new realization if the DLL is already loaded.
    * After the entire simulation completes, GoldSim will request cleanup (`XFMethod = 99`) if the DLL is still loaded.

---

## 7. Technical Specifications and Constraints

* **Target Platform:** GSPy.dll will be compiled as a **64-bit DLL** for the Windows operating system to match GoldSim's architecture.
* **Dependencies:** The solution requires the user to have a working Python installation on their machine and to provide the path to it in the configuration file. All additional files required by the DLL, such as runtime libraries, must be present.
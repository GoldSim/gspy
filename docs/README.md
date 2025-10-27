# GSPy: The GoldSim-Python Bridge

**Current Version: 1.8.0** | [Changelog](CHANGELOG.md)

GSPy is a C++ bridge that allows GoldSim models to call external Python scripts. It acts as a shim DLL for GoldSim's `External` element, enabling users to leverage the capabilities of the Python ecosystem directly within their dynamic simulations.

This project was designed to be simple, robust, and flexible, allowing modelers to extend GoldSim's capabilities without needing to write or compile any C++ code themselves.

-----

## Features

  * **Seamless Integration:** Call Python scripts directly from a GoldSim `External` element.
  * **Data Marshalling:** Pass a variety of mixed data types to and from Python, including:
      * Scalars
      * Vectors (1D Arrays)
      * Matrices (2D Arrays)
      * Time Series
      * Lookup Tables (only from Python to GoldSim)
  * **Data-Driven Configuration:** A JSON file defines the interface, allowing for configurations without changing code.
  * **Error Handling:** Python exceptions are caught gracefully and reported directly to the GoldSim user, simplifying debugging.
  * **Enhanced Diagnostic Logging:** High-performance, configurable logging system with automatic log file headers, thread-safe operations, and seamless Python integration.

-----

## How It Works

GSPy uses a modular, three-part architecture. The DLL acts as an interpreter and data marshaller between the two environments. A naming convention ensures that the correct DLL is always linked to the correct configuration file.

`[GoldSim Model (*.gsm)] <--> [Renamed GSPy DLL (e.g., my_calc.dll)] <--> [Config File (my_calc.json)] & [Python Script (*.py)]`

GSPy links three files together: the DLL, the JSON configuration, and the Python script.

**The DLL and JSON file names MUST Match:** The GSPy DLL and its JSON configuration file must have the exact same name and be in the same directory. The DLL looks for a .json file with its own name.

Correct: GSPy_Model.dll and GSPy_Model.json

Incorrect: GSPy_Model.dll and config.json

The JSON points to the Python Script: The "script_path" field inside the JSON file tells the DLL which Python script to execute. The Python script can be named anything and be located anywhere, as long as the path is correct.

-----

## Prerequisites

To use the pre-compiled GSPy DLLs (v1.8.0+), you will need:

1.  **GoldSim 15+** (The provided DLLs are 64-bit).
2.  A **64-bit** installation of **Python 3.11** OR **Python 3.14**.
    * GSPy v1.8.0+ specifically requires one of these two versions due to changes in the Python C-API and the way the DLLs are compiled. Ensure you have the correct 64-bit installer from [python.org](https://www.python.org/).
    * You must install the Python version matching the GSPy DLL you intend to use (e.g., `GSPy_Release_py311.dll` requires Python 3.11).
3.  3.  The **NumPy** and **SciPy** Python packages installed *for the specific Python version you are using*.
    * **NumPy** is required for basic GSPy operation (handling arrays).
    * **SciPy** is required for advanced numerical methods like implicit equation solving (used in some examples like the Hazen-Williams solver) and other scientific computing tasks. It is recommended for broader GSPy usage.
    * After installing Python, open a command prompt and run the appropriate command:
        * For Python 3.11: `py -3.11 -m pip install numpy scipy`
        * For Python 3.14: `py -3.14 -m pip install numpy scipy`
4.  **Python Directory Added to System PATH:**
    * The installation directory of your chosen Python version (e.g., `C:\...\Python311`) **must** be added to your Windows system `PATH` environment variable.
    * **Reason:** This is required so that Windows can find the necessary Python runtime DLL (e.g., `python311.dll`) when GoldSim initially tries to load the `GSPy_*.dll`. See "Method 2" below for instructions.
5.  **JSON `python_path` Configured:**
    * You must configure the `python_path` setting inside the `GSPy_*.json` configuration file to point to the *exact* installation directory of the required Python version.
    * **Reason:** This path is used by the GSPy C++ code at runtime to initialize the correct embedded Python interpreter. See "Method 1" below for instructions.

-----

## Python Setup & Troubleshooting

Correctly configuring Python paths is essential for GSPy to function. There are two critical path settings: the **System PATH** (for Windows to load the DLL) and the **JSON `python_path`** (for GSPy to initialize Python).

### **Requirement 1: Add Python to System PATH**

This allows Windows to find the core Python DLL (e.g., `python311.dll`) needed by `GSPy_*.dll` when GoldSim starts. **This is mandatory.**

1.  **Find Python Path:** Locate your Python 3.11 or 3.14 installation directory (e.g., `C:\Users\YourUsername\AppData\Local\Programs\Python\Python311`).
2.  **Edit System Environment Variables:** Search for "Environment Variables" in the Windows Start Menu and click "Edit the system environment variables".
3.  Click **"Environment Variables..."**.
4.  Under **"System variables"**, find and select **"Path"**, then click **"Edit..."**.
5.  Click **"New"** and add your Python installation directory (e.g., `C:\Users\YourUsername\AppData\Local\Programs\Python\Python311`).
6.  *(Optional but Recommended)* Click **"New"** again and add the corresponding `Scripts` directory (e.g., `C:\Users\YourUsername\AppData\Local\Programs\Python\Python311\Scripts`).
7.  Click **"OK"** on all dialogs.
8.  **Restart GoldSim or your computer** for the changes to take effect.

### **Requirement 2: Configure `python_path` in JSON**

This tells the GSPy C++ code which specific Python interpreter to initialize and use for running your script. **This is mandatory.**

1.  **Open** the `GSPy_*.json` file corresponding to the DLL you are using (e.g., `GSPy_PY311.json`).
2.  **Edit** the `python_path` value. It must be the full, absolute path to the root directory of the matching Python installation. Use double backslashes (`\\`).

    ```json
    {
      "python_path": "C:\\Users\\YourUsername\\AppData\\Local\\Programs\\Python\\Python311",
      "script_path": "your_script.py",
      "function_name": "process_data",
      "log_level": 3,
      "inputs": [...],
      "outputs": [...]
    }
    ```
3.  **Save** the JSON file.

### Verifying Your Python Setup

Before using GSPy, verify your installations:

1.  **Open a NEW Command Prompt** (after setting PATH and restarting if necessary).
2.  **Verify PATH:** Type `where python`. You should see the path to your desired Python version (3.11 or 3.14) listed.
3.  **Test Correct Python Version:**
    * If using 3.11: `py -3.11 --version` (Should show 3.11.x)
    * If using 3.14: `py -3.14 --version` (Should show 3.14.x)
4.  **Test NumPy/SciPy Installation:**
    * If using 3.11: `py -3.11 -c "import numpy, scipy; print('NumPy & SciPy for 3.11 OK')"`
    * If using 3.14: `py -3.14 -c "import numpy, scipy; print('NumPy & SciPy for 3.14 OK')"`

If any command fails, review the installation steps and path settings.

### Installation Tips for Windows Users

* **Use the official Python installer** from [python.org](https://python.org).
* **Recommended:** Check **"Add python.exe to PATH"** during installation (though verify it adds the correct version if you have multiple installs).
* **Use Python 3.11 or 3.14 (64-bit)** as required by GSPy v1.8.0+.
* **Always use 64-bit Python** (required for GSPy DLLs).

### Troubleshooting Common Errors

1.  **GoldSim Error: "Cannot load DLL..." (No Log File Created)**
    * **Cause:** Windows cannot find the required `pythonXXX.dll` (e.g., `python311.dll`) dependency for your `GSPy_*.dll`.
    * **Solution:** Ensure the correct Python installation directory (3.11 or 3.14) is added to your system `PATH` environment variable and that you have restarted GoldSim/your computer. Alternatively, manually copy the required `pythonXXX.dll` from the Python installation folder into the same directory as your `GSPy_*.dll`.
2.  **GSPy Log Error: `Could not initialize NumPy C-API`**
    * **Cause:** Usually a mismatch between the Python version GSPy was compiled for and the Python version found/used at runtime, or NumPy not installed correctly for that version.
    * **Solution:** Verify you are using the correct GSPy DLL (`_py311` or `_py314`) for your installed Python version. Double-check the `python_path` in your JSON file is correct. Ensure NumPy is installed for that specific Python version (`py -X.Y -m pip install numpy`).
3.  **GSPy Log Error: `Failed to load Python script...` or `Cannot find function...`**
    * **Cause:** GSPy initialized Python correctly but couldn't find/import your `.py` file or the specified function within it.
    * **Solution:** Check the `script_path` and `function_name` in your JSON file. Ensure the `.py` file exists at that location and doesn't have syntax errors. Make sure the function name matches exactly.
4.  **Check JSON `python_path`:** Verify it matches your actual installation directory precisely (use `where python` or `py -X.Y -c "import sys; print(sys.executable)"` to confirm).
5.  **Verify Python is 64-bit:** Use `py -X.Y -c "import platform; print(platform.architecture())"`. Should show '64bit'.
6.  **Check file permissions:** Ensure GoldSim/GSPy has permission to read the Python directory and your script file.
7.  **Restart GoldSim:** Especially after changing environment variables.

-----

## Quick Start Guide

This guide will run a simple "scalar in, scalar out" test.

### 1\. Create a Project Folder

Create a new, empty folder on your computer to hold all the files for your test.

### 2\. Prepare the GSPy Files

Copy the base `GSPy.dll` into your new folder and rename it to **`scalar_test.dll`**.

### 3\. Create the Configuration File

In the same folder, create a new text file named **`scalar_test.json`** and paste the following content. **Remember to update the `python_path`**. Note the *.json configuration file must have the same name as the DLL.

```json
{
  "python_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Python\\Python311",
  "script_name": "scalar_test.py",
  "function_name": "process_data",
  "log_level": 0,
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

### 4\. Create the Python Script

In the same folder, create a script named **`scalar_test.py`** (the name doesn't need to be the same as the dll or json file) and paste the following content:

```python
import traceback
import gspy

def process_data(*args):
  """
  Receives one scalar, multiplies it by 10, and returns one scalar.
  """
  try:
    gspy.log("Starting scalar calculation", 2)  # INFO level
    input_scalar = args[0]
    gspy.log(f"Input value: {input_scalar}", 3)  # DEBUG level
    
    result = input_scalar * 10.0
    gspy.log(f"Calculation complete, result: {result}", 2)  # INFO level
    
    # The return value MUST be a tuple
    return (result,)
  except Exception:
    gspy.log("Error in calculation: " + traceback.format_exc(), 0)  # ERROR level
    print(traceback.format_exc())
    return (0.0,)
```

### 5\. Configure GoldSim

1.  Create a new GoldSim model and save it in your project folder.
2.  Create an `External` element.
3.  In its properties, set the **DLL Path** to **`scalar_test.dll`** and the Function Name to GSPy.
4.  Go to the **Interface** tab and define **one scalar input** and **one scalar output**.
5.  Run the model. The result should be 10 times the input.

-----

## Usage Reference

### Configuration (`.json`) File Details

  * **`python_path`**: Full path to your Python installation directory.
  * **`script_name`**: The name of your Python script.
  * **`function_name`**: The function in your script that GSPy will call (default: "process_data")
  * **`inputs` / `outputs`**: Lists of data objects. **The order must match the order in the GoldSim Interface tab.**
      * **`name`**: A descriptive name for your reference.
      * **`type`**: Can be `"scalar"`, `"vector"`, `"matrix"`, `"timeseries"`, or `"table"` (table only available for outputs).
      * **`dimensions`**: The shape of the data. Use `[]` for scalars or scalar time series, `[10]` for a 10-element vector, `[5, 3]` for a 5x3 matrix.
      * **`dimensions_from`** (Optional): For vector and matrix inputs only, this allows you to define dimensions dynamically at runtime. Its value is an array of strings that must reference the name of preceding scalar inputs. The referenced scalar(s) must be defined as inputs **before** the dynamic array in the inputs list.
        * For a vector: "dimensions_from": ["vector_size_scalar"]
        * For a matrix: "dimensions_from": ["row_count_scalar", "col_count_scalar"]
      * **`max_points` / `max_elements`**: Required for dynamic **outputs** `"timeseries"` or `"table"` to pre-allocate memory (only required for outputs from python to GoldSim)
  * **`log_level`** (Optional): Controls logging verbosity with atomic-level performance optimization. Default is 2 (INFO).
      * **`0`** = ERROR only (fastest, ~90-95% performance improvement for production)
      * **`1`** = ERROR + WARNING (optimized for critical issues)
      * **`2`** = ERROR + WARNING + INFO (default, balanced performance)
      * **`3`** = ERROR + WARNING + INFO + DEBUG (full verbosity, development only)

### Performance Optimization

GSPy features a high-performance logging system with atomic-level filtering and thread-safe operations. For production simulations, add `"log_level": 0` to your JSON configuration:

```json
{
  "python_path": "C:\\Python311",
  "script_path": "my_script.py", 
  "function_name": "process_data",
  "log_level": 0,
  "inputs": [...],
  "outputs": [...]
}
```

**Performance Benefits:**
- **Fast-path filtering**: Disabled log levels have minimal overhead (< 10ns per call)
- **Atomic operations**: Thread-safe level checking without locks
- **Hybrid flush policy**: Immediate flush for errors/warnings, write-only for info/debug
- **Automatic fallback**: Seamless stderr redirect if file operations fail

This eliminates ~90-95% of logging overhead while preserving critical error reporting and maintaining thread safety.

### Python Script API

  * Your function (e.g., `process_data`) must accept arguments using **`*args`**.
  * Inputs are passed in a tuple (`args`) in the order defined in the JSON.
  * Your function **must** return a **tuple** of results, even if there is only one (e.g., `return (my_result,)`). The order must match the JSON `outputs`.

#### Python Logging

Python scripts can write custom messages to the GSPy log file using the enhanced `gspy` module with thread-safe, high-performance logging:

```python
import gspy

def process_data(*args):
    # Write messages at different log levels (with atomic filtering)
    gspy.log("Starting calculation", 2)  # INFO level
    gspy.log("Debug info: processing input", 3)  # DEBUG level  
    gspy.log("Warning: unusual input value", 1)  # WARNING level
    gspy.log("Critical error occurred", 0)  # ERROR level
    
    # Default level is INFO (2) if not specified
    gspy.log("Simple message")
    
    # Your calculation logic here...
    return (result,)
```

**Enhanced Python Logging Features:**
- **Thread-safe operations**: Safe for concurrent access with DLL logging
- **Atomic filtering**: Disabled levels have minimal performance impact
- **Reentrancy protection**: Prevents infinite recursion in logging calls
- **Unified output**: Python and DLL messages appear in the same log file with consistent formatting

**Log Levels:** 0=ERROR, 1=WARNING, 2=INFO (default), 3=DEBUG

#### Log File Format

Each GSPy run automatically creates a log file with a professional header:

```
========================================
GSPy: The GoldSim-Python Bridge
Version: 1.8.0
Build Date: Oct 25 2025 14:30:15
========================================

2025-10-25 14:30:15 - INFO: Logger initialized successfully
2025-10-25 14:30:15 - INFO: Starting scalar calculation
2025-10-25 14:30:15 - INFO: Calculation complete, result: 50.0
```

**Features:**
- **Automatic header**: Version, build date, and project title
- **Timestamped entries**: Precise timing for debugging
- **Thread-safe writes**: Safe for concurrent Python and DLL logging
- **Fallback handling**: Automatic stderr redirect if file operations fail

#### Data Type Mapping

| From GoldSim | Received in Python as...                                                             |
| :----------- | :----------------------------------------------------------------------------------- |
| Scalar       | `float`                                                                              |
| Vector       | 1D NumPy Array                                                                       |
| Matrix       | 2D NumPy Array                                                                       |
| Time Series  | Python Dictionary with keys: `"timestamps"`, `"data"`, `"time_basis"`, `"data_type"` |

| To GoldSim   | Returned from Python as...                                                           |
| :----------- | :----------------------------------------------------------------------------------- |
| Scalar       | `float` or `int`                                                                     |
| Vector       | 1D NumPy Array                                                                       |
| Matrix       | 2D NumPy Array                                                                       |
| Time Series  | Python Dictionary with keys: `"timestamps"`, `"data"`, `"time_basis"`, `"data_type"` |
| Lookup Table | Python Dictionary with keys: `"table_dim"`, `"row_labels"`, `"col_labels"`, `"data"` |

-----
## Extended Examples

### Example 1: Time Series Example
This example demonstrates how to handle time series data. It passes both a scalar time series and a 2-element vector time series from GoldSim to a Python script. The script performs a simple calculation and returns two new time series back to GoldSim.

**GSPy_TS_Test.json**
This configuration defines the interface. Note the use of "type": "timeseries" and that max_points is required for outputs to pre-allocate memory.

```json
{
  "python_path": "C:\\Python311",
  "script_path": "test_timeseries.py",
  "function_name": "process_data",
  "inputs": [
    {
      "name": "input_timeseries1",
      "type": "timeseries",
      "dimensions": []
    },
    {
      "name": "input_timeseries2",
      "type": "timeseries",
      "dimensions": [2]
    }
  ],
  "outputs": [
    {
      "name": "timeseries_1",
      "type": "timeseries",
      "dimensions": [2],
      "max_points": 1000
    },
    {
      "name": "timeseries_2",
      "type": "timeseries",
      "dimensions": [],
      "max_points": 1000
    }
  ]
}
```
**test_timeseries.py**
The Python script receives and returns time series as dictionaries. The data key holds a NumPy array where each row represents a dimension and each column represents a time step.

```python
import numpy as np
import traceback

def process_data(*args):
  """
  Receives a scalar time series and a vector time series.
  Returns a vector time series and a new scalar time series.
  """
  try:
    # 1. Unpack the two input Time Series dictionaries
    input_ts_scalar_dict = args[0]
    input_ts_vector_dict = args[1]
    
    # 2. Extract the data and timestamps from each
    scalar_timestamps = input_ts_scalar_dict["timestamps"]
    scalar_data = input_ts_scalar_dict["data"]
    
    vector_timestamps = input_ts_vector_dict["timestamps"]
    vector_data = input_ts_vector_dict["data"] # This will have shape (2, num_points)
    
    # 3. Prepare the outputs
    
    # Output 1: Pass the input vector time series straight through
    output_ts_vector = {
        "timestamps": vector_timestamps,
        "data": vector_data,
        "time_basis": input_ts_vector_dict["time_basis"],
        "data_type": input_ts_vector_dict["data_type"]
    }
    
    # Output 2: Create a new scalar time series by adding the inputs
    # Note: Assumes both time series have the same timestamps
    new_scalar_data = vector_data[0, :] + scalar_data
    
    output_ts_scalar = {
        "timestamps": vector_timestamps,
        "data": new_scalar_data,
        "time_basis": input_ts_scalar_dict["time_basis"],
        "data_type": input_ts_scalar_dict["data_type"]
    }

    # 4. Return the results as a tuple in the correct order
    return (output_ts_vector, output_ts_scalar)

  except Exception:
    # In case of error, return properly formatted empty time series
    empty_ts = {"timestamps": np.array([0.0]), "data": np.array([0.0]), "time_basis": 0.0, "data_type": 0.0}
    return (empty_ts, empty_ts)
```

Notes:

**Time Series Dictionary Format**: Time series are passed as Python dictionaries with **four required keys**:
- `"timestamps"`: 1D NumPy array of time values
- `"data"`: NumPy array containing the time series values
- `"time_basis"`: Float value (typically 0.0, pass through from input)
- `"data_type"`: Float value (typically 0.0, pass through from input)

Data Shape: The "data" value is a NumPy array. A scalar time series will have a 1D array, while a vector time series will have a 2D array of shape (number_of_elements, number_of_timesteps).

Memory Allocation: You must specify "max_points" for any output time series in the JSON file. This allows GoldSim to allocate the necessary memory buffer to receive the data from Python.

**Important**: Always preserve the `time_basis` and `data_type` values from input time series when creating outputs. These contain metadata that GoldSim requires.

-----

### Example 2: Lookup Table

This example demonstrates how to dynamically generate a complete GoldSim Lookup Table from within Python. It takes a single scalar value from GoldSim and uses it to construct the row labels, column labels, and the main data body of a 2D lookup table, which is then returned to GoldSim.

**LookupTable.json**
The JSON configuration specifies the output type as "table". For table outputs, you must provide "max_elements" so GoldSim can pre-allocate enough memory. This should be a safe upper bound for the total number of cells in the table (rows * cols + rows + cols).

```json
{
  "python_path": "C:\\Python311",
  "script_path": "lookup_table_script.py",
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
      "name": "output_table",
      "type": "table",
      "max_elements": 500
    }
  ]
}
```

**lookup_table_script.py**
To return a lookup table, the Python script must construct and return a dictionary with a specific structure and keys. The C++ bridge code looks for these exact keys to correctly build the table in GoldSim.

```python
import numpy as np
import traceback

def process_data(*args):
  """
  Receives a scalar and uses it to generate and return a 2D Lookup Table.
  """
  try:
    # 1. Unpack the input scalar
    input_scalar = args[0]
    
    # 2. Generate the table components based on the input
    row_labels = np.arange(4) * input_scalar  # e.g., [0, 10, 20, 30]
    col_labels = np.arange(3) + 1            # e.g., [1, 2, 3]
    
    # Create the 2D data array for the table body
    data = np.outer(row_labels, col_labels)

    # 3. Assemble the required dictionary for the lookup table
    table_dictionary = {
      "table_dim": 2,
      "row_labels": row_labels,
      "col_labels": col_labels,
      "data": data
    }
    
    # 4. Return the result as a tuple
    return (table_dictionary,)

  except Exception:
    # In case of error, return an empty dictionary
    return ({},)
```

Notes:

Required Dictionary Structure: A lookup table must be returned as a Python dictionary with the following keys:

 - "table_dim": An integer, 1 for a 1D table or 2 for a 2D table.
 - "row_labels": A 1D NumPy array of monotonically increasing values for the row headers.
 - "col_labels": A 1D NumPy array of monotonically increasing values for the column headers. (This key should be omitted for a 1D table).
 - "data": A 1D or 2D NumPy array containing the table's body data. Its shape must match the labels.

Dynamic Generation: This method is extremely powerful for scenarios where table data is not static, such as generating temperature-dependent decay rates or pressure-dependent material properties on the fly.

-----

### Example 3: Mixed Data

This is a "stress test" example that showcases the full power and flexibility of GSPy. It demonstrates passing a large number of mixed data types (scalars, vectors, matrices, and time series) to Python in a single function call. The script then performs various calculations and returns a completely different set of outputs, including a dynamically generated Lookup Table.

This example proves that the inputs and outputs do not need to be symmetrical. You can have any number of inputs and any number of outputs of any supported type.

**Mixed_Types.json**
The JSON file defines the complex mapping. Notice that the inputs array has nine items, while the outputs array has seven items of different types and in a different order.

```json
{
  "python_path": "C:\\Python311",
  "script_path": "mixed_types.py",
  "function_name": "process_data",
  "inputs": [
    { "name": "input_vector", "type": "vector", "dimensions": [ 5 ] },
    { "name": "scalar_1", "type": "scalar", "dimensions": [] },
    { "name": "scalar_2", "type": "scalar", "dimensions": [] },
    { "name": "input_matrix", "type": "matrix", "dimensions": [ 3, 2 ] },
    { "name": "scalar_3", "type": "scalar", "dimensions": [] },
    { "name": "timeseries_1", "type": "timeseries", "dimensions": [] },
    { "name": "timeseries_2", "type": "timeseries", "dimensions": [] },
    { "name": "scalar_4", "type": "scalar", "dimensions": [] }
  ],
  "outputs": [
    { "name": "output_matrix", "type": "matrix", "dimensions": [ 4, 2 ] },
    { "name": "output_timeseries", "type": "timeseries", "dimensions": [], "max_points": 1000 },
    { "name": "output_scalar_1", "type": "scalar", "dimensions": [] },
    { "name": "output_table", "type": "table", "max_elements": 50 },
    { "name": "output_vector_1", "type": "vector", "dimensions": [ 6 ] },
    { "name": "output_vector_2", "type": "vector", "dimensions": [ 3 ] },
    { "name": "output_scalar_2", "type": "scalar", "dimensions": [] }
  ]
}
```
**mixed_types.py**
The Python script correctly unpacks the nine arguments from the *args tuple. It then returns a tuple with seven results, ensuring the order matches the outputs array in the JSON file.

```python
import numpy as np
import traceback

def process_data(*args):
  """
  Receives 9 inputs and returns 7 outputs of mixed types.
  """
  try:
    # 1. Unpack all 9 input arguments in order
    input_vector = args[0]
    scalar_1 = args[1]
    scalar_2 = args[2]
    input_matrix = args[3]
    scalar_3 = args[4]
    ts_dict_1 = args[5]
    ts_dict_2 = args[6]
    scalar_4 = args[7] # This input is unused, which is perfectly fine

    # 2. Perform various calculations to generate results
    v1 = np.arange(4) + scalar_1
    v2 = np.array([np.mean(input_vector), np.sum(input_matrix)])
    output_matrix = np.outer(v1, v2)
    output_scalar_1 = np.std(ts_dict_1['data']) + scalar_1
    output_vector_1 = input_matrix.flatten() * scalar_2
    data_2 = ts_dict_2['data']
    output_vector_2 = np.array([np.min(data_2), np.max(data_2), np.mean(data_2)])
    output_scalar_2 = np.dot(input_vector[:3], output_vector_2) + scalar_3

    # 3. Generate a new Time Series and Lookup Table
    output_timeseries = {
        "timestamps": ts_dict_1["timestamps"],
        "data": ts_dict_1["data"] + ts_dict_2["data"],
        "time_basis": ts_dict_1["time_basis"],
        "data_type": ts_dict_1["data_type"]
    }
    output_table = {
      "table_dim": 2,
      "row_labels": np.array([1, 2, 3]) + scalar_1,
      "col_labels": np.array([10, 20]) + scalar_2,
      "data": np.array([[1, 2], [3, 4], [5, 6]]) * scalar_3
    }

    # 4. Return all 7 results as a tuple in the specified output order
    return (
        output_matrix, 
        output_timeseries, 
        output_scalar_1, 
        output_table,
        output_vector_1, 
        output_vector_2, 
        output_scalar_2
    )

  except Exception:
    print(f"!!! PYTHON EXCEPTION !!!\n{traceback.format_exc()}")
    # In case of error, return a tuple of the correct size with dummy values
    empty_ts = {"timestamps": np.array([0.0]), "data": np.array([0.0]), "time_basis": 0.0, "data_type": 0.0}
    empty_table = {}
    return (np.zeros((4,2)), empty_ts, 0.0, empty_table, np.zeros(6), np.zeros(3), 0.0)
```

Notes:

Asymmetric Interface: The number, type, and order of inputs do not need to match the outputs. GSPy handles the complex mapping defined in the JSON.

Argument Order is Critical: The order of variables received in Python's *args tuple is determined by their order in the "inputs" array of the JSON file.

Return Order is Critical: The order of variables in the return tuple from Python must exactly match their order in the "outputs" array of the JSON file.

-----
### Example 4: Dynamic Array Sizing
The example demonstrates how to size a vector or a matrix at runtime using sclar values passed from GoldSim.

**Dynamic_Size_Test.json**
```json
{
  "python_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Python\\Python311",
  "script_path": "test_dynamic_sizing.py",
  "function_name": "process_data",
  "inputs": [
    {
      "name": "vector_size",
      "type": "scalar",
      "dimensions": []
    },
    {
      "name": "matrix_rows",
      "type": "scalar", 
      "dimensions": []
    },
    {
      "name": "matrix_cols",
      "type": "scalar",
      "dimensions": []
    },
    {
      "name": "static_scalar",
      "type": "scalar",
      "dimensions": []
    },
    {
      "name": "static_vector",
      "type": "vector",
      "dimensions": [3]
    },
    {
      "name": "dynamic_vector",
      "type": "vector",
      "dimensions_from": ["vector_size"]
    },
    {
      "name": "dynamic_matrix",
      "type": "matrix",
      "dimensions_from": ["matrix_rows", "matrix_cols"]
    }
  ],
  "outputs": [
    {
      "name": "result_scalar",
      "type": "scalar",
      "dimensions": []
    },
    {
      "name": "result_vector",
      "type": "vector",
      "dimensions": [5]
    }
  ]
}
```

**test_dynamic_sizing.py**
```python
import numpy as np
import traceback

def process_data(*args):
    """
    Test function for dynamic sizing functionality.
    Expected inputs:
    0. vector_size (scalar) - controls size of dynamic_vector
    1. matrix_rows (scalar) - controls rows of dynamic_matrix  
    2. matrix_cols (scalar) - controls cols of dynamic_matrix
    3. static_scalar (scalar) - regular scalar input
    4. static_vector (vector[3]) - regular static vector
    5. dynamic_vector (vector with size from vector_size)
    6. dynamic_matrix (matrix with size from matrix_rows x matrix_cols)
    """
    try:
        print(f"\n=== Dynamic Sizing Test ===")
        print(f"Received {len(args)} arguments:")
        
        # Extract and validate inputs
        vector_size = args[0]
        matrix_rows = args[1] 
        matrix_cols = args[2]
        static_scalar = args[3]
        static_vector = args[4]
        dynamic_vector = args[5]
        dynamic_matrix = args[6]
        
        print(f"  vector_size = {vector_size}")
        print(f"  matrix_rows = {matrix_rows}")
        print(f"  matrix_cols = {matrix_cols}")
        print(f"  static_scalar = {static_scalar}")
        print(f"  static_vector = {static_vector} (shape: {static_vector.shape})")
        print(f"  dynamic_vector = {dynamic_vector} (shape: {dynamic_vector.shape})")
        print(f"  dynamic_matrix = {dynamic_matrix} (shape: {dynamic_matrix.shape})")
        
        # Validate dynamic dimensions
        expected_vector_size = int(vector_size)
        expected_matrix_shape = (int(matrix_rows), int(matrix_cols))
        
        print(f"\nValidation:")
        print(f"  Expected dynamic_vector size: {expected_vector_size}, actual: {dynamic_vector.shape}")
        print(f"  Expected dynamic_matrix shape: {expected_matrix_shape}, actual: {dynamic_matrix.shape}")
        
        if dynamic_vector.shape == (expected_vector_size,):
            print("  Dynamic vector size is correct!")
        else:
            print("  Dynamic vector size mismatch!")
            
        if dynamic_matrix.shape == expected_matrix_shape:
            print("  Dynamic matrix shape is correct!")
        else:
            print("  Dynamic matrix shape mismatch!")
        
        # Create outputs
        result_scalar = np.sum(static_vector) + static_scalar + np.sum(dynamic_vector) + np.sum(dynamic_matrix)
        result_vector = np.array([1.0, 2.0, 3.0, 4.0, 5.0])
        
        print(f"\nOutputs:")
        print(f"  result_scalar = {result_scalar}")
        print(f"  result_vector = {result_vector}")
        
        return (result_scalar, result_vector)
    
    except Exception as e:
        print(f"!!! PYTHON EXCEPTION !!!")
        print(traceback.format_exc())
        return (0.0, np.zeros(5))
```
-----

## Developer Documentation

### Multi-Python Build System

GSPy now supports building against multiple Python versions (3.11 and 3.14) using Visual Studio Property Sheets. This flexible system allows developers to quickly switch between Python targets without manually editing project files.

**Rationale for Supported Versions:**

* **Python 3.11:** Chosen as the primary stable target. It represents the final, mature version before significant C-API changes introduced in Python 3.12. Targeting 3.11 provides a robust option for users who prioritize stability or whose environments rely on pre-3.12 compatibility. GSPy v1.8.0 includes C++ patches ensuring NumPy initialization works correctly on 3.11. Security updates for 3.11 continue until October 2027. Note: Users must have Python 3.11 installed to use this DLL; it is not binary compatible with Python 3.10 or earlier.

#### Prerequisites for Development

* Visual Studio 2022 with the "Desktop development with C++" workload
* Python 3.11 and/or Python 3.14 (64-bit installations)
* NumPy package installed for each Python version

#### Setting Up Environment Variables

Before building, you must configure environment variables that point to your Python installations:

**Step 1: Locate Your Python Installations**
1. Open Command Prompt
2. For Python 3.11: `where python3.11` or check `C:\Users\[Username]\AppData\Local\Programs\Python\Python311`
3. For Python 3.14: `where python3.14` or check `C:\Users\[Username]\AppData\Local\Programs\Python\Python314`

**Step 2: Set Environment Variables**
1. Open **System Properties** → **Advanced** → **Environment Variables**
2. Under **System Variables**, click **New** and add:
   - **Variable name**: `PYTHON_3_11_HOME`
   - **Variable value**: `C:\Users\[Username]\AppData\Local\Programs\Python\Python311` (your actual path)
3. Click **New** again and add:
   - **Variable name**: `PYTHON_3_14_HOME`
   - **Variable value**: `C:\Users\[Username]\AppData\Local\Programs\Python\Python314` (your actual path)
4. Click **OK** to save all changes
5. **Restart Visual Studio** for changes to take effect

**Step 3: Verify Setup**
Open a new Command Prompt and verify:
```cmd
echo %PYTHON_3_11_HOME%
echo %PYTHON_3_14_HOME%
```

Both should display your Python installation paths.

#### Switching Between Python Versions

The new build system uses Visual Studio Property Sheets to manage Python configurations:

**Method 1: Using Property Manager (Recommended)**
1. Open the GSPy solution in Visual Studio
2. Go to **View** → **Property Manager**
3. Expand your project configurations (Debug|x64, Release|x64)
4. Right-click on a configuration and select **Add Existing Property Sheet**
5. Choose either:
   - `python_311.props` for Python 3.11
   - `python_314.props` for Python 3.14
6. Build the project

**Method 2: Using Solution Explorer**
1. Right-click the GSPy project in Solution Explorer
2. Select **Add** → **Existing Item**
3. Choose the desired `.props` file
4. The Property Sheet will be applied to all configurations

**Switching Between Versions:**
1. In Property Manager, right-click the current Property Sheet and select **Remove**
2. Add the desired Property Sheet using the steps above
3. Rebuild the solution

#### Build Configurations

The system supports these combinations:

| Configuration | Platform | Property Sheet | Target Python | Output |
|---------------|----------|----------------|---------------|---------|
| Debug | x64 | python_311.props | Python 3.11 | GSPy_Debug_311.dll |
| Release | x64 | python_311.props | Python 3.11 | GSPy_Release_311.dll |
| Debug | x64 | python_314.props | Python 3.14 | GSPy_Debug_314.dll |
| Release | x64 | python_314.props | Python 3.14 | GSPy_Release_314.dll |

#### Typical Development Workflows

**Workflow 1: Single Python Version Development**
1. Set up environment variables for your target Python version
2. Add the corresponding Property Sheet to your project
3. Develop and test normally
4. Build in Release mode for distribution

**Workflow 2: Multi-Version Testing**
1. Set up environment variables for both Python versions
2. Add Python 3.11 Property Sheet
3. Build and test your changes
4. Switch to Python 3.14 Property Sheet
5. Build and test again to ensure compatibility
6. Create release builds for both versions

**Workflow 3: Team Development**
1. Each developer sets up their own environment variables
2. Property Sheets are shared in the repository
3. Developers can work with their preferred Python version
4. CI/CD builds both versions automatically

#### Troubleshooting

**Build Error: "Cannot find Python headers"**
- Verify `PYTHON_X_XX_HOME` environment variables are set correctly
- Ensure the paths point to the root Python installation directory
- Restart Visual Studio after setting environment variables
- Check that Python installation includes development headers

**Build Error: "Cannot find python3XX.lib"**
- Verify Python was installed with development libraries
- Check that `[PYTHON_HOME]\libs\` directory exists and contains `python3XX.lib`
- Ensure you're using a 64-bit Python installation

**Property Sheet Not Taking Effect**
- Verify the Property Sheet is listed in Property Manager
- Check that it's applied to the correct configuration (Debug/Release x64)
- Try removing and re-adding the Property Sheet
- Clean and rebuild the solution

**Environment Variables Not Recognized**
- Restart Visual Studio completely after setting variables
- Verify variables are set at System level, not User level
- Test variables in Command Prompt: `echo %PYTHON_3_11_HOME%`

**NumPy Headers Not Found**
- Ensure NumPy is installed: `pip install numpy`
- Verify NumPy installation: `python -c "import numpy; print(numpy.__file__)"`
- Check that `[PYTHON_HOME]\Lib\site-packages\numpy\_core\include` exists

**Multiple Python Versions Conflict**
- Use specific Python executables: `python3.11 -m pip install numpy`
- Verify each Python installation has its own NumPy: `python3.11 -c "import numpy"`
- Ensure environment variables point to correct Python versions

#### Team Onboarding

**For New Developers:**
1. **Install Prerequisites:**
   - Visual Studio 2022 with C++ workload
   - Python 3.11 and/or 3.14 (64-bit)
   - NumPy for each Python version

2. **Configure Environment:**
   - Set `PYTHON_3_11_HOME` and `PYTHON_3_14_HOME` environment variables
   - Restart Visual Studio

3. **Verify Setup:**
   - Clone the repository
   - Open GSPy.sln in Visual Studio
   - Add a Property Sheet (python_311.props or python_314.props)
   - Build the solution successfully

4. **Test Your Setup:**
   - Run the example projects in the `examples/` folder
   - Verify the DLL works with your Python installation

**For Project Leads:**
- Ensure all team members follow the same environment variable naming convention
- Document any project-specific Python package requirements
- Consider creating setup scripts for automated environment configuration
- Establish team standards for which Python versions to support

#### Advanced Configuration

**Adding New Python Versions:**
1. Create a new Property Sheet file (e.g., `python_312.props`)
2. Copy the structure from existing Property Sheets
3. Update paths to use `$(PYTHON_3_12_HOME)` environment variable
4. Update library reference to `python312.lib`
5. Set up the corresponding environment variable

**Custom Python Installations:**
- Environment variables can point to any Python installation location
- Useful for custom builds, conda environments, or portable Python
- Ensure the installation includes development headers and libraries

**Build Automation:**
- CI/CD systems can set environment variables programmatically
- Use batch scripts to switch between Python versions automatically
- Consider PowerShell scripts for complex build workflows

## Building from Source

To build the C++ DLL from source, you will need:

  * Visual Studio 2022 with the "Desktop development with C++" workload.
  * Environment variables configured for your target Python version (see Developer Documentation above).
  * A Property Sheet applied for your target Python version.
  * Compile in **Release** mode for the **x64** platform.

### Updating Version Numbers

To increment the GSPy version, edit only these 3 constants in `GSPy.h`:

```cpp
#define GSPY_VERSION_MAJOR 1
#define GSPY_VERSION_MINOR 8  
#define GSPY_VERSION_PATCH 0
```

All version strings, log headers, and GoldSim version reporting update automatically.

-----

## License

This project is licensed under the MIT License.

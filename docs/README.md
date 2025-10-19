# GSPy: The GoldSim-Python Bridge

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
  * **Dynamic Array Sizing:** Define the size of input vectors and matrices at runtime using scalar values from your GoldSim model.
  * **Error Handling:** Python exceptions are caught gracefully and reported directly to the GoldSim user, simplifying debugging.
  * **Diagnostic Logging:** Automatically generates a log file for each run, providing a clear trace of the DLL's operations.

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

To use the pre-compiled GSPy DLL, you will need:

1.  **GoldSim 14+** (The provided DLL is 64-bit).
2.  A 64-bit installation of **Python** (version 3.8 or newer is recommended).
3.  The **NumPy** Python package. It can be installed by opening a command prompt and running:
    ```
    pip install numpy
    ```

-----

## Python Setup & Troubleshooting

### Common Python Path Issues

The most frequent issue users encounter is GSPy not finding Python. Here are the solutions:

#### **Method 1: Use Full Python Path (Recommended)**
In your JSON configuration, specify the complete path to your Python installation:

```json
{
  "python_path": "C:\\Users\\YourUsername\\AppData\\Local\\Programs\\Python\\Python312",
  "script_path": "your_script.py",
  ...
}
```

**How to find your Python path:**
1. Open Command Prompt (Windows Key + R, type `cmd`)
2. Type: `where python`
3. Use the directory path (remove `\\python.exe` from the end)

#### **Method 2: Add Python to System PATH**
If you want to use a shorter path in your JSON config:

1. **Windows 11/10**: Search for "Environment Variables" in Start Menu
2. Click **"Edit the system environment variables"**
3. Click **"Environment Variables..."** button
4. Under **"System variables"**, find and select **"Path"**, then click **"Edit..."**
5. Click **"New"** and add your Python installation directory (e.g., `C:\Users\YourUsername\AppData\Local\Programs\Python\Python312`)
6. Click **"New"** again and add the Scripts directory (e.g., `C:\Users\YourUsername\AppData\Local\Programs\Python\Python312\Scripts`)
7. Click **"OK"** on all dialogs
8. **Restart your computer** for changes to take effect

After adding to PATH, you can use shorter paths in your JSON:
```json
{
  "python_path": "C:\\Python312",
  ...
}
```

### Verifying Your Python Setup

Before using GSPy, verify your Python installation:

1. **Open Command Prompt**
2. **Test Python**: Type `python --version` - should show Python 3.8+
3. **Test NumPy**: Type `python -c "import numpy; print('NumPy OK')"` - should print "NumPy OK"

If either command fails, you need to fix your Python installation before using GSPy.

### Installation Tips for Windows Users

- **Use the official Python installer** from [python.org](https://python.org)
- **Check "Add Python to PATH"** during installation
- **Choose "Install for all users"** if you have admin rights
- **Use Python 3.9-3.12** for best compatibility (3.13+ may have issues)
- **Always use 64-bit Python** (required for GSPy)

### If GSPy Still Can't Find Python

1. **Check your JSON `python_path`** matches your actual Python installation directory
2. **Verify Python is 64-bit**: `python -c "import platform; print(platform.architecture())"`
3. **Check file permissions** - ensure GSPy can read the Python directory
4. **Try absolute paths** in your JSON config rather than relative paths
5. **Restart GoldSim** after changing environment variables

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
  "python_path": "C:\\Users\\username\\AppData\\Local\\Programs\\Python\\Python313",
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
  * **`log_level`** (Optional): Controls logging verbosity for performance optimization. Default is 2 (INFO).
      * **`0`** = ERROR only (fastest, recommended for production)
      * **`1`** = ERROR + WARNING
      * **`2`** = ERROR + WARNING + INFO (default)
      * **`3`** = ERROR + WARNING + INFO + DEBUG (slowest, development only)

### Performance Optimization

For production simulations, add `"log_level": 0` to your JSON configuration to minimize logging overhead:

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

This eliminates ~90-95% of log writes, significantly improving simulation performance while preserving error reporting.

### Python Script API

  * Your function (e.g., `process_data`) must accept arguments using **`*args`**.
  * Inputs are passed in a tuple (`args`) in the order defined in the JSON.
  * Your function **must** return a **tuple** of results, even if there is only one (e.g., `return (my_result,)`). The order must match the JSON `outputs`.

#### Python Logging

Python scripts can write custom messages to the GSPy log file using the built-in `gspy` module:

```python
import gspy

def process_data(*args):
    # Write messages at different log levels
    gspy.log("Starting calculation", 2)  # INFO level
    gspy.log("Debug info: processing input", 3)  # DEBUG level  
    gspy.log("Warning: unusual input value", 1)  # WARNING level
    gspy.log("Critical error occurred", 0)  # ERROR level
    
    # Default level is INFO (2) if not specified
    gspy.log("Simple message")
    
    # Your calculation logic here...
    return (result,)
```

**Log Levels:** 0=ERROR, 1=WARNING, 2=INFO (default), 3=DEBUG

#### Data Type Mapping

| From GoldSim | Received in Python as... |
| :--- | :--- |
| Scalar | `float` |
| Vector | 1D NumPy Array |
| Matrix | 2D NumPy Array |
| Time Series |Python Dictionary with keys: `"timestamps"`, `"data"`, `"time_basis"`, `"data_type"`  |

| To GoldSim | Returned from Python as... |
| :--- | :--- |
| Scalar | `float` or `int` |
| Vector | 1D NumPy Array |
| Matrix | 2D NumPy Array |
| Time Series |  Python Dictionary with keys: `"timestamps"`, `"data"`, `"time_basis"`, `"data_type"` |
| Lookup Table | Python Dictionary |

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

"table_dim": An integer, 1 for a 1D table or 2 for a 2D table.

"row_labels": A 1D NumPy array of monotonically increasing values for the row headers.

"col_labels": A 1D NumPy array of monotonically increasing values for the column headers. (This key should be omitted for a 1D table).

"data": A 1D or 2D NumPy array containing the table's body data. Its shape must match the labels.

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
  "python_path": "C:\\Users\\JasonLillywhite\\AppData\\Local\\Programs\\Python\\Python313",
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

## Building from Source

To build the C++ DLL from source, you will need:

  * Visual Studio 2022 with the "Desktop development with C++" workload.
  * Project properties configured to point to your Python and NumPy `include` and `libs` directories.
  * Compile in **Release** mode for the **x64** platform.

-----

## License

This project is licensed under the MIT License.

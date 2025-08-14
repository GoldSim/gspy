# GSPy - GoldSim Python Interface

A DLL interface that enables integration between GoldSim and Python, allowing complex calculations to be implemented in Python while maintaining full integration with GoldSim simulations.

## What GSPy Is (and Is Not)

**What GSPy Is**: A lightweight, open-source C++ bridge that connects GoldSim to a user's own standard Python environment. It is provided as a utility for advanced users.

**What GSPy Is Not**: GSPy is **not** a formal GoldSim product, a Python distribution, or a formally supported tool. GoldSim support cannot provide assistance for custom Python code, library usage, or environment configuration issues.

## IMPORTANT DISCLAIMER

**USE AT YOUR OWN RISK**: GSPy is provided "as is" without any warranty. Users are responsible for thorough testing and validation in their specific environment before production use. See [LICENSE](LICENSE) and [SECURITY.md](SECURITY.md) for complete terms and security considerations.


## Prerequisites

- **GoldSim**: Version 14.0 or newer
- **Python**: Version 3.8 or newer (tested up to 3.13)
- **Windows**: 64-bit
- **Required Python Packages**: NumPy, pandas

## Overview

GSPy provides a bridge between GoldSim's External Element and Python scripts, featuring automatic script loading, intelligent I/O detection, and comprehensive error handling.

### Features

- **Zero Configuration**: Automatically loads `gspy_script.py` and detects interface requirements
- **Automatic I/O detection**: No manual setup - GSPy reads your script's `gspy_info()` function
- **Robust GIL management**: Proper Global Interpreter Lock handling prevents access violations
- **Comprehensive error handling**: Detailed logging and error reporting
- **Modern C++ implementation**: Clean, maintainable codebase with proper memory management
- **Rich Data Type Support**:
    - Scalars (single double values)
    - 1D and 2D arrays (NumPy arrays)
    - GoldSim Time Series (Python dictionaries or pandas Series)
    - GoldSim Lookup Tables (Python dictionaries)


## Setup and Configuration

Follow these steps to set up GSPy with your GoldSim model:

1. **Obtain the DLL**
    - Build or download the compiled `GSPy.dll` file.

2. **Place Required Files**
    - Put `GSPy.dll` in the same directory as your GoldSim model file (or another known location).
    - Add your Python script (e.g., `gspy_script.py`) to the same directory.

3. **Configure GoldSim External Element**
    - In GoldSim, add an External Element.
    - Set the DLL Path to the location of `GSPy.dll`.
    - In the External Element properties, specify the Python script and function to call (usually `gspy_script.py` and `goldsim_calculate`).

4. **Reference Your Python Script and Function**
    - Ensure your script implements both `gspy_info()` and `goldsim_calculate(inputs)` functions as described below.

5. **Run Your Simulation**
    - GoldSim will automatically load and execute your Python script using GSPy.

**Note:** GSPy is not a standard Python package. You do not install it with `pip`. It is a DLL that integrates with GoldSim.

### Adding More Inputs and Outputs

To handle multiple inputs and outputs, simply update the counts in `gspy_info()` and add corresponding logic:

```python
def gspy_info():
    """Example with 3 inputs and 2 outputs."""
    return {'inputs': 3, 'outputs': 2}

def goldsim_calculate(inputs):
    """Handle multiple inputs and outputs."""
    # Access inputs by key: input1, input2, input3, etc.
    value1 = inputs.get('input1', 0.0)
    value2 = inputs.get('input2', 0.0) 
    value3 = inputs.get('input3', 0.0)
    
    # Perform calculations
    result1 = value1 + value2
    result2 = value1 * value3
    
    # Return outputs by key: output1, output2, etc.
    return {
        'output1': result1,
        'output2': result2
    }
```

**Key Points**:
- Input keys are always `input1`, `input2`, `input3`, etc.
- Output keys are always `output1`, `output2`, `output3`, etc.
- The number of inputs/outputs in `gspy_info()` must match what you access in `goldsim_calculate()`
- **The `.get()` method**: `inputs.get('input1', 0.0)` safely retrieves the value or returns `0.0` if the key doesn't exist, preventing crashes

### Time Series Outputs

GSPy supports creating time series outputs from Python. Use the special key format `goldsim_timeseries_output_N` where N is the output number:

```python
def gspy_info():
    """Time series output requires more output slots (8 header + 2*N data points)."""
    return {'inputs': 1, 'outputs': 14}  # For 3 time points: 8 + (2*3) = 14

def goldsim_calculate(inputs):
    """Create a time series output."""
    scaling_factor = inputs.get('input1', 1.0)
    
    # Define time points and values
    time_points = [0.0, 33.0, 100.0]
    data_values = [scaling_factor * (t * 0.5) for t in time_points]
    
    # Return as time series
    return {
        'goldsim_timeseries_output_1': {
            'is_calendar': False,           # False for elapsed time, True for calendar dates
            'data_type': 'instantaneous',   # 'instantaneous', 'constant', 'change', or 'discrete'
            'times': time_points,
            'values': data_values
        }
    }
```

**Time Series Output Requirements**:
- Use key format: `goldsim_timeseries_output_1`, `goldsim_timeseries_output_2`, etc.
- Include `is_calendar`, `data_type`, `times`, and `values` fields
- Calculate output count as: 8 (header) + 2 × (number of time points)
- `times` and `values` lists must have the same length

### Error Reporting

GSPy automatically captures Python exceptions and reports them to GoldSim as error messages. When an error occurs in your script, GoldSim will display the Python error message in a popup dialog.

```python
def goldsim_calculate(inputs):
    """Example with error handling."""
    try:
        value = inputs.get('input1', 0.0)
        
        # Your calculation logic here
        if value < 0:
            raise ValueError("Input value cannot be negative")
            
        result = value * 2.0 + 10.0
        return {'output1': result}
        
    except Exception as e:
        # GSPy will automatically catch this exception and show it to the user
        raise RuntimeError(f"Calculation failed: {str(e)}")
```

**How it works**:
- Any unhandled Python exception in `goldsim_calculate()` is caught by GSPy
- The error message is automatically forwarded to GoldSim
- GoldSim displays the error in a popup dialog
- No special error handling functions are needed - just use standard Python exceptions

## Requirements

- **Python 3.13** (or compatible version)
- **Windows 64-bit**
- **NumPy** (optional, for numerical computations)


## Supported Data Types

### Input Types
- **Scalar**: Single numeric values
- **Vector**: Arrays of numeric values
- **Time Series**: GoldSim time series data (automatically unpacked into dictionaries)

### Output Types
- **Scalar**: Single numeric values
- **Vector**: Arrays of numeric values
- **Time Series**: GoldSim time series data (automatically packed from dictionaries)
- **Lookup Table**: GoldSim lookup table data (automatically packed from dictionaries)

## Python Script Interface

### Required Functions

Your `gspy_script.py` must implement these two functions:

```python
def gspy_info():
    """
    Define the interface specification.
    
    Returns:
        dict: Must contain 'inputs' and 'outputs' keys with integer values
    """
    return {
        'inputs': 1,    # Number of input arguments expected
        'outputs': 1    # Number of output arguments to return
    }

def goldsim_calculate(inputs):
    """
    Main calculation function called by GSPy.
    
    Args:
        inputs (dict): Input values from GoldSim
            - Keys: 'input1', 'input2', etc. for scalar/vector inputs
            - Time series inputs are automatically unpacked into structured dictionaries
            
    Returns:
        dict: Output values for GoldSim
            - Keys: 'output1', 'output2', etc.
            - Values: numeric values or lists for vector outputs
    """
    # Your calculation logic here
    pass
```

## Examples

The `examples/` directory contains working examples for common use cases:

- **`Scalar to Scalar`**: Basic scalar input → scalar output
- **`Scalar to Vector`**: Scalar input → vector output
- **`Vector to Scalar.py`**: Vector input → scalar output
- **`LookupTable Output`**: Generate an import a lookup table into GoldSim
- **`Time Series Input`**: Time series input → scalar output 
- **`Time Series Output`**: Scalar input → time series output
- **`Numpy Library`**: Demonstrate how to import python libraries

To use an example:
1. Copy the desired example file to your model directory
2. Rename it to `gspy_script.py`
3. Run your GoldSim model

## Building from Source

### Prerequisites
- **Visual Studio 2022** with C++ development tools
- **Python 3.13** development libraries

### Build Steps
1. Open `GSPy.sln` in Visual Studio 2022
2. Select "Release" configuration and "x64" platform
3. Build → Build Solution
4. Output: `x64\Release\GSPy.dll`

## Architecture

### GIL Management
GSPy properly handles Python's Global Interpreter Lock (GIL) to prevent access violations:

```cpp
PyGILState_STATE gstate = PyGILState_Ensure();
// Safe Python API calls here
PyGILState_Release(gstate);
```

### Automatic I/O Detection
GSPy calls your script's `gspy_info()` function to automatically determine interface requirements, eliminating manual configuration in most cases.

### GoldSim Integration
GSPy implements the full GoldSim External Function protocol:
- **XF_INITIALIZE**: Python interpreter setup and script loading
- **XF_CALCULATE**: Function execution with data marshalling
- **XF_REP_VERSION**: Version reporting (1.0)
- **XF_REP_ARGUMENTS**: Automatic I/O count reporting
- **XF_CLEANUP**: Resource cleanup and interpreter finalization

## Troubleshooting

### Common Issues

**Problem**: DLL crashes or access violations  
**Solution**: Use the latest GSPy.dll with proper GIL management

**Problem**: "gspy_script.py not found"  
**Solution**: Ensure the script is in the same directory as GSPy.dll

**Problem**: "goldsim_calculate function not found"  
**Solution**: Verify your script implements both required functions exactly as specified

**Problem**: Incorrect input/output counts  
**Solution**: Check your `gspy_info()` function returns the correct counts

### Debug Logging
GSPy creates detailed logs for troubleshooting. Check the console output and any generated log files for diagnostic information.

## Known Limitations

- **Python Version**: Currently tested with Python 3.13. Other versions may require recompilation

## Version History

- **v1.0**: Initial release with automatic I/O detection and robust error handling

## Legal Notice

**DISCLAIMER**: GSPy is provided "as is" without warranty of any kind. Users assume full responsibility for testing, validation, and use of this software. By using GSPy, you acknowledge and accept the terms outlined in the [LICENSE](LICENSE) file.

**Important Documents**:
- [LICENSE](LICENSE) - Complete license terms and disclaimers
- [SECURITY.md](SECURITY.md) - Security guidelines and risk assessment
- [CONTRIBUTING.md](CONTRIBUTING.md) - Guidelines for contributors

**Contact**: For questions about GSPy, please visit [GoldSim Technology Group](https://www.goldsim.com)

---
*Copyright (c) 2025 GoldSim Technology Group. All rights reserved.*

## License

This project is provided for use with GoldSim simulations. Please refer to your GoldSim license for usage terms.
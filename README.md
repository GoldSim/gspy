# GSPy - GoldSim Python Interface

A DLL interface that enables integration between GoldSim and Python, allowing complex calculations to be implemented in Python while maintaining full integration with GoldSim simulations.

## What GSPy Is (and Is Not)

**What GSPy Is**: A lightweight, open-source C++ bridge that connects GoldSim to a user's own standard Python environment. It is provided as a utility for advanced users.

**What GSPy Is Not**: GSPy is **not** a formal GoldSim product, a Python distribution, or a formally supported tool. GoldSim support cannot provide assistance for custom Python code, library usage, or environment configuration issues.

## IMPORTANT DISCLAIMER

**USE AT YOUR OWN RISK**: GSPy is provided "as is" without any warranty. Users are responsible for thorough testing and validation in their specific environment before production use. See [LICENSE](LICENSE) and [SECURITY.md](SECURITY.md) for complete terms and security considerations.

## Overview

GSPy provides a bridge between GoldSim's External Element and Python scripts, featuring automatic script loading, intelligent I/O detection, and comprehensive error handling.

### Features

- **Zero Configuration**: Automatically loads `gspy_script.py` 
- **Universal Data Type Support**: Scalars, vectors, lookup tables, time series (both input and output)
- **High Performance**: Optimized Python integration with thread-safe execution
- **Robust Error Handling**: Comprehensive fallbacks prevent crashes, detailed logging
- **Advanced Data Structures**: Full support for GoldSim's native data formats
- **Bidirectional Marshalling**: Seamless conversion between GoldSim and Python formats
- **NumPy Integration**: Full support for python libraries
- **Flexible I/O**: Handle any number of inputs and outputs automatically

## Quick Start

1. **Place files in the same directory**:
   - `GSPy.dll` (the interface DLL)
   - `gspy_script.py` (your Python script)
   - Your GoldSim model file

2. **Create your Python script** (`gspy_script.py`):
   ```python
   def gspy_info():
       """Define the interface specification."""
       return {'inputs': 1, 'outputs': 1}

   def goldsim_calculate(inputs):
       """Main function called by GoldSim."""
       input_value = inputs.get('input1', 0.0)
       result = input_value * 2.0 + 10.0
       return {'output1': result}
   ```

3. **Configure GoldSim External Element**:
   - DLL Path: `GSPy.dll`
   - **Inputs/Outputs**: Must match your script's `gspy_info()` function
   - Example: If your script returns `{'inputs': 5, 'outputs': 4}`, configure GoldSim for 5 inputs, 4 outputs

4. **Run your simulation** - GSPy automatically loads and executes your Python script!

> **💡 Important**: The input/output counts you configure in GoldSim must match what your Python script's `gspy_info()` function returns. This is standard GoldSim External Element behavior.

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

**Additional Notes**:
- Input keys are always `input1`, `input2`, `input3`, etc.
- Output keys are always `output1`, `output2`, `output3`, etc.
- The number of inputs/outputs in `gspy_info()` must match what you access in `goldsim_calculate()`
- **The `.get()` method**: `inputs.get('input1', 0.0)` safely retrieves the value or returns `0.0` if the key doesn't exist, preventing crashes

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

### **Fully Supported Input Types**
- **Scalar**: Single numeric values → `{'input1': 5.0}`
- **Vector**: Multiple numeric values → `{'input1': 1.0, 'input2': 2.0, ...}`
- **Time Series**: GoldSim time series → `{'input1': {'times': [...], 'values': [...]}}`

### **Fully Supported Output Types**
- **Scalar**: Single values → `{'output1': 5.0}`
- **Vector**: Lists of values → `{'output1': [1.0, 2.0, 3.0]}`
- **Multiple Scalars**: Separate outputs → `{'output1': 5.0, 'output2': 10.0}`
- **Lookup Tables**: 1D tables → `{'goldsim_lookup_table_1d_output_1': {...}}`
- **Time Series**: Native GoldSim format → `{'goldsim_timeseries_output_1': {...}}`

## Python Script Interface

### Required Functions

Your `gspy_script.py` must implement these two functions with **exact naming**:

```python
def gspy_info():
    """
    Define the interface specification.
    
    Returns:
        dict: Must contain EXACTLY 'inputs' and 'outputs' keys with integer values
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

## Complete Examples

The `examples/` directory contains **7 fully working examples** covering all supported data types:

| Example                 | Input → Output | Description                   | Libraries |
| ----------------------- | -------------- | ----------------------------- | --------- |
| **Scalar to Scalar**    | 1 → 1          | Basic mathematical operations | None      |
| **Scalar to Vector**    | 1 → 3          | Concentration calculations    | None      |
| **Vector to Scalar**    | 4 → 1          | Statistical analysis          | None      |
| **Lookup Table Output** | 1 → 8          | 1D interpolation tables       | None      |
| **Time Series Input**   | 14 → 1         | Time series processing        | None      |
| **Time Series Output**  | 1 → 14         | Time series generation        | None      |
| **NumPy Library**       | 5 → 4          | Advanced statistics           | NumPy     |

### **Using Examples:**
1. Navigate to any example directory (e.g., `examples/Scalar to Scalar/`)
2. Copy `GSPy.dll` to that directory
3. Open the `.gsm` file in GoldSim
4. Run the simulation - everything is pre-configured!

Each example includes:
- `gspy_script.py` - The Python script
- `*.gsm` - Pre-configured GoldSim model
- Complete documentation and comments

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

### Thread Safety
GSPy ensures thread-safe Python execution to prevent crashes and access violations when called from GoldSim's simulation engine.

### Automatic I/O Detection
GSPy calls your script's `gspy_info()` function to automatically determine interface requirements, eliminating manual configuration in most cases.

### GoldSim Integration
GSPy implements the full GoldSim External Function protocol:
- **XF_INITIALIZE**: Python interpreter setup and script loading
- **XF_CALCULATE**: Function execution with data marshalling
- **XF_REP_VERSION**: Version reporting (1.5)
- **XF_REP_ARGUMENTS**: Automatic I/O count reporting
- **XF_CLEANUP**: Resource cleanup and interpreter finalization

## Troubleshooting

### Common Issues

**Problem**: DLL crashes or access violations  
**Solution**: Use the latest GSPy.dll with improved thread safety

**Problem**: "gspy_script.py not found"  
**Solution**: Ensure the script is in the same directory as GSPy.dll

**Problem**: "goldsim_calculate function not found"  
**Solution**: Verify your script implements both required functions exactly as specified

**Problem**: Incorrect input/output counts or "arguments do not match" errors  
**Solution**: Ensure your `gspy_info()` function uses EXACTLY these key names:
```python
return {'inputs': X, 'outputs': Y}  # Correct
return {'num_inputs': X, 'outputs': Y}  # Wrong - will default to 1,1
```

### Debug Logging
GSPy creates detailed logs for troubleshooting. Check the console output and any generated log files for diagnostic information.

## **Production Ready**

GSPy v2.0 is a **complete, production-ready system** with:
- **All major GoldSim data types supported**
- **Comprehensive error handling and fallbacks**
- **High performance with optimized Python integration**
- **Extensive testing across all example types**
- **Full documentation and working examples**

## System Requirements

- **Python**: 3.9+ (tested with 3.13.2)
- **Platform**: Windows x64
- **GoldSim**: Compatible with External Element interface
- **Optional**: NumPy for advanced numerical examples

## Version History

- **v1.5**: **Major Release** - Complete rewrite with full data type support
  - All GoldSim data types (scalars, vectors, lookup tables, time series)
  - Bidirectional data marshalling
  - NumPy integration
  - Robust error handling with fallbacks
  - Performance optimization
  - 7 complete working examples

- **v1.0**: Initial release with basic scalar support

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

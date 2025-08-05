# GSPy - GoldSim Python Interface

A DLL interface that enables integration between GoldSim and Python, allowing complex calculations to be implemented in Python while maintaining full integration with GoldSim simulations.

## IMPORTANT DISCLAIMER

**USE AT YOUR OWN RISK**: GSPy is provided "as is" without any warranty. Users are responsible for thorough testing and validation in their specific environment before production use. See [LICENSE](LICENSE) and [SECURITY.md](SECURITY.md) for complete terms and security considerations.

## Overview

GSPy provides a bridge between GoldSim's External Element and Python scripts, featuring automatic script loading, intelligent I/O detection, and comprehensive error handling.

### Key Features

- **Convention-based script loading**: Automatically loads `gspy_script.py` from the DLL directory
- **Automatic I/O detection**: DLL automatically determines input/output counts via `gspy_info()`
- **Robust GIL management**: Proper Global Interpreter Lock handling prevents access violations
- **Multiple data types**: Support for scalar, vector, and time series data (input only)
- **Comprehensive error handling**: Detailed logging and error reporting
- **Modern C++ implementation**: Clean, maintainable codebase with proper memory management

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
   - Number of Inputs: Auto-detected (or specify manually)
   - Number of Outputs: Auto-detected (or specify manually)

4. **Run your simulation** - GSPy will automatically load and execute your Python script.

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
- **Time Series**: Not currently supported (known limitation)

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

- **`1_scalar_to_scalar.py`**: Basic scalar input → scalar output
- **`2_scalar_to_vector.py`**: Scalar input → vector output (e.g., concentration calculations)
- **`3_vector_to_scalar.py`**: Vector input → scalar output (e.g., statistical analysis)
- **`4_lookup_table_1d.py`**: 1D interpolation and lookup tables
- **`5_timeseries_to_scalar.py`**: Time series input → scalar output (e.g., statistical analysis)
- **`6_numpy_statistics.py`**: Includes the numpy library

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

- **Time Series Output**: Creating time series outputs from Python is not currently supported due to format compatibility issues
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
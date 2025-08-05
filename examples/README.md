# GSPy Example Scripts

This directory contains working example Python scripts demonstrating various inpu- **Use NumPy**: For numerical computations, NumPy provides better performance and advanced functions
- **Debug with prints**: Use `print()` statements - output appears in GoldSim's console
- **Import libraries**: Popular libraries like NumPy, SciPy, Pandas work seamlessly
- **Handle exceptions**: Use try-except blocks to provide graceful error handlingoutput patterns with the GSPy DLL.

## How to Use an Example

1. **Find the example script** that best matches your needs (e.g., `1_scalar_to_scalar.py`)

2. **Copy the example file** to the same directory as your GoldSim model and `GSPy.dll`

3. **Important:** Rename the copied file to exactly `gspy_script.py`

4. **Configure GoldSim:** The GSPy DLL features **automatic I/O detection**! The DLL automatically determines the correct number of inputs and outputs by calling the `gspy_info()` function in your script

5. **Run your GoldSim model**

## Available Examples

### 1_scalar_to_scalar.py 
- **Purpose:** Basic scalar input to scalar output
- **Inputs:** 1 scalar value (`input1`)
- **Outputs:** 1 scalar value (`output1`)
- **Calculation:** `output1 = input1 * 10`
- **Use Case:** Simple mathematical transformations

### 2_scalar_to_vector.py
- **Purpose:** Scalar input to vector output
- **Inputs:** 1 scalar value (`input1` - flow rate)
- **Outputs:** 1 vector value (`output1` - 3-element concentration array)
- **Calculation:** Concentrations A, B, C based on inverse flow rate
- **Use Case:** Chemical concentration calculations, multi-component analysis

### 3_vector_to_scalar.py
- **Purpose:** Vector input to scalar output
- **Inputs:** 1 vector value (`input1` - array of measurements)
- **Outputs:** 1 scalar value (`output1` - statistical result)
- **Calculation:** Mean, standard deviation, or other statistical analysis
- **Use Case:** Data aggregation, statistical analysis

### 4_lookup_table_1d.py 
- **Purpose:** 1D interpolation and lookup tables
- **Inputs:** 1 scalar value (`input1` - lookup key)
- **Outputs:** 1 scalar value (`output1` - interpolated result)
- **Calculation:** Linear interpolation from predefined data table
- **Use Case:** Property lookups, empirical correlations

### 5_timeseries_to_scalar.py
- **Purpose:** Time series input to scalar output
- **Inputs:** 1 time series (automatically unpacked by GSPy)
- **Outputs:** 1 scalar value (`output1` - statistical result)
- **Calculation:** Mean, maximum, or other time series analysis
- **Use Case:** Time series analysis, statistical summaries of temporal data

### 6_numpy_statistics.py
- **Purpose:** Advanced statistical analysis using NumPy
- **Inputs:** 5 scalar values (`input1` through `input5` - measurement data)
- **Outputs:** 4 scalar values (mean, std dev, min, max)
- **Calculation:** Statistical summary with outlier detection using NumPy
- **Libraries:** Demonstrates NumPy integration for efficient numerical operations
- **Use Case:** Scientific data analysis, quality control, measurement validation

## Example Script Structure

All examples follow this standard structure:

```python
def gspy_info():
    """Define the interface specification."""
    return {
        'inputs': 1,    # Number of inputs expected
        'outputs': 1    # Number of outputs returned
    }

def goldsim_calculate(inputs):
    """Main calculation function."""
    # Extract inputs
    input_value = inputs.get('input1', 0.0)
    
    # Perform calculation
    result = your_calculation(input_value)
    
    # Return results
    return {'output1': result}
```

## Input/Output Types

### Supported Input Types
- **Scalar**: Single numeric values (accessed as `inputs['input1']`)
- **Vector**: Numeric arrays (accessed as `inputs['input1']` - returns a list)
- **Time Series**: Automatically unpacked into structured dictionaries containing:
  - `times`: List of time points
  - `values`: List of corresponding values
  - `header`: Metadata about the time series

### Supported Output Types
- **Scalar**: Single numeric values (`{'output1': 42.0}`)
- **Vector**: Numeric arrays (`{'output1': [1.0, 2.0, 3.0]}`)

### Known Limitations
- **Time Series Output**: Creating time series outputs is not currently supported due to format compatibility issues

## Tips for Development

1. **Test incrementally**: Start with simple examples and gradually add complexity
2. **Use error handling**: Wrap calculations in try-except blocks
3. **Validate inputs**: Check input types and ranges before processing
4. **Use NumPy**: For numerical computations, NumPy provides better performance
5. **Debug with prints**: Use `print()` statements - output appears in GoldSim's console

## Performance Considerations

- **Import overhead**: Python imports occur once during initialization
- **Function call overhead**: Minimal for each calculation
- **NumPy arrays**: More efficient for large datasets than Python lists
- **Memory usage**: GSPy properly manages memory between GoldSim and Python

## Getting Help

If you encounter issues:
1. Check the main README.md for troubleshooting guidance
2. Verify your script implements both required functions (`gspy_info` and `goldsim_calculate`)
3. Test your Python script independently before integrating with GoldSim
4. Check GoldSim's console output for detailed error messages
- **GoldSim Config:** 1 input, 1 output (output configured as vector of size 3)

### 2b_scalar_to_three_scalars.py
- **Purpose:** Scalar input to three separate scalar outputs
- **Inputs:** 1 scalar value (`input1` - flow rate)
- **Outputs:** 3 scalar values (`output1`, `output2`, `output3` - individual concentrations)
- **Calculation:** Same as 2_scalar_to_vector.py but returns separate scalars
- **GoldSim Config:** 1 input, 3 outputs (all scalars)

### 3_vector_to_scalar.py
- **Purpose:** Vector input to scalar output (sum calculation)
- **Inputs:** 4 scalar values (`input1`, `input2`, `input3`, `input4` - vector elements)
- **Outputs:** 1 scalar value (`output1` - sum of all inputs)
- **Calculation:** Sums all input vector elements
- **GoldSim Config:** 4 inputs (all scalars), 1 output (scalar)

### 4_lookup_table_1d.py
- **Purpose:** Generate native GoldSim 1D lookup table from scaling factor
- **Inputs:** 1 scalar value (`input1` - scaling factor)
- **Outputs:** 1 native 1D lookup table (24 internal values: 2 + 2×11)
- **Calculation:** Scales base temperature-reaction rate table by input factor
- **GoldSim Config:** 1 input, 1 output (configured as 1D table type)
- **Use Cases:** Dynamic table generation, parameter studies, scenario analysis, calibrated lookup tables

## Testing an Example

You can test any example script directly by running it with Python:

```bash
python 1_scalar_to_scalar.py
```

This will run the built-in test case and show you the expected behavior.

## Creating Your Own Script

1. Start with the example that's closest to your needs
2. Copy and rename it to `gspy_script.py`
3. Modify the `goldsim_calculate()` function to implement your logic
4. Update the GoldSim configuration to match your inputs/outputs
5. Test and iterate

## Need Help?

- Check that your script is named exactly `gspy_script.py`
- Verify your GoldSim External element configuration matches the script
- Use the built-in test functionality to debug your calculations
- Check the GSPy log file (`gspy_log.txt`) for detailed error information
# GSPy Example 5: Time Series Input, Scalar Output

def gspy_info():
    """
    Declares the I/O interface for this script.
    This example is configured for a test Time Series with 3 data points.
    Per the GoldSim manual, this requires 8 (header) + (2 * 3) (data) = 14 input arguments.
    It returns 1 scalar output.
    """
    return {'inputs': 14, 'outputs': 1}

def goldsim_calculate(inputs):
    """
    This script receives a native GoldSim Time Series as a dictionary,
    calculates the mean of its values, and returns a single scalar result.
    
    GoldSim Configuration:
    - Inputs: 1 (Time Series Definition)
    - Outputs: 1 (scalar - mean value)
    
    The Time Series input will be automatically unpacked by the C++ DLL
    into a user-friendly dictionary format.
    """
    
    # DEBUG: Write debug info to a file so we can see what's happening
    try:
        with open('python_debug.txt', 'w') as f:
            f.write(f"DEBUG: inputs type = {type(inputs)}\n")
            f.write(f"DEBUG: inputs keys = {list(inputs.keys()) if isinstance(inputs, dict) else 'Not a dict'}\n")
            
            # We assume the Time Series is the first input provided by GoldSim.
            # The C++ DLL has already unpacked it into a dictionary for us.
            time_series_input = inputs.get('input1', {})
            
            f.write(f"DEBUG: time_series_input type = {type(time_series_input)}\n")
            f.write(f"DEBUG: time_series_input = {time_series_input}\n")
            
            # Get the list of values from the time series data
            values = time_series_input.get('values', [])
            times = time_series_input.get('times', [])
            header = time_series_input.get('header', [])
            
            f.write(f"DEBUG: times = {times}\n")
            f.write(f"DEBUG: values = {values}\n")
            f.write(f"DEBUG: header = {header}\n")
            
            # Calculate the mean, handling the case of no data to avoid errors
            if values:
                mean_value = sum(values) / len(values)
                f.write(f"DEBUG: calculated mean = {mean_value}\n")
            else:
                mean_value = 0.0
                f.write(f"DEBUG: no values, returning 0.0\n")
    except:
        # If debug file writing fails, just continue
        pass
    
    # Actual calculation (fallback in case debug fails)
    time_series_input = inputs.get('input1', {})
    values = time_series_input.get('values', [])
    
    if values:
        mean_value = sum(values) / len(values)
    else:
        mean_value = 0.0
    
    return {'output1': mean_value}
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

# Test the function if run directly
if __name__ == "__main__":
    print("Time Series to Scalar Test:")
    print("=" * 40)
    
    # Test cases with different time series data
    test_cases = [
        {
            'description': 'Simple 3-point series',
            'input1': {
                'times': [0.0, 1.0, 2.0],
                'values': [10.0, 20.0, 30.0]
            }
        },
        {
            'description': 'Variable values',
            'input1': {
                'times': [0.0, 5.0, 10.0],
                'values': [5.5, 15.2, 8.3]
            }
        },
        {
            'description': 'Single point',
            'input1': {
                'times': [0.0],
                'values': [42.0]
            }
        },
        {
            'description': 'Empty series',
            'input1': {
                'times': [],
                'values': []
            }
        }
    ]
    
    for test_case in test_cases:
        print(f"\nTest: {test_case['description']}")
        result = goldsim_calculate(test_case)
        
        time_series = test_case['input1']
        times = time_series.get('times', [])
        values = time_series.get('values', [])
        
        print(f"Times: {times}")
        print(f"Values: {values}")
        print(f"Mean: {result['output1']:.2f}")
        
        if values:
            expected_mean = sum(values) / len(values)
            print(f"Expected: {expected_mean:.2f}")
    
    print("\nExpected behavior:")
    print("- Time series data is unpacked into times and values lists")
    print("- Mean is calculated from the values list")
    print("- Empty series returns 0.0")
    print("- Single point returns that point's value")
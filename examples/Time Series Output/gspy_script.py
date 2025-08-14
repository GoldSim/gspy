# GSPy Example 6: Scalar Input to Time Series Output

def gspy_info():
    """This script takes 1 scalar input and returns 1 native Time Series output.
    A scalar Time Series with 3 points requires 8 (header) + (2 * 3) (data) = 14 output arguments."""
    return {
        'inputs': 1, 
        'outputs': 14
    }

def goldsim_calculate(inputs):
    """This script takes a single scalar input, uses it to generate a time series,
    and returns the data using the correct dictionary structure."""
    
    scaling_factor = inputs.get('input1', 1.0)
    
    # Define the required time points and calculate the values
    time_points = [0.0, 5.0, 20.0]
    data_values = [1.3, 1.8, 0.87]
    
    # Return the data in the correct dictionary format
    return {
        'goldsim_timeseries_output_1': {
            'is_calendar': False,
            'data_type': 'instantaneous',
            'times': time_points,
            'values': data_values
        }
    }
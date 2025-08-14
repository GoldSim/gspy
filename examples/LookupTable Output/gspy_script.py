# GSPy Example 4: Returning a 1D Lookup Table

def gspy_info():
    """
    Declares the I/O interface for this script.
    A 1D Table with 3 points requires 2 + (2 * 3) = 8 output arguments.
    """
    return {'inputs': 1, 'outputs': 8}

def goldsim_calculate(inputs):
    """
    This script generates data for a native GoldSim 1D Lookup Table.
    It must return both the independent and dependent values for the table.
    
    GoldSim Configuration:
    - Inputs: 1 (scalar - scaling factor)
    - Outputs: 1 (1D lookup table)
    
    The output format must be:
    [1.0, N, ...independent_values..., ...dependent_values...]
    where N is the number of table points.
    """
    
    scaling_factor = inputs.get('input1', 1.0)
    
    # Define the independent values
    independent_vals = [0.0, 10.0, 20.0]
    
    # Define base dependent values
    base_dependent_vals = [0.1, 0.3, 0.8]
    
    # Calculate the dependent values based on scaling factor
    dependent_vals = [val * scaling_factor for val in base_dependent_vals]
    
    # Return the data in the special dictionary format for 1D lookup table
    return {
        'goldsim_lookup_table_1d_output_1': {
            'independent_values': independent_vals,
            'dependent_values': dependent_vals
        }
    }
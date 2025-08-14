# GSPy Example 4: Returning a 1D Lookup Table

def gspy_info():
    """
    Declares the I/O interface for this script.
    A 1D Table with 11 points requires 2 + (2 * 11) = 24 output arguments.
    """
    return {'inputs': 1, 'outputs': 24}

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
    
    # Define the independent values (e.g., temperature points)
    independent_vals = [0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0]
    
    # Define base dependent values (e.g., reaction rates)
    base_dependent_vals = [0.1, 0.3, 0.8, 1.5, 2.5, 4.0, 6.2, 9.1, 12.8, 17.5, 23.0]
    
    # Calculate the dependent values based on scaling factor
    dependent_vals = [val * scaling_factor for val in base_dependent_vals]
    
    # Return the data in the special dictionary format for 1D lookup table
    return {
        'goldsim_lookup_table_1d_output_1': {
            'independent_values': independent_vals,
            'dependent_values': dependent_vals
        }
    }

# Test the function if run directly
if __name__ == "__main__":
    print("1D Lookup Table Generation Test:")
    print("=" * 50)
    
    # Test cases with different scaling factors
    test_cases = [1.0, 2.0, 0.5]
    
    for scaling_factor in test_cases:
        test_inputs = {'input1': scaling_factor}
        result = goldsim_calculate(test_inputs)
        
        print(f"\nScaling Factor: {scaling_factor}")
        table_data = result['goldsim_lookup_table_1d_output_1']
        
        print("Independent values (Temperature °C):")
        print("  ", table_data['independent_values'])
        print("Dependent values (Reaction Rate):")
        print("  ", table_data['dependent_values'])
    
    print("\nExpected GoldSim format:")
    print("- Dimension count: 1.0")
    print("- Number of points: 11")
    print("- Independent values: [0.0, 10.0, ..., 100.0]")
    print("- Dependent values: [scaled values based on input]")
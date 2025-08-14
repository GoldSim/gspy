# GSPy Example 2b: Scalar to Three Scalars
# This script takes a single numerical input and returns three separate scalar outputs
# (alternative to returning a single vector)

def gspy_info():
    """
    Declares the I/O interface for this script.
    This function is required for dynamic I/O detection.
    """
    return {'inputs': 1, 'outputs': 3}

def goldsim_calculate(inputs):
    """
    Takes one input named 'input1' and returns three separate scalar outputs
    named 'output1', 'output2', and 'output3'.
    
    GoldSim Configuration:
    - Inputs: 1 (scalar)
    - Outputs: 3 (all scalars)
    """
    flow_rate = inputs.get('input1', 0.0)
    
    # Calculate concentrations based on the flow rate
    concentration_A = 150.0 / flow_rate if flow_rate > 0 else 0
    concentration_B = 220.5 / flow_rate if flow_rate > 0 else 0
    concentration_C = 95.0 / flow_rate if flow_rate > 0 else 0
    
    # Return three separate scalar outputs
    return {
        'output1': concentration_A,
        'output2': concentration_B,
        'output3': concentration_C
    }

# Test the function if run directly
if __name__ == "__main__":
    test_inputs = {'input1': 10.0}
    result = goldsim_calculate(test_inputs)
    print(f"Test: goldsim_calculate({test_inputs}) = {result}")
    print(f"Flow rate: {test_inputs['input1']}")
    print(f"Concentration A: {result['output1']:.1f}")
    print(f"Concentration B: {result['output2']:.1f}")
    print(f"Concentration C: {result['output3']:.1f}")
    print("Expected: A=15.0, B=22.1, C=9.5")
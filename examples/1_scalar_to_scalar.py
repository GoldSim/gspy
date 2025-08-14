# GSPy Example 1: Scalar to Scalar
# This script takes a single numerical input and returns a single numerical output.

def gspy_info():
    """
    Declares the I/O interface for this script.
    This function is required for dynamic I/O detection.
    """
    return {'inputs': 1, 'outputs': 1}

def goldsim_calculate(inputs):
    """
    Takes one input named 'input1' and returns one output named 'output1'.
    
    GoldSim Configuration:
    - Inputs: 1 (scalar)
    - Outputs: 1 (scalar)
    """
    input_val = inputs.get('input1', 0.0)
    
    # Perform a simple calculation
    result = input_val * 10
    
    return {'output1': result}

# Test the function if run directly
if __name__ == "__main__":
    test_inputs = {'input1': 5.0}
    result = goldsim_calculate(test_inputs)
    print(f"Test: goldsim_calculate({test_inputs}) = {result}")
    print(f"Expected: input1 * 10 = {test_inputs['input1']} * 10 = {result['output1']}")
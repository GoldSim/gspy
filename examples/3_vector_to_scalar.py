# GSPy Example 3: Vector to Scalar
# This script takes a vector (a list of numbers) as input and
# returns a single scalar value, such as the sum of the vector's elements.

def gspy_info():
    """
    Declares the I/O interface for this script.
    This function is required for dynamic I/O detection.
    """
    return {'inputs': 4, 'outputs': 1}  # 4-element vector input, 1 scalar output

def goldsim_calculate(inputs):
    """
    Takes a vector input from GoldSim and returns a single scalar output.
    The C++ DLL provides all inputs in a flat dictionary, so this script
    reconstructs the vector from the 'input1', 'input2', ... keys.
    
    GoldSim Configuration:
    - Inputs: N (multiple scalars representing vector elements)
    - Outputs: 1 (scalar sum)
    """
    # Reconstruct the vector from the inputs dictionary.
    # This loop collects all values for keys starting with 'input'.
    input_vector = []
    i = 1
    while f'input{i}' in inputs:
        input_vector.append(inputs[f'input{i}'])
        i += 1
    
    # Perform a calculation on the vector. For example, calculate the sum.
    # Handle the case of an empty input vector to avoid errors.
    total_sum = sum(input_vector) if input_vector else 0.0
    
    # Return the single scalar result.
    return {'output1': total_sum}

# Test the function if run directly
if __name__ == "__main__":
    # Test with a 4-element vector
    test_inputs = {
        'input1': 10.0,
        'input2': 20.0, 
        'input3': 15.0,
        'input4': 5.0
    }
    result = goldsim_calculate(test_inputs)
    print(f"Test: goldsim_calculate({test_inputs}) = {result}")
    print(f"Input vector: {[test_inputs[f'input{i}'] for i in range(1, 5)]}")
    print(f"Sum: {result['output1']}")
    print("Expected: 10 + 20 + 15 + 5 = 50")
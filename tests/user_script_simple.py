def goldsim_calculate(inputs):
    """
    Simple test function for GoldSim integration.
    Takes a single scalar input and returns a scalar output.
    """
    # Get the input value
    value = inputs.get('value', 0.0)
    
    # Simple calculation: multiply by 2 and add 1
    result = value * 2.0 + 1.0
    
    # Return as dict with 'result' key
    return {'result': result}
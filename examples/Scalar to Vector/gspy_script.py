# GSPy Example 2: Scalar to Vector
# This script takes a single numerical input (e.g., a flow rate) and
# returns a vector (a list of numbers) with three elements
# (e.g., concentrations for 3 different species).

def gspy_info():
    """
    Declares the I/O interface for this script.
    This function is required for dynamic I/O detection.
    """
    return {'inputs': 1, 'outputs': 3}  # 1 input, vector output counts as 3 outputs

def goldsim_calculate(inputs):
    """
    Takes one input named 'input1' and returns one vector output
    named 'output1'.
    
    GoldSim Configuration:
    - Inputs: 1 (scalar)
    - Outputs: 1 (vector of size 3)
    """
    flow_rate = inputs.get('input1', 0.0)
    
    # Calculate concentrations based on the flow rate
    concentration_A = 150.0 / flow_rate if flow_rate > 0 else 0
    concentration_B = 220.5 / flow_rate if flow_rate > 0 else 0
    concentration_C = 95.0 / flow_rate if flow_rate > 0 else 0
    
    concentrations = [concentration_A, concentration_B, concentration_C]
    
    # The entire list is returned as a single output. GoldSim must be
    # configured to expect a vector output of size 3 for this output.
    return {'output1': concentrations}
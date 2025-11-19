"""
GSPy Error Handling Demonstration

This script demonstrates both error handling approaches:
1. Graceful degradation (log only)
2. Fatal error signaling (stops GoldSim)

Input: A scalar value
Output: A scalar result

Behavior based on input value:
- input > 0: Normal calculation (returns input * 2)
- input = 0: Graceful degradation (logs warning, returns 0)
- input < 0: Fatal error (signals to GoldSim, stops simulation)
"""

import gspy
import traceback

def process_data(*args):
    try:
        input_value = args[0]
        
        # Case 1: Normal operation
        if input_value > 0:
            result = input_value * 2
            gspy.log(f"Normal calculation: {input_value} * 2 = {result}", 2)
            return (result,)
        
        # Case 2: Graceful degradation - log warning but continue
        elif input_value == 0:
            gspy.log("Warning: Input is zero. Returning default value.", 1)
            return (0.0,)
        
        # Case 3: Fatal error - signal to GoldSim
        else:
            raise ValueError(f"Input value must be non-negative, got {input_value}")
    
    except ValueError as e:
        # Expected error - signal to GoldSim
        err_msg = traceback.format_exc()
        gspy.log(f"Validation error: {err_msg}", 0)
        gspy.error(f"Invalid input: {str(e)}")
        return (0.0,)
    
    except Exception as e:
        # Unexpected error - signal to GoldSim
        err_msg = traceback.format_exc()
        gspy.log(f"Unexpected error: {err_msg}", 0)
        gspy.error(f"Critical failure: {str(e)}")
        return (0.0,)

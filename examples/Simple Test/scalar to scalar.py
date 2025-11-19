import traceback
import gspy

def process_data(*args):
  """
  Receives a single scalar value, multiplies it by 2,
  and returns the resulting scalar.
  """
  try:
    # 1. Unpack the input scalar from the arguments tuple
    input_scalar = args[0]

    # Check a condition and write a WARNING message to the log
    if input_scalar < 0:
        # Log Level 1 = WARNING
        gspy.log(f"Input scalar is negative: {input_scalar}. Proceeding with calculation.", 1)
    
    # 2. Perform the calculation
    result_scalar = input_scalar * 2.0
    
    # 3. For debugging, print the values
    print(f"Python received scalar: {input_scalar}, returning scalar: {result_scalar}")
    
    # 4. Return the result as a tuple
    return (result_scalar,)

  except Exception as e:
    # In case of an error, print the details and signal to GoldSim
    err_msg = traceback.format_exc()
    gspy.log(f"Error in process_data: {err_msg}", 0)  # Log Level 0 = ERROR
    
    # Signal a fatal error to GoldSim (will display error dialog and stop simulation)
    gspy.error(f"Fatal error in process_data: {str(e)}")
    
    return (0.0,)  # This will not be used since gspy.error() was called
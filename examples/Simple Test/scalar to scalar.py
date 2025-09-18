import traceback

def process_data(*args):
  """
  Receives a single scalar value, multiplies it by 2,
  and returns the resulting scalar.
  """
  try:
    # 1. Unpack the input scalar from the arguments tuple
    input_scalar = args[0]
    
    # 2. Perform the calculation
    result_scalar = input_scalar * 2.0
    
    # 3. For debugging, print the values
    print(f"Python received scalar: {input_scalar}, returning scalar: {result_scalar}")
    
    # 4. Return the result as a tuple
    return (result_scalar,)

  except Exception as e:
    # In case of an error, print the details and return a dummy output
    print("!!! PYTHON EXCEPTION !!!")
    print(traceback.format_exc())
    return (0.0,)
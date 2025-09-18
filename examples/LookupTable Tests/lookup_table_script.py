import numpy as np
import traceback

def process_data(*args):
  """
  Receives a single scalar and uses it to generate and return
  a 2D Lookup Table as a dictionary.
  """
  try:
    # 1. Unpack the input scalar
    input_scalar = args[0]
    
    # 2. Generate the table components based on the input
    num_rows = 4
    num_cols = 3
    
    # Create row and column labels using the input scalar
    row_labels = np.arange(num_rows) * input_scalar
    col_labels = np.arange(num_cols) + 1
    
    # Create the 2D data array for the table body
    # Here we use an outer product for an interesting result
    data = np.outer(row_labels, col_labels)

    print("--- Python: Generating 2D Lookup Table ---")
    print(f"Data shape: {data.shape}")
    
    # 3. Assemble the dictionary to be returned
    # The C++ code will read these exact keys.
    table_dictionary = {
      "table_dim": 2,
      "row_labels": row_labels,
      "col_labels": col_labels,
      "data": data
    }
    
    # 4. Return the result as a tuple
    return (table_dictionary,)

  except Exception as e:
    print("!!! PYTHON EXCEPTION !!!")
    print(traceback.format_exc())
    
    # Return a dummy 1x1 table in case of an error
    return ({
        "table_dim": 1,
        "row_labels": np.zeros(1),
        "data": np.zeros(1)
    },)
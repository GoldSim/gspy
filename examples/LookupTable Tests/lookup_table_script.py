import numpy as np
import traceback
import gspy

def process_data(*args):
  """
  Receives a single scalar and uses it to generate and return
  1D, 2D, and 3D Lookup Tables as dictionaries.
  """
  try:
    # 1. Unpack the input scalar
    input_scalar = args[0]
    
    gspy.log(f"--- Python: Generating 1D, 2D, and 3D Lookup Tables (input_scalar = {input_scalar}) ---", 2)
    
    # 2. Generate 1D Lookup Table
    num_rows_1d = 5
    
    # Create row labels and data using the input scalar
    row_labels_1d = np.array([0.0, 10.0, 20.0, 30.0, 40.0]) * input_scalar
    data_1d = np.array([1.0, 4.0, 9.0, 16.0, 25.0]) * input_scalar  # Simple quadratic function
    
    gspy.log(f"1D Table - Rows: {num_rows_1d}", 3)
    gspy.log(f"1D Table - Data shape: {data_1d.shape}", 3)
    gspy.log(f"1D Table - Row labels: {row_labels_1d}", 3)
    gspy.log(f"1D Table - Data: {data_1d}", 3)
    
    # Assemble the 1D table dictionary
    table_1d_dictionary = {
      "table_dim": 1,
      "row_labels": row_labels_1d,
      "data": data_1d
    }
    
    # 3. Generate 2D Lookup Table
    num_rows_2d = 4
    num_cols_2d = 3
    
    # Create row and column labels using the input scalar
    row_labels_2d = np.arange(num_rows_2d) * input_scalar
    col_labels_2d = np.arange(num_cols_2d) + 1
    
    # Create the 2D data array for the table body
    data_2d = np.outer(row_labels_2d, col_labels_2d)

    gspy.log(f"2D Table - Rows: {num_rows_2d}, Cols: {num_cols_2d}", 3)
    gspy.log(f"2D Table - Data shape: {data_2d.shape}", 3)
    gspy.log(f"2D Table - Row labels: {row_labels_2d}", 3)
    gspy.log(f"2D Table - Col labels: {col_labels_2d}", 3)
    
    # Assemble the 2D table dictionary
    table_2d_dictionary = {
      "table_dim": 2,
      "row_labels": row_labels_2d,
      "col_labels": col_labels_2d,
      "data": data_2d
    }
    
    # 4. Generate 3D Lookup Table
    num_rows_3d = 3
    num_cols_3d = 2
    num_layers_3d = 4
    
    # Create row, column, and layer labels
    row_labels_3d = np.array([10.0, 20.0, 30.0]) * input_scalar
    col_labels_3d = np.array([1.0, 2.0]) + input_scalar
    layer_labels_3d = np.array([100.0, 200.0, 300.0, 400.0])
    
    # Create the 3D data array
    # Shape should be (num_rows, num_cols, num_layers)
    data_3d = np.zeros((num_rows_3d, num_cols_3d, num_layers_3d))
    
    # Fill with interesting data based on input_scalar
    for r in range(num_rows_3d):
        for c in range(num_cols_3d):
            for l in range(num_layers_3d):
                # Create a function that depends on all three dimensions
                data_3d[r, c, l] = (row_labels_3d[r] + col_labels_3d[c] * layer_labels_3d[l]) * input_scalar

    gspy.log(f"3D Table - Rows: {num_rows_3d}, Cols: {num_cols_3d}, Layers: {num_layers_3d}", 3)
    gspy.log(f"3D Table - Data shape: {data_3d.shape}", 3)
    gspy.log(f"3D Table - Row labels: {row_labels_3d}", 3)
    gspy.log(f"3D Table - Col labels: {col_labels_3d}", 3)
    gspy.log(f"3D Table - Layer labels: {layer_labels_3d}", 3)
    
    # Assemble the 3D table dictionary
    table_3d_dictionary = {
      "table_dim": 3,
      "row_labels": row_labels_3d,
      "col_labels": col_labels_3d,
      "layer_labels": layer_labels_3d,  # Required for 3D tables
      "data": data_3d
    }
    
    gspy.log("--- All lookup tables generated successfully ---", 2)
    
    # 5. Return all three tables as a tuple in the correct order (1D, 2D, 3D)
    return (table_1d_dictionary, table_2d_dictionary, table_3d_dictionary)

  except Exception as e:
    gspy.log("!!! PYTHON EXCEPTION !!!", 0)
    gspy.log(traceback.format_exc(), 0)
    
    # Return dummy tables in case of an error
    dummy_1d = {
        "table_dim": 1,
        "row_labels": np.array([0.0, 1.0]),
        "data": np.array([1.0, 2.0])
    }
    
    dummy_2d = {
        "table_dim": 2,
        "row_labels": np.array([0.0, 1.0]),
        "col_labels": np.array([0.0, 1.0]),
        "data": np.ones((2, 2))
    }
    
    dummy_3d = {
        "table_dim": 3,
        "row_labels": np.array([0.0]),
        "col_labels": np.array([0.0]),
        "layer_labels": np.array([0.0]),
        "data": np.ones((1, 1, 1))
    }
    
    return (dummy_1d, dummy_2d, dummy_3d)
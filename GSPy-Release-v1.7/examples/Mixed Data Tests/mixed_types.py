import numpy as np
import traceback

def process_data(*args):
  """
  Receives 7 inputs and returns 7 outputs of mixed types.
  """
  try:
    # 1. Unpack all 7 input arguments
    input_vector = args[0]
    scalar_1 = args[1]
    scalar_2 = args[2]
    input_matrix = args[3]
    scalar_3 = args[4]
    ts_dict_1 = args[5]
    ts_dict_2 = args[6]

    # 2. Perform original calculations
    v1 = np.arange(4) + scalar_1
    v2 = np.array([np.mean(input_vector), np.sum(input_matrix)])
    output_matrix = np.outer(v1, v2)
    output_scalar_1 = np.std(ts_dict_1['data']) + scalar_1
    output_vector_1 = input_matrix.flatten() * scalar_2
    data_2 = ts_dict_2['data']
    output_vector_2 = np.array([np.min(data_2), np.max(data_2), np.mean(data_2)])
    output_scalar_2 = np.dot(input_vector[:3], output_vector_2) + scalar_3

    # 3. Generate NEW outputs (Time Series and Lookup Table)
    # Create a new Time Series by adding the data from the two input series
    output_timeseries = {
        "timestamps": ts_dict_1["timestamps"],
        "data": ts_dict_1["data"] + ts_dict_2["data"],
        "data_type": 0,
        "time_basis": 0.0
    }

    # Create a new 2D Lookup Table
    output_table = {
      "table_dim": 2,
      "row_labels": np.array([1, 2, 3]) + scalar_1,
      "col_labels": np.array([10, 20]) + scalar_2,
      "data": np.array([[1, 2], [3, 4], [5, 6]]) * scalar_3
    }

    # 4. Return all 7 results as a tuple in the new, specified order
    return (
        output_matrix, 
        output_timeseries, 
        output_scalar_1, 
        output_table,
        output_vector_1, 
        output_vector_2, 
        output_scalar_2
    )

  except Exception as e:
    print("!!! PYTHON EXCEPTION !!!")
    print(traceback.format_exc())
    
    # In case of error, return a tuple of the correct size/shape with dummy values
    dummy_matrix = np.zeros((4, 2))
    dummy_ts = {"timestamps": np.array([0]), "data": np.zeros(1), "data_type": 0, "time_basis": 0}
    dummy_scalar_1 = 0.0
    dummy_table = {"table_dim": 1, "row_labels": np.zeros(1), "data": np.zeros(1)}
    dummy_vector_1 = np.zeros(6)
    dummy_vector_2 = np.zeros(3)
    dummy_scalar_2 = 0.0
    return (
        dummy_matrix,
        dummy_ts,
        dummy_scalar_1,
        dummy_table,
        dummy_vector_1,
        dummy_vector_2,
        dummy_scalar_2
    )
import numpy as np
import traceback

def process_data(*args):
  """
  Receives a scalar time series and a vector time series.
  
  Returns a vector time series and a new scalar time series.
  """
  try:
    # 1. Unpack the two input Time Series dictionaries
    input_ts_scalar_dict = args[0]
    input_ts_vector_dict = args[1]
    
    # 2. Extract the data and timestamps from each
    scalar_timestamps = input_ts_scalar_dict["timestamps"]
    scalar_data = input_ts_scalar_dict["data"]
    
    vector_timestamps = input_ts_vector_dict["timestamps"]
    vector_data = input_ts_vector_dict["data"] # This will have shape (2, num_points)

    print("--- Python Script Received ---")
    print(f"Scalar TS data shape: {scalar_data.shape}")
    print(f"Vector TS data shape: {vector_data.shape}")
    
    # 3. Prepare the outputs
    
    # Output 1: Pass the input vector time series straight through
    output_ts_vector = {
        "timestamps": vector_timestamps,
        "data": vector_data,
        "data_type": 0,
        "time_basis": 0.0
    }
    
    # Output 2: Create a new scalar time series by combining the inputs
    # (e.g., add the scalar data to the first row of the vector data)
    # Note: Assumes both time series have the same timestamps for this example
    new_scalar_data = vector_data[0, :] + scalar_data
    
    output_ts_scalar = {
        "timestamps": vector_timestamps,
        "data": new_scalar_data,
        "data_type": 0,
        "time_basis": 0.0
    }

    # 4. Return the results as a tuple in the correct order
    return (output_ts_vector, output_ts_scalar)

  except Exception as e:
    print("!!! PYTHON EXCEPTION !!!")
    print(traceback.format_exc())
    
    # In case of error, return a tuple of the correct size/shape with dummy values
    dummy_ts_vector = {"timestamps": np.array([0]), "data": np.zeros((2,1)), "data_type": 0, "time_basis": 0}
    dummy_ts_scalar = {"timestamps": np.array([0]), "data": np.zeros(1), "data_type": 0, "time_basis": 0}
    return (dummy_ts_vector, dummy_ts_scalar)
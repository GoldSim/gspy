import numpy as np
import traceback
import gspy

def process_data(*args):
  """
  Receives a scalar time series, a vector time series, and a matrix time series.
  
  Returns a vector time series (2 items), scalar time series, and matrix time series (2x3).
  """
  try:
    # 1. Unpack the three input Time Series dictionaries
    input_ts_scalar_dict = args[0]
    input_ts_vector_dict = args[1]
    input_ts_matrix_dict = args[2]
    
    # 2. Extract the data and timestamps from each
    scalar_timestamps = input_ts_scalar_dict["timestamps"]
    scalar_data = input_ts_scalar_dict["data"]  # Shape: (3,) - values [1, 10, 5]
    
    vector_timestamps = input_ts_vector_dict["timestamps"]
    vector_data = input_ts_vector_dict["data"]  # Shape: (2, 3) - 2 rows, 3 time points
    
    matrix_timestamps = input_ts_matrix_dict["timestamps"]
    matrix_data = input_ts_matrix_dict["data"]  # Shape: (3, 2, 6) - 3 rows, 2 cols, 6 time points

    gspy.log("--- Python Script Received ---", 2)
    gspy.log(f"Scalar TS data shape: {scalar_data.shape}, values: {scalar_data}", 3)
    gspy.log(f"Scalar TS timestamps: {scalar_timestamps}", 3)
    gspy.log(f"Scalar TS time_basis: {input_ts_scalar_dict['time_basis']} (0=elapsed, 1=calendar)", 3)
    gspy.log(f"Vector TS data shape: {vector_data.shape}", 3)
    gspy.log(f"Vector TS timestamps: {vector_timestamps}", 3)
    gspy.log(f"Vector TS time_basis: {input_ts_vector_dict['time_basis']} (0=elapsed, 1=calendar)", 3)
    gspy.log(f"Matrix TS data shape: {matrix_data.shape}", 3)
    gspy.log(f"Matrix TS timestamps: {matrix_timestamps}", 3)
    gspy.log(f"Matrix TS time_basis: {input_ts_matrix_dict['time_basis']} (0=elapsed, 1=calendar)", 3)
    
    # 3. Prepare the outputs
    
    # Output 1: Vector time series (2 items) - Scale the input vector by 2
    output_vector_data = vector_data * 2.0  # Simple scaling transformation
    output_ts_vector = {
        "timestamps": vector_timestamps,
        "data": output_vector_data,
        "data_type": input_ts_vector_dict["data_type"],  # Preserve input data_type
        "time_basis": input_ts_vector_dict["time_basis"]  # Preserve input time_basis
    }
    
    # Output 2: Scalar time series - Sum the scalar input with the mean of the first vector row
    vector_row1_mean = np.mean(vector_data[0, :])  # Mean of first row of vector
    output_scalar_data = scalar_data + vector_row1_mean  # Add mean to each scalar value
    output_ts_scalar = {
        "timestamps": scalar_timestamps,
        "data": output_scalar_data,
        "data_type": input_ts_scalar_dict["data_type"],  # Preserve input data_type
        "time_basis": input_ts_scalar_dict["time_basis"]  # Preserve input time_basis
    }
    
    # Output 3: Matrix time series (2x3) - Transpose and take subset of input matrix
    # Take first 2 rows and transpose to get 2x3 matrix, use first 3 time points
    matrix_subset = matrix_data[:2, :, :3]  # Take first 2 rows, all columns, first 3 time points
    processed_matrix_data = np.transpose(matrix_subset, (1, 0, 2))  # Shape becomes (2, 2, 3)
    
    # Extend to 2x3 by duplicating the second row
    extended_matrix = np.zeros((2, 3, 3))
    extended_matrix[:, :2, :] = processed_matrix_data  # Fill first 2 columns
    extended_matrix[:, 2, :] = processed_matrix_data[:, 1, :] * 1.5  # Third column = 1.5 * second column
    
    output_ts_matrix = {
        "timestamps": scalar_timestamps,  # Use scalar timestamps (3 points) for output matrix
        "data": extended_matrix,
        "data_type": input_ts_scalar_dict["data_type"],  # Preserve input data_type
        "time_basis": input_ts_scalar_dict["time_basis"]  # Preserve input time_basis
    }

    gspy.log("--- Python Script Outputs ---", 2)
    gspy.log(f"Output Vector shape: {output_vector_data.shape}", 3)
    gspy.log(f"Output Scalar shape: {output_scalar_data.shape}", 3)
    gspy.log(f"Output Matrix shape: {extended_matrix.shape}", 3)

    # 4. Return the results as a tuple in the correct order
    return (output_ts_vector, output_ts_scalar, output_ts_matrix)

  except Exception as e:
    gspy.log("!!! PYTHON EXCEPTION !!!", 0)
    gspy.log(traceback.format_exc(), 0)
    
    # In case of error, return a tuple of the correct size/shape with dummy values
    # Try to preserve time_basis from inputs if available, otherwise default to elapsed time
    try:
        time_basis = args[0]["time_basis"] if len(args) > 0 else 0.0
        data_type = args[0]["data_type"] if len(args) > 0 else 0.0
        # Use appropriate dummy timestamps based on time_basis
        if time_basis == 1.0:  # Calendar dates
            dummy_timestamps = np.array([45543.0, 45544.0, 45545.0])  # Approx Sept 12-14, 2025
        else:  # Elapsed time
            dummy_timestamps = np.array([0, 1, 2])
    except:
        time_basis = 0.0
        data_type = 0.0
        dummy_timestamps = np.array([0, 1, 2])
    
    dummy_ts_vector = {"timestamps": dummy_timestamps, "data": np.zeros((2, 3)), "data_type": data_type, "time_basis": time_basis}
    dummy_ts_scalar = {"timestamps": dummy_timestamps, "data": np.zeros(3), "data_type": data_type, "time_basis": time_basis}
    dummy_ts_matrix = {"timestamps": dummy_timestamps, "data": np.zeros((2, 3, 3)), "data_type": data_type, "time_basis": time_basis}
    return (dummy_ts_vector, dummy_ts_scalar, dummy_ts_matrix)
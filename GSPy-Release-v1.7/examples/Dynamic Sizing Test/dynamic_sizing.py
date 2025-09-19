import numpy as np
import traceback

def process_data(*args):
    """
    Test function for dynamic sizing functionality.
    Expected inputs:
    0. vector_size (scalar) - controls size of dynamic_vector
    1. matrix_rows (scalar) - controls rows of dynamic_matrix  
    2. matrix_cols (scalar) - controls cols of dynamic_matrix
    3. static_scalar (scalar) - regular scalar input
    4. static_vector (vector[3]) - regular static vector
    5. dynamic_vector (vector with size from vector_size)
    6. dynamic_matrix (matrix with size from matrix_rows x matrix_cols)
    """
    try:
        print(f"\n=== Dynamic Sizing Test ===")
        print(f"Received {len(args)} arguments:")
        
        # Extract and validate inputs
        vector_size = args[0]
        matrix_rows = args[1] 
        matrix_cols = args[2]
        static_scalar = args[3]
        static_vector = args[4]
        dynamic_vector = args[5]
        dynamic_matrix = args[6]
        
        print(f"  vector_size = {vector_size}")
        print(f"  matrix_rows = {matrix_rows}")
        print(f"  matrix_cols = {matrix_cols}")
        print(f"  static_scalar = {static_scalar}")
        print(f"  static_vector = {static_vector} (shape: {static_vector.shape})")
        print(f"  dynamic_vector = {dynamic_vector} (shape: {dynamic_vector.shape})")
        print(f"  dynamic_matrix = {dynamic_matrix} (shape: {dynamic_matrix.shape})")
        
        # Validate dynamic dimensions
        expected_vector_size = int(vector_size)
        expected_matrix_shape = (int(matrix_rows), int(matrix_cols))
        
        print(f"\nValidation:")
        print(f"  Expected dynamic_vector size: {expected_vector_size}, actual: {dynamic_vector.shape}")
        print(f"  Expected dynamic_matrix shape: {expected_matrix_shape}, actual: {dynamic_matrix.shape}")
        
        if dynamic_vector.shape == (expected_vector_size,):
            print("  ✓ Dynamic vector size is correct!")
        else:
            print("  ✗ Dynamic vector size mismatch!")
            
        if dynamic_matrix.shape == expected_matrix_shape:
            print("  ✓ Dynamic matrix shape is correct!")
        else:
            print("  ✗ Dynamic matrix shape mismatch!")
        
        # Create outputs
        result_scalar = np.sum(static_vector) + static_scalar + np.sum(dynamic_vector) + np.sum(dynamic_matrix)
        result_vector = np.array([1.0, 2.0, 3.0, 4.0, 5.0])
        
        print(f"\nOutputs:")
        print(f"  result_scalar = {result_scalar}")
        print(f"  result_vector = {result_vector}")
        
        return (result_scalar, result_vector)
    
    except Exception as e:
        print(f"!!! PYTHON EXCEPTION !!!")
        print(traceback.format_exc())
        return (0.0, np.zeros(5))
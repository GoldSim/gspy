"""
GSPy Example: NumPy Statistical Analysis
========================================

This example demonstrates using NumPy for advanced statistical calculations.
- Input: Vector of measurements (passed as multiple scalar inputs)
- Output: Statistical summary (mean, std, min, max as separate scalars)
- Libraries: Uses NumPy for efficient numerical operations

Use Case: Process measurement data to extract key statistical metrics
"""

import numpy as np


def gspy_info():
    """Define the interface specification."""
    return {
        'inputs': 5,    # 5 measurement values
        'outputs': 4    # mean, std, min, max
    }


def goldsim_calculate(inputs):
    """
    Calculate statistical summary using NumPy.
    
    Args:
        inputs: Dictionary with keys 'input1', 'input2', ..., 'input5'
    
    Returns:
        Dictionary with statistical results
    """
    try:
        # Extract measurement data
        measurements = [
            inputs.get('input1', 0.0),
            inputs.get('input2', 0.0),
            inputs.get('input3', 0.0),
            inputs.get('input4', 0.0),
            inputs.get('input5', 0.0)
        ]
        
        # Convert to NumPy array for efficient processing
        data = np.array(measurements)
        
        # Calculate statistics using NumPy functions
        mean_val = np.mean(data)
        std_val = np.std(data, ddof=1)  # Sample standard deviation
        min_val = np.min(data)
        max_val = np.max(data)
        
        # Additional NumPy operations to demonstrate capabilities
        normalized_data = (data - mean_val) / std_val if std_val > 0 else data
        outlier_threshold = 2.0  # Z-score threshold
        outlier_count = np.sum(np.abs(normalized_data) > outlier_threshold)
        
        print(f"NumPy Analysis Results:")
        print(f"  Original data: {data}")
        print(f"  Mean: {mean_val:.3f}")
        print(f"  Std Dev: {std_val:.3f}")
        print(f"  Range: [{min_val:.3f}, {max_val:.3f}]")
        print(f"  Outliers (|z| > {outlier_threshold}): {outlier_count}")
        
        return {
            'output1': float(mean_val),    # Mean
            'output2': float(std_val),     # Standard deviation
            'output3': float(min_val),     # Minimum
            'output4': float(max_val)      # Maximum
        }
        
    except Exception as e:
        print(f"Error in NumPy calculation: {e}")
        # Return safe defaults
        return {
            'output1': 0.0,
            'output2': 0.0,
            'output3': 0.0,
            'output4': 0.0
        }


# Test the function when run directly
if __name__ == "__main__":
    print("Testing NumPy Statistics Example")
    print("=" * 40)
    
    # Test with sample data
    test_inputs = {
        'input1': 10.5,
        'input2': 12.3,
        'input3': 9.8,
        'input4': 15.2,
        'input5': 11.1
    }
    
    # Check NumPy availability
    try:
        import numpy as np
        print(f"✅ NumPy version: {np.__version__}")
    except ImportError:
        print("❌ NumPy not available!")
        exit(1)
    
    # Run the calculation
    info = gspy_info()
    print(f"\nInterface: {info['inputs']} inputs → {info['outputs']} outputs")
    
    result = goldsim_calculate(test_inputs)
    
    print(f"\nResults:")
    print(f"  Mean (output1): {result['output1']:.3f}")
    print(f"  Std Dev (output2): {result['output2']:.3f}")
    print(f"  Minimum (output3): {result['output3']:.3f}")
    print(f"  Maximum (output4): {result['output4']:.3f}")
    
    # Verify with manual calculation
    manual_mean = sum(test_inputs.values()) / len(test_inputs)
    print(f"\nVerification:")
    print(f"  Manual mean: {manual_mean:.3f}")
    print(f"  NumPy mean: {result['output1']:.3f}")
    print(f"  Match: {'✅' if abs(manual_mean - result['output1']) < 1e-10 else '❌'}")

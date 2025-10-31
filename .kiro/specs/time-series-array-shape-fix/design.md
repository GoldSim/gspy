# Design Document

## Overview

This design addresses a critical array shape inconsistency bug in the TimeSeriesManager's marshaling functions. The issue occurs in the `MarshalGoldSimTimeSeriesToPython` function where the dimension ordering logic creates NumPy arrays with incorrect shape compared to what the `MarshalPythonTimeSeriesToGoldSim` function expects.

**Current Problem:**
- `MarshalGoldSimTimeSeriesToPython` creates arrays with shape (num_cols, num_rows, num_time_points) for matrix time series
- `MarshalPythonTimeSeriesToGoldSim` expects arrays with shape (num_rows, num_cols, num_time_points) for matrix time series
- This mismatch causes data corruption when round-tripping matrix time series data

**Solution:**
Fix the dimension insertion order in `MarshalGoldSimTimeSeriesToPython` to match the expected shape in `MarshalPythonTimeSeriesToGoldSim`.

## Architecture

The fix involves a single, targeted change to the TimeSeriesManager.cpp file without affecting the overall architecture. The marshaling functions will maintain their current interfaces and responsibilities, but with corrected internal logic.

### Current Architecture (Unchanged)
```
GoldSim ←→ TimeSeriesManager ←→ Python
         ↑                    ↑
    MarshalPython...     MarshalGoldSim...
    TimeSeriesToGoldSim  TimeSeriesToPython
```

### Data Flow
1. **GoldSim → Python**: `MarshalGoldSimTimeSeriesToPython` converts GoldSim time series to Python dictionary with NumPy arrays
2. **Python → GoldSim**: `MarshalPythonTimeSeriesToGoldSim` converts Python dictionary back to GoldSim format

## Components and Interfaces

### Affected Component: MarshalGoldSimTimeSeriesToPython

**Current Buggy Logic:**
```cpp
// This creates wrong dimension order
if (num_rows > 0) {
    data_dims_vec.insert(data_dims_vec.begin(), num_rows);
    data_size *= num_rows;
}
if (num_cols > 0) {
    data_dims_vec.insert(data_dims_vec.begin(), num_cols);  // Inserted BEFORE num_rows
    data_size *= num_cols;
}
```

**Fixed Logic:**
```cpp
// Insert num_cols FIRST (if present)
if (num_cols > 0) {
    data_dims_vec.insert(data_dims_vec.begin(), num_cols);
    data_size *= num_cols;
}
// Insert num_rows SECOND (if present) - this will be at position 0, pushing num_cols to position 1
if (num_rows > 0) {
    data_dims_vec.insert(data_dims_vec.begin(), num_rows);
    data_size *= num_rows;
}
```

### Unchanged Component: MarshalPythonTimeSeriesToGoldSim

This function correctly expects and handles the following shapes:
- Scalar: 1D array (num_time_points)
- Vector: 2D array (num_rows, num_time_points) 
- Matrix: 3D array (num_rows, num_cols, num_time_points)

The logic in this function remains correct and unchanged.

## Data Models

### Expected Array Shapes (After Fix)

| Time Series Type | Array Dimensions | Shape |
|------------------|------------------|-------|
| Scalar | 1D | (num_time_points) |
| Vector | 2D | (num_rows, num_time_points) |
| Matrix | 3D | (num_rows, num_cols, num_time_points) |

### Dimension Vector Construction Logic

The `data_dims_vec` will be built in the correct order:

1. **Start with**: `[num_time_points]`
2. **If num_cols > 0**: Insert at beginning → `[num_cols, num_time_points]`
3. **If num_rows > 0**: Insert at beginning → `[num_rows, num_cols, num_time_points]`

This ensures the final shape matches what `MarshalPythonTimeSeriesToGoldSim` expects.

## Error Handling

### No New Error Conditions
The fix does not introduce new error conditions since:
- The same validation logic remains in place
- The same data types and ranges are handled
- Only the order of dimension insertion changes

### Existing Error Handling Preserved
- NumPy API initialization errors
- Invalid dictionary structure errors
- Missing required keys in Python dictionary
- Array size and type validation errors

## Testing Strategy

### Unit Tests Required
1. **Scalar Time Series Test**: Verify 1D array shape remains unchanged
2. **Vector Time Series Test**: Verify 2D array shape remains unchanged  
3. **Matrix Time Series Test**: Verify 3D array shape is corrected to (num_rows, num_cols, num_time_points)
4. **Round-trip Test**: Verify matrix data maintains integrity when marshaled GoldSim→Python→GoldSim

### Test Data Scenarios
- **Scalar**: Single time series with only time points
- **Vector**: Time series with multiple rows, single column
- **Matrix**: Time series with multiple rows and columns
- **Edge Cases**: Zero rows/columns, single row/column matrices

### Validation Approach
1. Create test time series data in GoldSim format
2. Marshal to Python and verify NumPy array shapes
3. Marshal back to GoldSim and verify data integrity
4. Compare original and round-trip data for exact matches

### Regression Testing
- Verify existing scalar and vector time series functionality remains unchanged
- Confirm no performance impact from the dimension ordering change
- Validate that existing Python scripts continue to work with correctly-shaped arrays
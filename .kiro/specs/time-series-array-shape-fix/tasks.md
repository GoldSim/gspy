# Implementation Plan

- [x] 1. Fix array dimension ordering in MarshalGoldSimTimeSeriesToPython












  - Locate the buggy dimension insertion logic in TimeSeriesManager.cpp
  - Swap the order of num_rows and num_cols insertion blocks
  - Ensure num_cols is inserted before num_rows to achieve correct final shape
  - Verify the fix produces arrays with shape (num_rows, num_cols, num_time_points) for matrix time series
  - _Requirements: 1.1, 1.2, 2.1, 2.4_

- [ ]* 1.1 Create unit tests for array shape validation
  - Write test cases for scalar, vector, and matrix time series marshaling
  - Verify correct NumPy array shapes are produced after the fix
  - Test round-trip data integrity (GoldSim→Python→GoldSim)
  - _Requirements: 1.1, 1.2, 1.3, 1.4, 1.5_

- [ ] 2. Validate the fix with existing functionality
  - Compile the modified TimeSeriesManager.cpp
  - Run existing tests to ensure no regression in scalar and vector time series
  - Verify that function signatures and public interfaces remain unchanged
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5_

- [ ]* 2.1 Add comprehensive test coverage for edge cases
  - Test zero rows/columns scenarios
  - Test single row/column matrix scenarios  
  - Validate error handling remains intact
  - _Requirements: 2.2, 2.3, 3.5_
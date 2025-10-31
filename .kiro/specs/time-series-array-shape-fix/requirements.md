# Requirements Document

## Introduction

This specification addresses a critical bug in the GSPy library's time series marshaling functionality where there is an inconsistency in NumPy array shape handling between GoldSim-to-Python and Python-to-GoldSim data conversion functions. The bug causes matrix time series data to have mismatched dimensions when round-tripping between GoldSim and Python, potentially leading to incorrect data interpretation and processing errors.

## Glossary

- **GSPy**: The C++ library that provides Python integration for GoldSim
- **TimeSeriesManager**: The C++ class responsible for managing time series data conversion between GoldSim and Python
- **MarshalGoldSimTimeSeriesToPython**: Function that converts GoldSim time series data to Python NumPy arrays
- **MarshalPythonTimeSeriesToGoldSim**: Function that converts Python NumPy arrays back to GoldSim time series format
- **Matrix Time Series**: Time series data with both row and column dimensions (3D array: rows × columns × time points)
- **Vector Time Series**: Time series data with only row dimension (2D array: rows × time points)
- **Scalar Time Series**: Time series data with no matrix dimensions (1D array: time points)
- **NumPy Array Shape**: The dimensional structure of a NumPy array expressed as (dimension1, dimension2, ...)

## Requirements

### Requirement 1

**User Story:** As a GoldSim user integrating Python scripts, I want matrix time series data to maintain consistent dimensional ordering when passed between GoldSim and Python, so that my data processing logic works correctly in both directions.

#### Acceptance Criteria

1. WHEN a matrix time series is marshaled from GoldSim to Python, THE TimeSeriesManager SHALL create a NumPy array with shape (num_rows, num_cols, num_time_points)
2. WHEN a matrix time series is marshaled from Python to GoldSim, THE TimeSeriesManager SHALL expect a NumPy array with shape (num_rows, num_cols, num_time_points)
3. WHEN a vector time series is marshaled from GoldSim to Python, THE TimeSeriesManager SHALL create a NumPy array with shape (num_rows, num_time_points)
4. WHEN a scalar time series is marshaled from GoldSim to Python, THE TimeSeriesManager SHALL create a NumPy array with shape (num_time_points)
5. THE TimeSeriesManager SHALL maintain identical array shape conventions in both marshaling directions

### Requirement 2

**User Story:** As a developer maintaining the GSPy library, I want the array dimension ordering logic to be consistent and correct, so that future modifications don't introduce similar bugs.

#### Acceptance Criteria

1. THE MarshalGoldSimTimeSeriesToPython function SHALL insert num_cols before num_rows in the data_dims_vec construction
2. THE MarshalPythonTimeSeriesToGoldSim function SHALL continue to expect num_rows as the first dimension and num_cols as the second dimension
3. WHEN both num_rows and num_cols are greater than zero, THE data_dims_vec SHALL be ordered as [num_rows, num_cols, num_time_points]
4. THE dimension insertion logic SHALL produce arrays that match the expected shape in the corresponding unmarshal function
5. THE fix SHALL not affect the handling of scalar or vector time series data

### Requirement 3

**User Story:** As a GoldSim user, I want existing functionality to remain unaffected by the bug fix, so that my current workflows continue to work without modification.

#### Acceptance Criteria

1. THE fix SHALL not modify the function signatures of any public methods
2. THE fix SHALL not change the behavior for scalar time series (1D arrays)
3. THE fix SHALL not change the behavior for vector time series (2D arrays)
4. THE fix SHALL maintain backward compatibility with existing Python scripts that handle correctly-shaped arrays
5. THE TimeSeriesManager SHALL continue to handle all existing time series types without regression
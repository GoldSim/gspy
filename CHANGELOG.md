# GSPy Changelog

## Version 1.0.0 (2025-01-04)

### Added
- **Initial release** of GSPy - GoldSim Python Interface
- **Automatic I/O detection** via `gspy_info()` function
- **Convention-based script loading** from `gspy_script.py`
- **Robust GIL management** to prevent Python API access violations
- **Multiple data type support**:
  - Scalar inputs and outputs
  - Vector inputs and outputs  
  - Time series inputs (automatically unpacked to dictionaries)
- **Comprehensive error handling** and logging
- **Working examples** for common use cases:
  - Scalar to scalar transformations
  - Scalar to vector calculations
  - Vector to scalar analysis
  - 1D lookup tables and interpolation
  - Time series to scalar analysis

### Technical Improvements
- **Fixed DataMarshaller.cpp**: Removed extra vector size field for scalar time series
- **Modern C++ implementation** with proper memory management
- **Automatic script discovery** and module loading
- **Full GoldSim External Function protocol** implementation

### Known Limitations
- **Time series outputs**: Not supported due to format compatibility issues
- **Python version**: Tested with Python 3.13, other versions may require recompilation

### Examples Included
- `1_scalar_to_scalar.py` - Basic mathematical transformations
- `2_scalar_to_vector.py` - Multi-component analysis (e.g., concentrations)
- `3_vector_to_scalar.py` - Statistical analysis of arrays
- `4_lookup_table_1d.py` - Property lookups and interpolation
- `5_timeseries_to_scalar.py` - Time series statistical analysis

### Documentation
- Comprehensive README with quick start guide
- Detailed examples documentation
- Troubleshooting guide
- Architecture and technical details
- Building from source instructions

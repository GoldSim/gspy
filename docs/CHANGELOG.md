# Changelog

All notable changes to this project will be documented in this file.

## [1.8.9] - 2026-01-22

### Added
- **Third-Party License Compliance:** Added comprehensive `THIRD_PARTY_LICENSES.md` file documenting all third-party dependencies
  * **Python Software Foundation License** - Full PSF license text for Python C API
  * **NumPy BSD 3-Clause License** - Complete BSD license text with copyright notices
  * **nlohmann/json MIT License** - MIT license text for JSON parsing library
  * Ensures full compliance with all open-source license requirements
  * Provides clear attribution to all third-party library authors
  * Documents runtime dependencies and their respective licenses

### Changed
- **License Documentation:** Enhanced legal compliance for public distribution
  * All required copyright notices now properly included
  * License texts provided in full as required by BSD 3-Clause and other licenses
  * Clear separation between GSPy's MIT license and third-party licenses

### Technical Details
- Added `docs/THIRD_PARTY_LICENSES.md` with complete license texts for Python, NumPy, and nlohmann/json
- Ensures compliance with permissive license requirements (attribution and notice retention)
- No functional changes to code - purely documentation enhancement for legal compliance

## [1.8.8] - 2025-11-18

### Added
- **Fatal Error Signaling:** New `gspy.error()` function allows Python code to signal critical errors and stop GoldSim simulations
  * **Function:** `gspy.error(message)` - Signals a fatal error and terminates the simulation
  * **Use Case:** Critical errors where results would be invalid (division by zero, missing required data, invalid configuration)
  * **Behavior:** Raises Python RuntimeError, C++ catches it, logs detailed error, sets status = 1, simulation stops
  * **Error Details:** Full error message and Python traceback written to log file
  * **GoldSim Display:** Shows "Error in external function. Return code 1." (generic message due to process separation)
  * **Architecture Note:** 32-bit GoldSim / 64-bit DLL run in separate processes, preventing direct error message passing

### Enhanced
- **Error Handling Documentation:** Comprehensive documentation for both error handling approaches
  * `docs/Using_gspy_error.md` - Quick start guide with examples
  * `docs/Error_Handling_Best_Practices.md` - Complete guide with use cases and patterns
  * `docs/Error_Handling_Quick_Reference.md` - Quick reference card
  * `docs/Slide_Updates_Error_Handling.md` - Updated presentation content
  * `docs/CHANGELOG_Error_Handling.md` - Detailed version history and migration guide
  * `examples/Error Handling Demo/` - Working demonstration with test cases

### Improved
- **Exception Handling:** Added comprehensive try-catch blocks in main GSPy function
  * Catches and logs C++ exceptions with detailed error messages
  * Prevents crashes and provides better error diagnostics
  * Ensures clean error reporting to GoldSim

- **Python Exception Detection:** Enhanced ExecuteCalculation to properly handle Python exceptions
  * Detects when Python raises exceptions (including from gspy.error())
  * Retrieves stored error messages for detailed logging
  * Provides fallback error handling for unexpected exceptions

### Technical Details
- **Implementation:** 
  * Added `PythonError()` function that raises Python RuntimeError
  * Added global error message storage (`g_python_error_message`)
  * Modified `ExecuteCalculation()` to detect Python exceptions and retrieve stored messages
  * Updated `SendErrorToGoldSim()` to use status = 1 (compatible with separate process architecture)
  * Added exception handling wrapper in main `GSPy()` function
  * Added cleanup in `FinalizePython()` for error message pointer

- **Error Handling Flow:**
  1. Python calls `gspy.error(message)`
  2. Message stored in C++ and Python RuntimeError raised
  3. C++ detects NULL return from Python
  4. C++ retrieves stored message and logs it
  5. C++ sets status = 1 (failure)
  6. GoldSim detects failure and stops simulation
  7. User checks log file for detailed error information

- **Process Architecture:** Solution accounts for 32-bit GoldSim / 64-bit DLL separate process requirement
  * Memory pointers cannot be shared across process boundaries
  * Error messages written to log file instead of passed via pointers
  * This is the correct approach for the 32-bit/64-bit architecture

### Comparison: Error Handling Approaches

| Approach | Function | Simulation | GoldSim Message | Details Location |
|----------|----------|------------|-----------------|------------------|
| **Graceful Degradation** | `gspy.log()` only | Continues | None | Log file |
| **Fatal Error** | `gspy.error()` | **Stops** | "Return code 1" | Log file |

### Migration Guide
- **No breaking changes** - Existing code continues to work unchanged
- **Optional enhancement** - Add `gspy.error()` for critical error handling
- **Recommended pattern:**
  ```python
  try:
      result = calculate(args)
      return (result,)
  except ValueError as e:
      # Recoverable - use graceful degradation
      gspy.log(f"Warning: {e}", 1)
      return (default_value,)
  except Exception as e:
      # Critical - stop simulation
      gspy.log(traceback.format_exc(), 0)
      gspy.error(f"Fatal: {e}")
      return (0.0,)
  ```

## [1.8.7] - 2025-11-15

### Fixed
- **3D Lookup Table Data Ordering Bug:** Fixed critical data mapping issue in `LookupTableManager.cpp`
  * **Issue:** 3D lookup table values were incorrectly mapped when passed from Python to GoldSim due to array ordering mismatch
  * **Root Cause:** NumPy stores 3D arrays with shape `(rows, cols, layers)` in C-order (row-major), while GoldSim expects layer-major ordering
  * **Solution:** Replaced direct `memcpy` with explicit reordering loops to properly convert from NumPy's (row, col, layer) format to GoldSim's layer-major format
  * **Impact:** All 3D lookup table values now correctly correspond to their intended row, column, and layer indices
  * **Verification:** Confirmed correct data mapping with test cases showing proper value alignment across all dimensions
  * **Note:** 1D and 2D lookup tables were unaffected as their data layouts naturally aligned with GoldSim's expected format

## [1.8.6] - 2025-11-07

### Changed
- **Simplified Log File Format:** Cleaned up log output with minimal, consistent header
  * Removed build date from header for cleaner appearance
  * Removed diagnostic clutter (config file, Python version, script path) from always-logged output
  * Log now shows only version header followed by timestamped entries based on log level
  * Provides cleaner output especially for production use with log level 0 (ERROR only)

### Improved
- **Documentation Examples:** Updated all Python code examples in README.md to follow best practices
  * Replaced all `print()` statements with `gspy.log()` calls
  * Added appropriate log levels: 0 (ERROR), 2 (INFO), 3 (DEBUG)
  * Ensures examples demonstrate proper logging integration
  * Added `import gspy` to all example scripts
- **README Enhancements:** Improved documentation structure and accessibility
  * Added comprehensive Table of Contents with anchor links to all major sections
  * Added "Downloading GSPy" section with step-by-step GitHub download instructions for new users
  * Improved navigation and discoverability of documentation content

### Technical Details
- Log format now consistently uses timestamped entries for all log levels
- LogAlways() function simplified to remove timestamp formatting
- Removed redundant diagnostic logging that cluttered output at higher log levels
- Version number updated to 1.8.6 in GSPy.h and README.md

## [1.8.3] - 2025-10-30
### Fixed
* **Time Series Array Dimension Bug:** Fixed array dimension ordering inconsistency in `MarshalGoldSimTimeSeriesToPython` function.
  * **Issue:** Matrix time series were created with incorrect shape `(num_cols, num_rows, num_time_points)` instead of expected `(num_rows, num_cols, num_time_points)`
  * **Solution:** Corrected dimension insertion order to ensure consistent array shapes between GoldSim-to-Python and Python-to-GoldSim marshalling
  * **Impact:** Ensures proper data integrity for matrix time series and prevents confusion in complex simulations
  * **Compatibility:** Scalar and vector time series remain unchanged; only matrix time series affected

### Enhanced
* **Complete Lookup Table Support:** Enhanced and verified full 1D, 2D, and 3D lookup table functionality.
  * **1D Tables:** Simple row labels with dependent values for basic interpolation
  * **2D Tables:** Row and column labels with matrix data for surface interpolation  
  * **3D Tables:** Row, column, and layer labels with 3D data arrays for volume interpolation
  * **Verification:** All table types successfully tested and verified to work with GoldSim
  * **Format Compliance:** Implementation follows GoldSim DLL specification for table data marshalling

* **Calendar Date Time Series Support:** 
  * **Automatic Detection:** GSPy automatically handles `time_basis` field (0=elapsed time, 1=calendar dates)
  * **Metadata Preservation:** Fix for `time_basis` and `data_type` values are preserved from inputs to outputs
  * **Error Prevention:** Prevents "First entry in input history data occurs after start of simulation" errors
  * **Julian Day Support:** Proper handling of Julian day numbers for calendar-based simulations

* **Enhanced Example Scripts:** Updated all example Python scripts with logging integration.
  * **GSPy Logging:** Replaced all `print()` statements with `gspy.log()` calls for unified logging
  * **Appropriate Log Levels:** Used ERROR (0), INFO (2), and DEBUG (3) levels for message categorization
  * **Performance Optimization:** Leverages GSPy's atomic log filtering for production performance
  * **Thread Safety:** All logging now uses GSPy's thread-safe logging system

### Updated
* **Documentation:** Comprehensive updates to README.md with verified examples and best practices.
  * **Time Series Section:** Complete documentation of scalar, vector, and matrix time series with correct array shapes
  * **Calendar Date Examples:** Examples showing calendar-based time series handling
  * **Lookup Table Examples:** Complete 1D, 2D, and 3D table generation with working code
  * **Python Version Requirements:** Updated examples to use Python 3.14 for optimal compatibility
  * **Troubleshooting Guide:** Enhanced troubleshooting section with solutions for common time series and table issues

### Technical Details
* **Array Shape Consistency:** Matrix time series now consistently use `(num_rows, num_cols, num_time_points)` shape in both directions
* **GoldSim Format Compliance:** All lookup table implementations verified against GoldSim Help DLL specification
* **Memory Layout Optimization:** NumPy C-order arrays provide correct memory sequence for GoldSim's expected data format
* **Metadata Handling:** Robust preservation of time series metadata prevents configuration mismatches
* **Example Verification:** All provided examples have been tested and verified to work correctly with GoldSim

## [1.8.0] - 2025-10-26
### Added
* **Multi-Python Build System:** Implemented flexible build system supporting multiple Python versions (3.11 and 3.14) through Visual Studio Property Sheets.
  * Created `python_311.props` and `python_314.props` Property Sheets for version-specific configuration
  * Environment variable-based configuration using `PYTHON_3_11_HOME` and `PYTHON_3_14_HOME`
  * Allow for switching between Python versions through Visual Studio Property Manager
  * Clean project file with no hard-coded Python paths for improved maintainability
* **Developer Documentation:** Added developer documentation section to README.md including:
  * Step-by-step environment variable setup guide
  * Python version switching workflows using Property Manager
  * Troubleshooting guide for common build and environment issues
  * Team onboarding instructions for new developers
  * Examples of typical development workflows for single and multi-version development
* **Enhanced Logging System:** Implemented high-performance, configurable logging with atomic-level optimization:
  * Added `log_level` configuration option in JSON files (0=ERROR, 1=WARNING, 2=INFO, 3=DEBUG)
  * Atomic log level filtering with fast-path optimization for production use
  * Thread-safe logging operations with mutex protection
  * Hybrid flush policy: immediate flush for errors/warnings, write-only for info/debug
  * Enhanced Python logging bridge with `gspy.log(message, level)` function
  * Automatic stderr fallback if file operations fail
  * Reentrancy protection for Python logging calls

### Changed
* **Build System Architecture:** Removed all hard-coded Python paths from GSPy.vcxproj file
  * Eliminated hard-coded Python include directories from all x64 configurations
  * Removed hard-coded Python library directories and dependencies
  * Preserved all existing compiler settings and project structure
* **Configuration Management:** Build system now uses Property Sheets for Python version management
  * Property Sheets provide complete Python environment setup per version
  * Support for both Debug and Release x64 configurations
  * Build fails cleanly when no Property Sheet is applied, providing clear error guidance
* **Logging Performance:** Optimized logging system for production use
  * Fast-path filtering eliminates ~90-95% of logging overhead when disabled
  * Reclassified verbose messages from INFO to DEBUG level
  * Atomic operations for thread-safe level checking without locks

### Technical Details
* Enhanced project maintainability by separating Python-specific configuration from core project settings
* Improved team collaboration with portable, environment variable-based configuration
* Reduced build system complexity and eliminated version-specific path maintenance
* Added support for future Python versions through template-based Property Sheet approach

## [1.7.1] - 2025-10-19
### Added
* **Configurable Log Levels:** Added `log_level` configuration option to JSON files for performance optimization.
  * Level 0 (ERROR) - Production mode with minimal logging for maximum performance
  * Level 1 (WARNING) - Errors and warnings only
  * Level 2 (INFO) - Default level with informational messages
  * Level 3 (DEBUG) - Full verbose logging for development
* **Convenience Logging Functions:** Added `LogError()`, `LogWarning()`, `LogInfo()`, and `LogDebug()` functions for better code organization.
* **Python Logging Support:** Python scripts can now write custom messages to the GSPy log file using `import gspy; gspy.log("message", level)`.

### Changed
* **Optimized File I/O:** Replaced `std::endl` with `'\n'` to eliminate unnecessary buffer flushing, improving logging performance.
* **Level-Based Filtering:** Log messages are now filtered at the source based on configured level, preventing unnecessary string operations and I/O calls.
* **Production Performance:** With `log_level: 0`, logging overhead is reduced by ~90-95%, significantly improving simulation performance.

### Technical Details
* Enhanced `Logger.cpp` and `Logger.h` with level-based logging system
* Added `GetLogLevel()` function to `ConfigManager` for reading log level from JSON configuration
* Implemented `gspy` Python module with `log()` function for Python-to-log communication
* Updated documentation in README.md with performance optimization and Python logging guidance

## [1.7.0] - 2025-09-18
### Added
Dynamic Array Sizing for Inputs: Implemented a major feature allowing the dimensions of input vector and matrix types to be determined dynamically at runtime.
* Introduced a new optional "dimensions_from" key in the JSON configuration.
* This key allows a user to reference the name of one or more scalar inputs that will provide the size for the array's dimensions during the simulation.
* Added comprehensive configuration validation and robust runtime error handling for this feature.

### Changed
The GSPy DLL will now report a variable number of inputs (-1) to GoldSim whenever a dynamic sizing configuration (dimensions_from) is used. This ensures GoldSim correctly handles the variable data stream.

## [1.6.0] - 2025-09-17

This is a complete rewrite and architectural overhaul of the GSPy bridge. It is not backward-compatible with previous versions.

### Added

* **Modular C++ Architecture:** The entire C++ codebase has been refactored into clean, single-responsibility modules (`PythonManager`, `ConfigManager`, `Logger`, etc.) for robustness and maintainability.
* **Flexible Data Marshalling:** Full support for any combination of inputs and outputs, including:
    * Scalars
    * Vectors (1D Arrays)
    * Matrices (2D Arrays)
    * Time Series (scalar, vector, matrix, series)
    * Lookup Tables (1D, 2D, and 3D Output)
* **JSON-Based Configuration:** Replaced the old `gspy_config.txt` with a modern, clear `.json` file format.
* **Dynamic DLL Naming Convention:** The DLL can be renamed (e.g., `my_calc.dll`) and will automatically look for a matching configuration file (`my_calc.json`), allowing multiple GSPy functions in a single model.
* **Portable Python Environment:** The DLL is now configured to work with a self-contained, portable Python environment, eliminating dependency issues on end-user machines.
* **Diagnostic Logging:** A robust, file-based logging system (`_log.txt`) is now included for easy debugging.

### Changed (Breaking Changes)

* **Configuration System:** The old `gspy_config.txt` is no longer supported. All projects must be migrated to the new `.json` format.
* **Python API:** The Python function must now accept arguments via `*args` and **must** return a `tuple` of results.
* **Error Handling:** The error reporting mechanism has been completely rewritten to be more robust and provide clearer messages to the GoldSim user.

### Removed

* Removed all legacy code from previous versions. The project is a complete rewrite from the ground up.

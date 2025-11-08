# Changelog

All notable changes to this project will be documented in this file.

## [1.8.5] - 2025-11-07

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
- Version number updated to 1.8.5 in GSPy.h and README.md

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

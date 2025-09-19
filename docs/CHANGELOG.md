# Changelog

All notable changes to this project will be documented in this file.

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

# Requirements Document

## Introduction

GSPy v1.7.1 fails to initialize on systems running Python 3.12 or newer due to breaking changes in the Python C-API. The NumPy C-API initialization requires explicit module import before calling _import_array() in Python 3.12+. This patch addresses the compatibility issue while maintaining backward compatibility with older Python versions.

## Glossary

- **GSPy**: GoldSim-Python Bridge - A C++ library that enables integration between GoldSim and Python
- **NumPy_C_API**: The C-level interface provided by NumPy for C/C++ extensions
- **PythonManager**: The C++ class responsible for managing Python runtime initialization in GSPy
- **_import_array()**: NumPy macro that initializes the NumPy C-API for use in C/C++ code
- **PyImport_ImportModule()**: Python C-API function that imports a Python module programmatically

## Requirements

### Requirement 1

**User Story:** As a GoldSim user with Python 3.12+, I want GSPy to initialize successfully, so that I can use Python functionality in my GoldSim models.

#### Acceptance Criteria

1. WHEN GSPy initializes on Python 3.12 or newer, THE PythonManager SHALL import the numpy module before calling _import_array()
2. IF PyImport_ImportModule("numpy") fails, THEN THE PythonManager SHALL log an appropriate error message and return false
3. WHEN numpy import succeeds, THE PythonManager SHALL properly release the module reference and proceed with _import_array()
4. THE GSPy SHALL log "NumPy C-API initialized successfully" message upon successful initialization
5. WHEN GSPy runs on Python 3.14, THE system SHALL initialize and execute test models without errors

### Requirement 2

**User Story:** As a GoldSim user with Python 3.11 or older, I want GSPy v1.7.3 to continue working, so that upgrading GSPy doesn't break my existing workflows.

#### Acceptance Criteria

1. WHEN GSPy v1.7.3 runs on Python 3.11, THE system SHALL initialize successfully using the enhanced initialization process
2. THE PythonManager SHALL maintain backward compatibility with all Python versions that worked with v1.7.1
3. WHEN existing GoldSim models run with GSPy v1.7.3 on Python 3.11, THE system SHALL produce identical results to v1.7.1

### Requirement 3

**User Story:** As a developer, I want clear error messages when NumPy is missing, so that I can quickly diagnose installation issues.

#### Acceptance Criteria

1. IF numpy module is not installed or cannot be imported, THEN THE PythonManager SHALL set errorMessage to "PyImport_ImportModule('numpy') failed. Check if NumPy is installed."
2. WHEN a Python import error occurs, THE system SHALL call PyErr_Print() to log the Python-level error details
3. THE GSPy SHALL provide clear log entries that distinguish between NumPy import failures and C-API initialization failures

### Requirement 4

**User Story:** As a developer, I want the project to use modern NumPy API versions, so that we avoid deprecated functionality and build warnings.

#### Acceptance Criteria

1. THE PythonManager SHALL define NPY_NO_DEPRECATED_API as NPY_1_22_API_VERSION
2. WHEN the project builds in Release x64 configuration, THE system SHALL produce no new warnings related to NumPy API usage
3. THE updated API version SHALL maintain compatibility with the minimum supported NumPy version

### Requirement 5

**User Story:** As a release manager, I want the version number updated to 1.7.3, so that users can identify the patched version.

#### Acceptance Criteria

1. THE GSPy SHALL update GSPY_VERSION_MINOR to 7
2. THE GSPy SHALL update GSPY_VERSION_PATCH to 3
3. WHEN users query the GSPy version, THE system SHALL report version 1.7.3
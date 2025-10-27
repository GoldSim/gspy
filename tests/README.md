# GSPy Test Suite

This directory contains test programs for validating GSPy functionality.

## Test Programs

### Version Tests
- `test_version_system.cpp` - Tests version constant generation and formatting
- `test_version_reporting.cpp` - Tests GoldSim version reporting interface

### Logging Tests  
- `test_logger_header.cpp` - Tests log file header generation
- `test_logger_fallback.cpp` - Tests stderr fallback when file operations fail

## Running Tests

Compile and run each test individually:

```cmd
cl test_version_system.cpp /I. /Fe:test_version_system.exe
test_version_system.exe
```

## Test Requirements

- Visual Studio C++ compiler
- Access to GSPy header files
- Write permissions for log file testing
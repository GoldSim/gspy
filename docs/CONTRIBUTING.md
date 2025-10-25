# Contributing to GSPy

## What GSPy Is (and Is Not)

**What GSPy Is**: A lightweight, open-source C++ bridge that connects GoldSim to a user's own standard Python environment. It acts as a data marshaller between GoldSim's External element and Python scripts, enabling users to leverage Python's capabilities within GoldSim simulations without requiring C++ knowledge.

**What GSPy Is Not**: GSPy is **not** a formal GoldSim product, a Python distribution, or a formally supported tool. GoldSim support cannot provide assistance for custom Python code, library usage, or environment configuration issues.

Thank you for your interest in contributing to GSPy! This document provides guidelines for contributing to the project.

## Code of Conduct

Please be respectful and professional in all interactions. GSPy is developed to serve the GoldSim modeling community, and we welcome contributions that advance this goal.

## How to Contribute

### Reporting Bugs

When reporting bugs, please include:

1. **Environment Information**:
   - GoldSim version (14+)
   - Python version (3.8+ recommended)
   - GSPy version
   - Windows version (64-bit)

2. **Reproduction Steps**:
   - Minimal example that reproduces the issue
   - Expected behavior vs. actual behavior
   - Any error messages or log output

3. **Supporting Files**:
   - Log file contents (e.g., `my_calc_log.txt`)
   - Sample JSON configuration file
   - Sample Python script that demonstrates the issue
   - GoldSim model file (if applicable)

### Suggesting Enhancements

For feature requests, please provide:

1. **Use Case Description**: What problem does this solve?
2. **Proposed Solution**: How should it work within the three-component architecture?
3. **Examples**: Sample JSON configuration and Python code
4. **Impact**: Who would benefit from this feature?

### Contributing Code

#### Development Setup

1. **Prerequisites**:
   - Visual Studio 2022 (Community edition or higher) - 64-bit build support
   - Python 3.8+ with NumPy (3.13+ recommended for development)
   - GoldSim 14+ (for testing)

2. **Build Environment**:
   ```bash
   # Clone the repository
   git clone [repository-url]
   cd GSPy
   
   # Open in Visual Studio
   start GSPy.sln
   ```

3. **Testing**:
   - Test with the three-component architecture (DLL + JSON + Python script)
   - Verify the naming convention (DLL and JSON must have matching names)
   - Test with multiple Python environments
   - Test various data types (scalars, vectors, matrices, time series, lookup tables)

#### Code Style Guidelines

**C++ Code**:
- Follow existing naming conventions
- Include comprehensive error handling with clear error messages
- Add appropriate logging statements
- Document complex algorithms, especially data marshalling logic
- Maintain memory safety practices
- Support the stateless operation principle

**Python Code**:
- Follow PEP 8 style guidelines
- Include docstrings for functions
- Return data as tuples or dictionaries as specified in requirements
- Add error handling for edge cases with logging to files (not console)
- Provide clear variable names
- Include test cases when run directly

**JSON Configuration**:
- Follow the exact schema defined in requirements.md
- Include all required fields: `python_path`, `script_path`, `function_name`, `inputs`, `outputs`
- Use appropriate `max_points` and `max_elements` for dynamic outputs
- Validate configurations are syntactically correct

#### Pull Request Process

1. **Branch Naming**: Use descriptive branch names
   - `feature/add-scipy-support`
   - `bugfix/memory-leak-fix`
   - `docs/update-readme`

2. **Commit Messages**: Use clear, descriptive commit messages
   - Start with verb in present tense
   - Include context and reasoning
   - Reference issues when applicable

3. **Testing Requirements**:
   - All existing examples must continue working
   - New features require example implementations with complete three-component setup
   - Test data marshalling for all supported data types
   - Include appropriate error handling tests
   - Verify naming convention compliance (DLL and JSON matching names)

4. **Documentation**:
   - Update README.md if needed
   - Add or update example documentation
   - Update requirements.md for architectural changes
   - Update CHANGELOG.md

### Documentation Contributions

Documentation improvements are always welcome:

- Fix typos or unclear explanations
- Add more detailed examples with complete JSON configurations
- Improve setup instructions
- Clarify the three-component architecture
- Document data marshalling patterns

### Example Contributions

New examples are valuable contributions and should demonstrate real-world usage:

1. **Example Requirements**:
   - Include all three components: renamed DLL, matching JSON config, and Python script
   - Solve a real-world problem
   - Include comprehensive documentation
   - Follow the standard function signature and return patterns
   - Include test data and expected results
   - Demonstrate proper error handling

2. **Example Categories**:
   - Scientific computing applications
   - Engineering calculations  
   - Data processing workflows
   - Integration with popular libraries (NumPy, SciPy, Pandas)
   - Time series analysis
   - Lookup table generation

## Development Workflow

### Setting Up Development Environment

1. **Install Dependencies**:
   ```bash
   # Python dependencies (NumPy is required)
   pip install numpy
   
   # Optional but recommended for development
   pip install scipy pandas matplotlib
   
   # Development tools (optional)
   pip install pytest black flake8
   ```

2. **Environment Configuration**:
   - Ensure Python path is correctly set in JSON configurations
   - Test with the actual Python installation that will be used
   - Verify JSON configurations follow the required schema

3. **Build Configuration**:
   - Use x64 Release configuration for production testing
   - Ensure version information is embedded correctly
   - Test the calling sequence (MethodID 0, 1, 2, 3, 99) compliance

### Testing Guidelines

#### Unit Testing
- Test individual data marshalling functions
- Verify error handling paths and error message clarity
- Check memory management and cleanup
- Test JSON configuration parsing

#### Integration Testing
- Test full workflow with GoldSim using the three-component architecture
- Verify all example scripts with their JSON configurations
- Test with different data sizes and types
- Test the complete calling sequence (initialization, calculation, cleanup)
- Verify naming convention enforcement

#### Performance Testing
- Benchmark with large datasets (vectors, matrices, time series)
- Monitor memory usage during data marshalling
- Check for memory leaks in Python interpreter management
- Test with maximum buffer sizes (`max_points`, `max_elements`)

## Project Structure

```
GSPy/
├── GSPy.cpp               # Main C++ interface implementation
├── GSPy.h                 # Main header file
├── PythonManager.cpp      # Python interpreter management
├── ConfigManager.cpp      # JSON configuration handling
├── Logger.cpp             # Logging system
├── TimeSeriesManager.cpp  # Time series data marshalling
├── LookupTableManager.cpp # Lookup table data marshalling
├── GSPy_Error.cpp         # Error handling
├── json.hpp              # JSON parsing library
├── x64/                  # Build outputs
├── examples/             # Test examples with DLL/JSON/Python triplets
└── docs/                 # Documentation
```

## Release Process

1. **Version Updates**:
   - Update only 3 constants in `GSPy.h`: `GSPY_VERSION_MAJOR`, `GSPY_VERSION_MINOR`, `GSPY_VERSION_PATCH`
   - All version strings, log headers, and GoldSim reporting update automatically
   - Update CHANGELOG.md with new version and changes
   - Tag releases appropriately

2. **Quality Assurance**:
   - Test all examples with the three-component architecture
   - Verify documentation accuracy with current requirements
   - Check for memory leaks
   - Test GoldSim calling sequence compliance
   - Validate JSON schema compliance

3. **Distribution**:
   - Build 64-bit version only (as per requirements)
   - Package with complete documentation
   - Include example configurations
   - Test installation process

## Getting Help

- **Questions**: Use GitHub Discussions
- **Bugs**: Create detailed GitHub Issues with all three components
- **Security**: Contact maintainers directly
- **GoldSim Support**: Refer to official GoldSim documentation (GSPy is not officially supported)

## Recognition

Contributors will be acknowledged in:
- CHANGELOG.md for significant contributions
- README.md for major features  
- Release notes for important fixes

Thank you for helping make GSPy better for the entire GoldSim community!

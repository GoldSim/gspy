# Contributing to GSPy

## What GSPy Is (and Is Not)

**What GSPy Is**: A lightweight, open-source C++ bridge that connects GoldSim to a user's own standard Python environment. It is provided as a utility for advanced users.

**What GSPy Is Not**: GSPy is **not** a formal GoldSim product, a Python distribution, or a formally supported tool. GoldSim support cannot provide assistance for custom Python code, library usage, or environment configuration issues.

Thank you for your interest in contributing to GSPy! This document provides guidelines for contributing to the project.

## Code of Conduct

Please be respectful and professional in all interactions. GSPy is developed to serve the GoldSim modeling community, and we welcome contributions that advance this goal.

## How to Contribute

### Reporting Bugs

When reporting bugs, please include:

1. **Environment Information**:
   - GoldSim version
   - Python version
   - GSPy version
   - Windows version (32-bit/64-bit)

2. **Reproduction Steps**:
   - Minimal example that reproduces the issue
   - Expected behavior vs. actual behavior
   - Any error messages or log output

3. **Supporting Files**:
   - `gspy_log.txt` contents
   - Sample `gspy_script.py` that demonstrates the issue
   - GoldSim model file (if applicable)

### Suggesting Enhancements

For feature requests, please provide:

1. **Use Case Description**: What problem does this solve?
2. **Proposed Solution**: How should it work?
3. **Examples**: Sample code or scenarios
4. **Impact**: Who would benefit from this feature?

### Contributing Code

#### Development Setup

1. **Prerequisites**:
   - Visual Studio 2022 (Community edition or higher)
   - Python 3.13+ with NumPy
   - GoldSim (for testing)

2. **Build Environment**:
   ```bash
   # Clone the repository
   git clone [repository-url]
   cd GSPy
   
   # Open in Visual Studio
   start GSPy.sln
   ```

3. **Testing**:
   - Test all example scripts
   - Verify both 32-bit and 64-bit builds
   - Test with multiple Python environments

#### Code Style Guidelines

**C++ Code**:
- Follow existing naming conventions
- Include comprehensive error handling
- Add appropriate logging statements
- Document complex algorithms
- Maintain memory safety practices

**Python Code**:
- Follow PEP 8 style guidelines
- Include docstrings for functions
- Add error handling for edge cases
- Provide clear variable names
- Include test cases when run directly

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
   - New features require example implementations
   - Include appropriate error handling tests

4. **Documentation**:
   - Update README.md if needed
   - Add or update example documentation
   - Update CHANGELOG.md

### Documentation Contributions

Documentation improvements are always welcome:

- Fix typos or unclear explanations
- Add more detailed examples
- Improve setup instructions
- Translate documentation (if applicable)

### Example Contributions

New examples are valuable contributions:

1. **Example Requirements**:
   - Solve a real-world problem
   - Include comprehensive documentation
   - Follow the standard script structure
   - Include test data and expected results

2. **Example Categories**:
   - Scientific computing applications
   - Engineering calculations
   - Data processing workflows
   - Integration with popular libraries

## Development Workflow

### Setting Up Development Environment

1. **Install Dependencies**:
   ```bash
   # Python dependencies
   pip install numpy scipy pandas matplotlib
   
   # Development tools (optional)
   pip install pytest black flake8
   ```

2. **Environment Variables**:
   - Set `GSPY` environment variable to project root
   - Ensure Python path includes NumPy headers

3. **Build Configuration**:
   - Use Release configuration for testing
   - Both x64 and Win32 platforms should build
   - Verify version information is embedded

### Testing Guidelines

#### Unit Testing
- Test individual functions in isolation
- Verify error handling paths
- Check memory management

#### Integration Testing
- Test full workflow with GoldSim
- Verify all example scripts
- Test with different data sizes

#### Performance Testing
- Benchmark with large datasets
- Monitor memory usage
- Check for memory leaks

## Project Structure

```
GSPy/
├── GSPy/                   # Main C++ source
│   ├── GSPyInterface.cpp   # Main interface
│   ├── DataMarshaller.cpp  # Data conversion
│   ├── *.h                 # Header files
│   └── GSPy.rc            # Resources
├── TestGSPy/              # Test executable
├── examples/              # Python examples
├── docs/                  # Documentation
└── bin/                   # Built DLLs
```

## Release Process

1. **Version Updates**:
   - Update version in resource files
   - Update CHANGELOG.md
   - Tag releases appropriately

2. **Quality Assurance**:
   - Test all examples
   - Verify documentation accuracy
   - Check for memory leaks

3. **Distribution**:
   - Build both 32-bit and 64-bit versions
   - Package with complete documentation
   - Test installation process

## Getting Help

- **Questions**: Use GitHub Discussions
- **Bugs**: Create detailed GitHub Issues
- **Security**: Contact maintainers directly
- **GoldSim Support**: Refer to official GoldSim documentation

## Recognition

Contributors will be acknowledged in:
- CHANGELOG.md for significant contributions
- README.md for major features
- Release notes for important fixes

Thank you for helping make GSPy better for the entire GoldSim community!

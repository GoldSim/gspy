# Design Document: Multi-Python Build System

## Overview

This design implements a flexible, maintainable build system for the GSPy project that supports multiple Python versions (3.11 and 3.14) through Visual Studio Property Sheets. The solution replaces hard-coded Python paths with reusable configuration files, enabling rapid switching between Python targets while maintaining build integrity.

## Architecture

### Current State Analysis

The existing GSPy.vcxproj file contains:
- Hard-coded Python 3.13/3.14 paths in x64 configurations
- Environment variable-based paths (PYTHON_HOME) in Win32 configurations  
- Inconsistent Python library references across configurations
- Mixed approaches that create maintenance complexity

### Target Architecture

The new architecture will use a layered configuration approach:

```
GSPy.vcxproj (Clean, no Python paths)
    ↓
Property Sheets Layer
    ├── python_311.props (Python 3.11 configuration)
    ├── python_314.props (Python 3.14 configuration)
    └── [future versions...]
    ↓
Visual Studio Build System
```

## Components and Interfaces

### 1. Clean Project File (GSPy.vcxproj)

**Purpose**: Contains only core project configuration without Python-specific settings

**Key Changes**:
- Remove all hard-coded Python include paths
- Remove all hard-coded Python library paths  
- Remove all hard-coded Python library dependencies
- Maintain existing preprocessor definitions and compiler settings
- Preserve ImportGroup sections for Property Sheet integration

### 2. Python Version Property Sheets

#### python_311.props
**Purpose**: Contains all Python 3.11 specific build configuration

**Configuration Elements**:
```xml
<AdditionalIncludeDirectories>
  - $(PYTHON_3_11_HOME)\include
  - $(PYTHON_3_11_HOME)\Lib\site-packages\numpy\_core\include
</AdditionalIncludeDirectories>

<AdditionalLibraryDirectories>
  - $(PYTHON_3_11_HOME)\libs
</AdditionalLibraryDirectories>

<AdditionalDependencies>
  - python311.lib
</AdditionalDependencies>
```

#### python_314.props  
**Purpose**: Contains all Python 3.14 specific build configuration

**Configuration Elements**:
```xml
<AdditionalIncludeDirectories>
  - $(PYTHON_3_14_HOME)\include
  - $(PYTHON_3_14_HOME)\Lib\site-packages\numpy\_core\include
</AdditionalIncludeDirectories>

<AdditionalLibraryDirectories>
  - $(PYTHON_3_14_HOME)\libs
</AdditionalLibraryDirectories>

<AdditionalDependencies>
  - python314.lib
</AdditionalDependencies>
```

### 3. Property Manager Integration

**Interface**: Visual Studio Property Manager window
- Provides GUI for adding/removing Property Sheets
- Enables rapid switching between Python configurations
- Maintains configuration state per build type (Debug/Release x64)

## Data Models

### Property Sheet Structure

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup />
  <ItemDefinitionGroup>
    <ClCompile>
      <AdditionalIncludeDirectories>
        $(PYTHON_X_XX_HOME)\include;
        $(PYTHON_X_XX_HOME)\Lib\site-packages\numpy\_core\include;
        %(AdditionalIncludeDirectories)
      </AdditionalIncludeDirectories>
    </ClCompile>
    <Link>
      <AdditionalLibraryDirectories>
        $(PYTHON_X_XX_HOME)\libs;
        %(AdditionalLibraryDirectories)
      </AdditionalLibraryDirectories>
      <AdditionalDependencies>pythonXXX.lib;%(AdditionalDependencies)</AdditionalDependencies>
    </Link>
  </ItemDefinitionGroup>
  <ItemGroup />
</Project>
```

### Environment Variables

**Required System Environment Variables**:
- `PYTHON_3_11_HOME`: Path to Python 3.11 installation (e.g., `C:\Users\[User]\AppData\Local\Programs\Python\Python311`)
- `PYTHON_3_14_HOME`: Path to Python 3.14 installation (e.g., `C:\Users\[User]\AppData\Local\Programs\Python\Python314`)

**Benefits of Environment Variable Approach**:
- **Team Portability**: Works regardless of username or installation path
- **Flexibility**: Supports custom Python installation locations
- **Maintainability**: Single point of configuration per developer
- **Build Server Compatibility**: Easy to configure in CI/CD environments

### Configuration Matrix

| Build Config | Platform | Property Sheet | Python Version | Library |
|--------------|----------|----------------|----------------|---------|
| Debug        | x64      | python_311.props | 3.11         | python311.lib |
| Release      | x64      | python_311.props | 3.11         | python311.lib |
| Debug        | x64      | python_314.props | 3.14         | python314.lib |
| Release      | x64      | python_314.props | 3.14         | python314.lib |

## Error Handling

### Build-Time Error Prevention

1. **Missing Property Sheet Detection**
   - Build will fail cleanly if no Python Property Sheet is applied
   - Clear error messages indicating missing Python configuration

2. **Path Validation**
   - Property Sheets contain absolute paths that must exist
   - Build system will report missing Python installations clearly

3. **Library Mismatch Prevention**
   - Each Property Sheet ensures correct Python library version
   - Eliminates runtime errors from version mismatches

### Runtime Error Handling

1. **Python Version Verification**
   - GSPy code should verify Python version at runtime
   - Graceful handling of version compatibility issues

2. **Dependency Validation**
   - Verify NumPy availability and version compatibility
   - Clear error messages for missing dependencies

## Testing Strategy

### Build Verification Tests

1. **Clean Build Test**
   - Verify project builds successfully with each Property Sheet
   - Test both Debug and Release configurations
   - Validate output DLL functionality

2. **Switch Test**
   - Test rapid switching between Python versions
   - Verify build integrity after multiple switches
   - Measure switch time (target: <10 seconds)

3. **Configuration Matrix Test**
   - Build all combinations of Debug/Release with each Python version
   - Verify correct library linking for each combination
   - Test output compatibility with target Python versions

### Integration Tests

1. **Python Runtime Tests**
   - Load GSPy DLL in Python 3.11 environment
   - Load GSPy DLL in Python 3.14 environment  
   - Verify all exported functions work correctly
   - Test NumPy integration for each version

2. **Regression Tests**
   - Ensure existing functionality remains intact
   - Test all example projects with new build system
   - Verify performance characteristics unchanged

### User Experience Tests

1. **Developer Workflow Tests**
   - Time measurement for Python version switching
   - Usability testing of Property Manager interface
   - Documentation accuracy verification

2. **New Developer Onboarding**
   - Test setup process for new team members
   - Verify documentation clarity and completeness
   - Validate troubleshooting procedures

## Implementation Phases

### Phase 1: Project Cleanup
- Remove hard-coded Python paths from GSPy.vcxproj
- Verify project structure remains intact
- Test that build fails cleanly without Python configuration

### Phase 2: Environment Setup and Property Sheet Creation
- Document required environment variables (PYTHON_3_11_HOME, PYTHON_3_14_HOME)
- Create python_311.props with environment variable-based configuration
- Create python_314.props with environment variable-based configuration
- Test individual Property Sheet functionality with environment variables

### Phase 3: Integration and Testing
- Integrate Property Sheets with project configurations
- Perform comprehensive build testing
- Validate switching workflow

### Phase 4: Documentation and Team Setup
- Create user documentation with environment variable setup instructions
- Document team onboarding process for new developers
- Create troubleshooting guide for common environment issues
- Optimize switching process for speed

## Future Extensibility

### Additional Python Versions
- Template-based approach for new Python versions
- Automated Property Sheet generation scripts
- Version detection and validation utilities

### Cross-Platform Considerations
- Property Sheet approach is Windows/Visual Studio specific
- Future Linux/macOS support would require different approach
- Consider CMake integration for cross-platform builds

### Advanced Features
- Automated Python installation detection and environment variable setup
- Build configuration validation scripts
- PowerShell scripts for rapid environment variable configuration
- Integration with package managers for Python version management
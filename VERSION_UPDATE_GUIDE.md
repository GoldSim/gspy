# GSPy Version Update Guide

## How to Update the Version Number

To update GSPy to a new version, you only need to modify **three constants** in `GSPy.h`:

```cpp
#define GSPY_VERSION_MAJOR 1
#define GSPY_VERSION_MINOR 7  
#define GSPY_VERSION_PATCH 1
```

### Examples:

**For version 1.8.0:**
```cpp
#define GSPY_VERSION_MAJOR 1
#define GSPY_VERSION_MINOR 8
#define GSPY_VERSION_PATCH 0
```

**For version 2.0.0:**
```cpp
#define GSPY_VERSION_MAJOR 2
#define GSPY_VERSION_MINOR 0
#define GSPY_VERSION_PATCH 0
```

**For version 1.7.2:**
```cpp
#define GSPY_VERSION_MAJOR 1
#define GSPY_VERSION_MINOR 7
#define GSPY_VERSION_PATCH 2
```

## What Gets Updated Automatically

When you change these three constants, the following are automatically generated:

1. **String Version** (`GSPY_VERSION`): Used in log headers and display
   - Example: "1.7.1"

2. **Double Version** (`GSPY_VERSION_DOUBLE`): Used for GoldSim API compliance
   - Example: 1.71 (calculated as 1 + 7*0.1 + 1*0.01)

3. **Log File Header**: Shows the new version automatically

4. **GoldSim Version Reporting**: Returns the correct double value to GoldSim

## GoldSim API Compliance

The version system is designed to comply with GoldSim's External DLL interface:

- **methodID 2** (Report Version) returns a double value
- Version format: `MAJOR.MINOR_PATCH` (e.g., 1.71 for version 1.7.1)
- This matches GoldSim's expectation for version reporting

## Testing Version Updates

After updating the version constants:

1. **Compile the project** to ensure no syntax errors
2. **Run version tests**:
   ```
   test_version_system.exe
   test_version_reporting.exe
   ```
3. **Check log file header** by running any GSPy operation
4. **Verify GoldSim integration** by testing with a GoldSim model

## Version History Tracking

Don't forget to update:
- `docs/CHANGELOG.md` with the new version and changes
- Any documentation that references version numbers
- Release notes or deployment documentation
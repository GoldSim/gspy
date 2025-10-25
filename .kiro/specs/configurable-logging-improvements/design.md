# Design Document

## Overview

This design document outlines the implementation of a high-performance, configurable logging system for GSPy that provides unified logging for both the DLL and Python scripts. The system will enhance the existing Logger with atomic log level management, fast-path filtering, configurable flush policies, and seamless Python integration while maintaining backward compatibility.

## Architecture

### High-Level Architecture

```mermaid
graph TB
    subgraph "User Configuration"
        JSON[JSON Config File<br/>log_level: 0-3]
    end
    
    subgraph "Enhanced Logger"
        CM[ConfigManager<br/>GetLogLevel()]
        AL[std::atomic&lt;int&gt;<br/>current_log_level_atomic]
        SL[inline ShouldLog()]
        LM[std::mutex log_mutex]
        OF[std::ofstream log_file]
    end
    
    subgraph "Python Integration"
        PLB[gspy.log() function<br/>thread_local guard]
    end
    
    subgraph "Output"
        LF[Log File<br/>{config_name}_log.txt]
        STDERR[stderr fallback]
    end
    
    JSON --> CM
    CM --> AL
    AL --> SL
    SL --> LM
    LM --> OF
    OF --> LF
    OF --> STDERR
    PLB --> SL
```

### Component Interaction Flow

1. **Initialization**: ConfigManager reads log_level from JSON, sets atomic variable
2. **Fast-path Check**: inline ShouldLog() checks atomic variable with memory_order_relaxed
3. **Thread Safety**: If logging enabled, acquire mutex before writing to ofstream
4. **Hybrid Flush**: Flush immediately for ERROR/WARNING, write-only for INFO/DEBUG
5. **Python Bridge**: gspy.log() uses same ShouldLog() path with reentrancy guard
6. **Fallback**: If file open fails, single stderr warning then redirect to stderr

## Components and Interfaces

### 1. Enhanced Logger.h/Logger.cpp

**Purpose**: Add atomic log level management and fast-path filtering to existing Logger

**New Additions**:
```cpp
// Add to Logger.h
void SetLogLevelFromInt(int level);
inline bool ShouldLog(LogLevel level);

// Add to Logger.cpp
std::atomic<int> current_log_level_atomic{LOG_INFO}; // Default LOG_INFO
std::mutex log_mutex;
bool fallback_to_stderr = false;
```

**Key Changes**:
- Add `std::atomic<int> current_log_level_atomic` (default LOG_INFO)
- Add `SetLogLevelFromInt(int)` function
- Add inline `ShouldLog()` using `memory_order_relaxed`
- Add `std::mutex log_mutex` for thread-safe writes
- Modify `Log()` to acquire mutex only if `ShouldLog()` returns true
- Add hybrid flush: immediate for ERROR/WARNING, write-only for INFO/DEBUG
- Add stderr fallback if file open fails

### 2. Enhanced PythonManager.cpp

**Purpose**: Add gspy.log() function with reentrancy protection

**New Additions**:
```cpp
// Add thread_local guard
static thread_local bool in_logging_call = false;

// Modify existing PythonLog function
static PyObject* PythonLog(PyObject* self, PyObject* args) {
    if (in_logging_call) return Py_RETURN_NONE; // Prevent recursion
    
    in_logging_call = true;
    // ... existing logic using Logger functions ...
    in_logging_call = false;
    
    Py_RETURN_NONE;
}
```

**Key Changes**:
- Add `thread_local bool in_logging_call` reentrancy guard
- Existing `PythonLog` function already calls Logger functions
- No changes to module registration (already working)

### 3. ConfigManager.cpp (Minimal Changes)

**Purpose**: Existing GetLogLevel() already works, just ensure proper default

**Current Implementation**: Already returns log_level from JSON with fallback to default
**No Changes Needed**: Current implementation already meets MVP requirements

## Data Models

### Log Level Enumeration (Existing)
```cpp
enum LogLevel {
    LOG_ERROR = 0,    // Critical errors only
    LOG_WARNING = 1,  // Errors + warnings  
    LOG_INFO = 2,     // Errors + warnings + info (default)
    LOG_DEBUG = 3     // All messages (development only)
};
```

### Configuration Schema (Existing)
```json
{
    "python_path": "C:\\Python311",
    "script_path": "my_script.py", 
    "function_name": "process_data",
    "log_level": 2,  // 0=ERROR, 1=WARNING, 2=INFO, 3=DEBUG (optional)
    "inputs": [...],
    "outputs": [...]
}
```

**Note**: ConfigManager already handles log_level parsing with proper defaults

## Error Handling

### Fallback Strategy
1. **Primary Path**: Write to configured log file
2. **Fallback Path**: If file operations fail, write single warning to stderr and redirect all subsequent logging to stderr
3. **No Exceptions**: All logging operations are exception-safe
4. **Graceful Degradation**: System continues operating even if logging fails

### Error Scenarios and Responses
- **File Creation Failure**: Single stderr warning, switch to stderr logging
- **Disk Full**: Attempt stderr fallback, continue execution
- **Permission Denied**: Attempt stderr fallback, continue execution
- **Python Integration Failure**: Log error, continue with DLL-only logging

## Testing Strategy

### Core Testing Focus
1. **Atomic Operations**: SetLogLevelFromInt() thread safety
2. **Fast-path Filtering**: ShouldLog() performance and correctness  
3. **Thread Safety**: Mutex behavior during concurrent logging
4. **Python Integration**: gspy.log() with reentrancy protection
5. **Fallback Handling**: stderr redirect when file operations fail
6. **Log Reclassification**: Verbose messages moved to DEBUG level

### Performance Validation
- **Disabled Logging**: Minimal overhead (< 10ns per filtered call)
- **Thread Safety**: No deadlocks under concurrent access
- **Python Bridge**: No recursion, proper level filtering

## Backward Compatibility

### Existing Interface Preservation
- All current Logger.h functions remain unchanged
- Existing log calls continue to work without modification
- ConfigManager interface extensions are additive only
- Python module registration maintains existing behavior

### Migration Strategy
- Existing code requires no changes
- New features are opt-in through configuration
- Performance improvements are automatic
- Fallback behavior ensures reliability
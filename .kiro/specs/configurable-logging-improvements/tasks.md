# Implementation Plan

- [x] 1. Enhance Logger with atomic log level management





  - Add `std::atomic<int> current_log_level_atomic` with default LOG_INFO value
  - Implement `SetLogLevelFromInt(int level)` function with validation
  - Add `std::mutex log_mutex` for thread-safe file operations
  - _Requirements: 1.1, 1.2, 1.3, 1.4_

- [x] 2. Implement fast-path filtering with thread-safe writes





  - Add inline `ShouldLog(LogLevel level)` function using memory_order_relaxed
  - Modify existing `Log()` function to acquire mutex only when ShouldLog() returns true
  - Ensure atomic read performance for disabled logging paths
  - _Requirements: 2.1, 2.2, 2.3, 2.4_

- [x] 3. Implement hybrid flush policy





  - Modify `Log()` function to flush immediately for ERROR and WARNING levels
  - Use write-only behavior (no flush) for INFO and DEBUG levels
  - Maintain existing timestamp and message formatting
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5_

- [x] 4. Add robust fallback handling






  - Modify `InitLogger()` to detect file open failures
  - Write single warning to stderr when file operations fail
  - Redirect all subsequent logging to stderr without exceptions
  - Maintain thread safety during fallback operations
  - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5_

- [x] 5. Enhance Python logging bridge




  - Add `thread_local bool in_logging_call` reentrancy guard to PythonManager.cpp
  - Modify existing `PythonLog()` function to prevent recursion
  - Ensure Python gspy.log() respects configured log levels from JSON
  - Maintain existing gspy module registration and function signature
  - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5_

- [x] 6. Reclassify verbose log messages





  - Review existing Log/LogInfo calls for per-element operations and move to LogDebug
  - Review marshalling detail messages and reclassify from INFO to DEBUG
  - Review initialization check messages and reclassify from INFO to DEBUG  
  - Preserve lifecycle errors and success messages at INFO level
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 7. Initialize enhanced logging system






  - Modify GSPy.cpp to call SetLogLevelFromInt() during logger initialization
  - Ensure log level is read from ConfigManager and applied atomically
  - Maintain existing log filename generation and single-file behavior
  - Verify early initialization before subsystem operations
  - _Requirements: 4.1, 4.2, 4.3, 4.4, 4.5_
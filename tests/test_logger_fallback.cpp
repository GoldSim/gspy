#include "../Logger.h"
#include <iostream>
#include <string>
#include <filesystem>

// Simple test to verify fallback handling works
int main() {
    std::cout << "Testing Logger fallback handling..." << std::endl;
    
    // Test 1: Try to initialize with an invalid path (should trigger fallback)
    std::string invalid_path = "/invalid/path/that/does/not/exist/test.log";
    
    std::cout << "Test 1: Initializing with invalid path: " << invalid_path << std::endl;
    InitLogger(invalid_path, LOG_INFO);
    
    // Test logging after fallback initialization
    std::cout << "Test 1: Attempting to log messages (should go to stderr)..." << std::endl;
    LogError("This is a test error message");
    LogWarning("This is a test warning message");
    LogInfo("This is a test info message");
    LogDebug("This is a test debug message");
    
    std::cout << "Test 1: Completed - check stderr output above" << std::endl;
    
    // Test 2: Try to initialize with a valid path
    std::string valid_path = "test_valid.log";
    
    std::cout << "\nTest 2: Initializing with valid path: " << valid_path << std::endl;
    InitLogger(valid_path, LOG_INFO);
    
    // Test logging after successful initialization
    std::cout << "Test 2: Attempting to log messages (should go to file)..." << std::endl;
    LogError("This is a test error message to file");
    LogWarning("This is a test warning message to file");
    LogInfo("This is a test info message to file");
    LogDebug("This is a test debug message to file - should not appear (level too high)");
    
    std::cout << "Test 2: Completed - check " << valid_path << " file" << std::endl;
    
    // Clean up test file
    if (std::filesystem::exists(valid_path)) {
        std::filesystem::remove(valid_path);
        std::cout << "Cleaned up test file: " << valid_path << std::endl;
    }
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}
#include "../Logger.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

int main() {
    std::cout << "Testing logger header functionality..." << std::endl;
    
    std::string test_file = "test_header_log.txt";
    
    // Test: Initialize logger - this should write the header
    std::cout << "Initializing logger with file: " << test_file << std::endl;
    InitLogger(test_file, LOG_INFO);
    
    // Write a few test messages
    LogInfo("Logger initialized successfully");
    LogInfo("This is a test message");
    LogDebug("This debug message should not appear with INFO level");
    LogWarning("This is a warning message");
    LogError("This is an error message");
    
    std::cout << "Test messages written. Checking file contents..." << std::endl;
    
    // Read and display the log file contents
    if (std::filesystem::exists(test_file)) {
        std::ifstream log_file(test_file);
        std::string line;
        std::cout << "\n--- Log file contents ---" << std::endl;
        while (std::getline(log_file, line)) {
            std::cout << line << std::endl;
        }
        std::cout << "--- End of log file ---\n" << std::endl;
        log_file.close();
        
        // Clean up test file
        std::filesystem::remove(test_file);
        std::cout << "Cleaned up test file: " << test_file << std::endl;
    } else {
        std::cout << "ERROR: Test file was not created!" << std::endl;
        return 1;
    }
    
    std::cout << "Header test completed successfully!" << std::endl;
    return 0;
}
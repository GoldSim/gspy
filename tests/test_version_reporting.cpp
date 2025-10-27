#include "../GSPy.h"
#include <iostream>
#include <filesystem>

int main() {
    std::cout << "Testing GSPy version reporting..." << std::endl;
    
    int status = 0;
    double inargs[1] = {0.0};  // Not used for version reporting
    double outargs[10] = {0.0}; // Enough space for any response
    
    // Test version reporting (methodID = 2)
    std::cout << "Calling GSPy with methodID 2 (Report Version)..." << std::endl;
    GSPy(2, &status, inargs, outargs);
    
    std::cout << "Status: " << status << std::endl;
    std::cout << "Reported version: " << outargs[0] << std::endl;
    std::cout << "Expected version: " << GSPY_VERSION_DOUBLE << std::endl;
    
    // Verify the version matches
    if (outargs[0] == GSPY_VERSION_DOUBLE) {
        std::cout << "SUCCESS: Version reporting works correctly!" << std::endl;
    } else {
        std::cout << "ERROR: Version mismatch!" << std::endl;
        return 1;
    }
    
    // Clean up any log files created during testing
    std::string log_file = "GSPy_log.txt";
    if (std::filesystem::exists(log_file)) {
        std::filesystem::remove(log_file);
        std::cout << "Cleaned up log file: " << log_file << std::endl;
    }
    
    std::cout << "Version reporting test completed successfully!" << std::endl;
    return 0;
}
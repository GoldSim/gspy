#include "../GSPy.h"
#include <iostream>
#include <iomanip>

int main() {
    std::cout << "Testing new version management system..." << std::endl;
    
    // Test version components
    std::cout << "Version components:" << std::endl;
    std::cout << "  Major: " << GSPY_VERSION_MAJOR << std::endl;
    std::cout << "  Minor: " << GSPY_VERSION_MINOR << std::endl;
    std::cout << "  Patch: " << GSPY_VERSION_PATCH << std::endl;
    
    // Test generated string version
    std::cout << "Generated string version: " << GSPY_VERSION << std::endl;
    
    // Test generated double version
    std::cout << "Generated double version: " << std::fixed << std::setprecision(2) 
              << GSPY_VERSION_DOUBLE << std::endl;
    
    // Test that they match expected values
    std::string expected_string = "1.8.0";
    double expected_double = 1.80;
    
    if (std::string(GSPY_VERSION) == expected_string) {
        std::cout << "✓ String version matches expected: " << expected_string << std::endl;
    } else {
        std::cout << "✗ String version mismatch!" << std::endl;
        return 1;
    }
    
    if (GSPY_VERSION_DOUBLE == expected_double) {
        std::cout << "✓ Double version matches expected: " << expected_double << std::endl;
    } else {
        std::cout << "✗ Double version mismatch! Got: " << GSPY_VERSION_DOUBLE 
                  << ", Expected: " << expected_double << std::endl;
        return 1;
    }
    
    std::cout << "\nVersion system test completed successfully!" << std::endl;
    std::cout << "\nTo update version in the future, simply change the three constants in GSPy.h:" << std::endl;
    std::cout << "  GSPY_VERSION_MAJOR, GSPY_VERSION_MINOR, GSPY_VERSION_PATCH" << std::endl;
    
    return 0;
}
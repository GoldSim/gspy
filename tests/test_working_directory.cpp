#include "../GSPy/GSPyInterface.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

int main() {
    std::cout << "Testing automatic script directory logging...\n";
    
    // Get current directory using Windows API
    char currentDirBuffer[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDirBuffer);
    std::string currentDir(currentDirBuffer);
    std::string testScriptPath = currentDir + "\\test_script.py";
    
    // Create a simple test script
    std::ofstream scriptFile(testScriptPath);
    scriptFile << "def goldsim_calculate(inputs):\n";
    scriptFile << "    value = inputs.get('value', 0.0)\n";
    scriptFile << "    return {'result': value * 2.0 + 1.0}\n";
    scriptFile.close();
    
    std::cout << "Created test script at: " << testScriptPath << std::endl;
    
    // Test: Initialize with the test script (this should automatically set working directory)
    char errorMsg[256];
    bool result = gs_SetScriptPath(testScriptPath.c_str(), errorMsg, sizeof(errorMsg));
    
    if (!result) {
        std::cout << "gs_SetScriptPath failed: " << errorMsg << std::endl;
    } else {
        std::cout << "gs_SetScriptPath succeeded" << std::endl;
    }
    
    // Check if log file was created in the same directory as the script
    std::string logPath = currentDir + "\\gspy_log.txt";
    std::ifstream logFile(logPath);
    
    if (logFile.good()) {
        std::cout << "SUCCESS: Log file created at: " << logPath << std::endl;
        
        // Read and display log contents
        std::string line;
        std::cout << "\nLog file contents:\n";
        std::cout << "==================\n";
        while (std::getline(logFile, line)) {
            std::cout << line << std::endl;
        }
        std::cout << "==================\n";
        
        logFile.close();
    } else {
        std::cout << "ERROR: Log file not found at: " << logPath << std::endl;
    }
    
    // Test gs_GetNumberOfInputs (should be logged)
    int numInputs = gs_GetNumberOfInputs();
    std::cout << "gs_GetNumberOfInputs returned: " << numInputs << std::endl;
    
    // Cleanup
    gs_Finalize();
    
    // Clean up test script
    std::filesystem::remove(testScriptPath);
    
    return 0;
}
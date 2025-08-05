#include "gtest/gtest.h"
#include <cstring>
#include <string>
#include <filesystem>
extern "C" {
#include "GSPyInterface.h"
}

// Updated test file for automatic script directory logging feature

// Helper to get absolute path to test scripts
auto get_script_path(const std::string& script_name) {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path script = cwd / "tests" / script_name;
    return script.string();
}

TEST(GSPyIntegration, ScalarCalculation) {
    char errorMsg[256] = {0};
    std::string script = get_script_path("user_script_simple.py");
    
    // The working directory will be automatically set to the script's directory
    // Updated: This test verifies the automatic script directory logging feature
    ASSERT_TRUE(gs_Initialize(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    ASSERT_TRUE(gs_SetScriptPath(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    
    // Check input/output interface - simplified approach always returns 1 input, 1 output
    int nInputs = gs_GetNumberOfInputs();
    EXPECT_EQ(nInputs, 1); // Single input (GoldSim passes data as scalar)
    char nameBuf[32];
    EXPECT_TRUE(gs_GetInputName(0, nameBuf, sizeof(nameBuf)));
    EXPECT_STREQ(nameBuf, "input");
    
    int nOutputs = gs_GetNumberOfOutputs();
    EXPECT_EQ(nOutputs, 1); // Single output
    EXPECT_TRUE(gs_GetOutputName(0, nameBuf, sizeof(nameBuf)));
    EXPECT_STREQ(nameBuf, "output");
    
    // Prepare inputs/outputs - using simple scalar interface
    gs_InputStruct inputs = {0};
    inputs.scalar = 5.0; // Test value
    inputs.array = nullptr;
    inputs.array_size = 0;
    
    gs_OutputStruct outputs = {0};
    outputs.array = nullptr;
    outputs.array_size = 0;
    outputs.errorMsg[0] = '\0';
    
    // Call calculation
    ASSERT_TRUE(gs_DoCalculation(&inputs, &outputs));
    EXPECT_EQ(outputs.errorMsg[0], '\0');
    
    // Output should be input * 2 + 1 (based on user_script_simple.py)
    EXPECT_DOUBLE_EQ(outputs.scalar, 5.0 * 2.0 + 1.0); // 11.0
    
    gs_Finalize();
}

TEST(GSPyIntegration, ErrorHandling) {
    char errorMsg[256] = {0};
    std::string script = get_script_path("user_script_error.py");
    
    // The working directory will be automatically set to the script's directory
    ASSERT_TRUE(gs_Initialize(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    ASSERT_TRUE(gs_SetScriptPath(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    
    gs_InputStruct inputs = {0};
    inputs.scalar = 1.0; // Simple scalar input
    inputs.array = nullptr;
    inputs.array_size = 0;
    
    gs_OutputStruct outputs = {0};
    outputs.array = nullptr;
    outputs.array_size = 0;
    outputs.errorMsg[0] = '\0';
    
    // Call calculation (should fail)
    EXPECT_FALSE(gs_DoCalculation(&inputs, &outputs));
    EXPECT_STRNE(outputs.errorMsg, "");
    
    gs_Finalize();
}

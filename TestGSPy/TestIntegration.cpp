// Integration tests for GoldSim-Python bridge
#include "gtest/gtest.h"
#include <cstring>
#include <string>
#include <filesystem>
extern "C" {
#include "GSPyInterface.h"
}

// Helper to get absolute path to test scripts
auto get_script_path(const std::string& script_name) {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path script = cwd / ".." / "tests" / script_name;
    return script.string();
}

TEST(GSPyIntegration, ScalarCalculation) {
    char errorMsg[256] = {0};
    std::string script = get_script_path("user_script_scalar.py");
    ASSERT_TRUE(gs_Initialize(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    ASSERT_TRUE(gs_SetScriptPath(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    // Check input/output interface
    int nInputs = gs_GetNumberOfInputs();
    EXPECT_EQ(nInputs, 2); // 'a', 'b'
    char nameBuf[32];
    EXPECT_TRUE(gs_GetInputName(0, nameBuf, sizeof(nameBuf)));
    EXPECT_STREQ(nameBuf, "a");
    EXPECT_TRUE(gs_GetInputName(1, nameBuf, sizeof(nameBuf)));
    EXPECT_STREQ(nameBuf, "b");
    int nOutputs = gs_GetNumberOfOutputs();
    EXPECT_EQ(nOutputs, 1); // 'result'
    EXPECT_TRUE(gs_GetOutputName(0, nameBuf, sizeof(nameBuf)));
    EXPECT_STREQ(nameBuf, "result");
    // Prepare inputs/outputs
    gs_InputStruct inputs = {0};
    double arr[2] = {2.5, 7.5};
    inputs.array = arr;
    inputs.array_size = 2;
    gs_OutputStruct outputs = {0};
    double outArr[1] = {0.0};
    outputs.array = outArr;
    outputs.array_size = 1;
    outputs.errorMsg[0] = '\0';
    // Call calculation
    ASSERT_TRUE(gs_DoCalculation(&inputs, &outputs));
    EXPECT_EQ(outputs.errorMsg[0], '\0');
    // Output should be a+b
    EXPECT_DOUBLE_EQ(outputs.array[0], 2.5 + 7.5);
    gs_Finalize();
}

TEST(GSPyIntegration, ErrorHandling) {
    char errorMsg[256] = {0};
    std::string script = get_script_path("user_script_error.py");
    ASSERT_TRUE(gs_Initialize(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    ASSERT_TRUE(gs_SetScriptPath(script.c_str(), errorMsg, sizeof(errorMsg)));
    ASSERT_EQ(errorMsg[0], '\0');
    gs_InputStruct inputs = {0};
    double arr[2] = {1.0, 2.0};
    inputs.array = arr;
    inputs.array_size = 2;
    gs_OutputStruct outputs = {0};
    double outArr[1] = {0.0};
    outputs.array = outArr;
    outputs.array_size = 1;
    outputs.errorMsg[0] = '\0';
    // Call calculation (should fail)
    EXPECT_FALSE(gs_DoCalculation(&inputs, &outputs));
    EXPECT_STRNE(outputs.errorMsg, "");
    gs_Finalize();
}

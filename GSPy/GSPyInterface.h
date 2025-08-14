#pragma once

#ifdef _WIN32
    #ifdef GSPY_EXPORTS
        #define GSPY_API __declspec(dllexport)
    #else
        #define GSPY_API __declspec(dllimport)
    #endif
#else
    #define GSPY_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// GoldSim I/O structures
struct gs_InputStruct {
    double scalar;
    double* array;
    int array_size;
};

struct gs_OutputStruct {
    double scalar;
    double* array;
    int array_size;
    char errorMsg[256];
};

// Main GoldSim-facing API functions
// Note: gs_DoCalculation automatically loads gspy_script.py from DLL directory
GSPY_API bool gs_Initialize(const char* scriptPath, char* errorMsg, int errorMsgSize);
GSPY_API bool gs_SetScriptPath(const char* scriptPath, char* errorMsg, int errorMsgSize);
GSPY_API void gs_Finalize();
GSPY_API bool gs_DoCalculation(gs_InputStruct* inputs, gs_OutputStruct* outputs);

// I/O metadata functions
GSPY_API int gs_GetNumberOfInputs();
GSPY_API bool gs_GetInputName(int idx, char* buffer, int buflen);
GSPY_API int gs_GetInputDataType(int idx);
GSPY_API int gs_GetNumberOfOutputs();
GSPY_API bool gs_GetOutputName(int idx, char* buffer, int buflen);
GSPY_API int gs_GetOutputDataType(int idx);

#ifdef __cplusplus
}
#endif

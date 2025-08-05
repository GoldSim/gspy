/*
Header file for ErrorRelay.cpp
*/

// Utility method used to simplify the sending of an error message to GoldSim
void CopyMsgToOutputs(const char* sMsg, double* outargs);

// String constants used in ReturnErrorMsg(int errcode, double *outargs)
static const char Error_in_Python_Compilation[] = "Error in Python compilation. Please check your Python code in imported modules.";
static const char Error_in_GSPy_DLL_in_INITIALIZATION[] = "Error in GSPy DLL in the INITIALIZATION action. See the log file.";
static const char Error_in_GSPy_DLL_in_CALCULATION[] = "Error in GSPy DLL in the CALCULATION action. See the log file.";
static const char Error_in_GSPy_DLL_in_VERSION_REPORT[] = "Error in GSPy DLL in VER. REPORT action; check Python code compilation.";
static const char Error_in_GSPy_DLL_in_ARGUMENTS_REPORT[] = "Error in GSPy DLL in the ARGUMENTS REPORT action. See the log file.";
static const char Error_in_GSPy_DLL_in_CLEANUP[] = "Error in GSPy DLL in the CLEAN UP action. See the log file.";
static const char Error_in_GSPy_DLL_Unsupported[] = "Error in GSPy DLL. Unsupported action type given to DLL interface.";

//EOF
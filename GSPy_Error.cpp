#include "GSPy_Error.h"
#include "Logger.h"
#include <cstring> // For strncpy_s

// The static buffer now lives here, dedicated to our error messenger.
static char errorMessageBuffer[256];

// The function's implementation is moved here.
void SendErrorToGoldSim(const std::string& message, int* status, double* outargs)
{
    // For 64-bit DLLs running in separate process space, pointer-based error
    // messages don't work. Instead, we just set the status to indicate failure.
    // The error message is logged, and GoldSim will show a generic error.
    
    // Log the full error message
    LogError("Fatal error to report to GoldSim: " + message);
    
    // Tell GoldSim a fatal error occurred
    // Use status = 1 for generic failure (not -1 which expects a pointer)
    *status = 1;
    
    LogDebug("Status set to: " + std::to_string(*status));
}
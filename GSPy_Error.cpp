#include "GSPy_Error.h"
#include <cstring> // For strncpy_s

// The static buffer now lives here, dedicated to our error messenger.
static char errorMessageBuffer[256];

// The function's implementation is moved here.
void SendErrorToGoldSim(const std::string& message, int* status, double* outargs)
{
    // Copy the message into our static buffer, ensuring it's null-terminated.
    strncpy_s(errorMessageBuffer, message.c_str(), sizeof(errorMessageBuffer) - 1);
    errorMessageBuffer[sizeof(errorMessageBuffer) - 1] = '\0'; // Ensure null termination

    // Tell GoldSim a fatal error with a message has occurred.
        * status = -1;

    // GoldSim expects the first output argument to be a pointer to the message.
        uintptr_t* pAddr = (uintptr_t*)outargs;
    *pAddr = (uintptr_t)errorMessageBuffer;
}
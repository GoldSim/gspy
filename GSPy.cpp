#include "GSPy.h"
#include "GSPy_Error.h"
#include "PythonManager.h"
#include <string>
#include "Logger.h"
#include "ConfigManager.h"

extern "C" void GSPy(int methodID, int* status, double* inargs, double* outargs)
{
    // Initialize the logger once using the new ConfigManager
    static bool logger_initialized = false;
    if (!logger_initialized) {
        std::string log_filename = GetLogFilename();
        int log_level = GetLogLevel(); // Get from config file
        InitLogger(log_filename, static_cast<LogLevel>(log_level));
        SetLogLevelFromInt(log_level); // Apply log level atomically
        logger_initialized = true;
    }

    LogDebug("GSPy called with MethodID: " + std::to_string(methodID));

    *status = 0;
    std::string errorMessage;

    switch (methodID)
    {
    case 0: // Initialize
        if (!InitializePython(errorMessage)) {
            SendErrorToGoldSim(errorMessage, status, outargs);
        }
        break;

    case 1: // Calculate
        ExecuteCalculation(inargs, outargs, errorMessage);
        if (!errorMessage.empty()) {
            SendErrorToGoldSim(errorMessage, status, outargs);
        }
        break;

    case 2: // Report Version
        LogInfo("Reporting version to GoldSim: " + std::string(GSPY_VERSION));
        outargs[0] = GSPY_VERSION_DOUBLE;
        break;

    case 3: // Report Arguments
        // We need to initialize to read the config to get argument counts
        if (!InitializePython(errorMessage)) {
            SendErrorToGoldSim(errorMessage, status, outargs);
            return;
        }
        outargs[0] = static_cast<double>(GetNumberOfInputs());
        outargs[1] = static_cast<double>(GetNumberOfOutputs());
        break;

    case 99: // Cleanup
        FinalizePython();
        break;

    default:
        *status = 1;
        break;
    }
}
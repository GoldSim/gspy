#pragma once

#include <string>

// A declaration of our error handling function
void SendErrorToGoldSim(const std::string& message, int* status, double* outargs);
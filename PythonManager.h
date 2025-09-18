#pragma once
#include <string>

// Initializes the Python interpreter, reads the config, and loads the script.
// Returns true on success, or provides an error message and returns false.
bool InitializePython(std::string& errorMessage);

// Cleans up Python resources.
void FinalizePython();

// Executes the Python calculation.
void ExecuteCalculation(double* inargs, double* outargs, std::string& errorMessage);

// Gets the number of inputs/outputs from the loaded configuration.
int GetNumberOfInputs();
int GetNumberOfOutputs();
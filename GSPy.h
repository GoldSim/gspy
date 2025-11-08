#pragma once
#define GSPY_API

// Version information
#define GSPY_VERSION_MAJOR 1
#define GSPY_VERSION_MINOR 8
#define GSPY_VERSION_PATCH 6

// Automatically generate version string and double from components
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define GSPY_VERSION TOSTRING(GSPY_VERSION_MAJOR) "." TOSTRING(GSPY_VERSION_MINOR) "." TOSTRING(GSPY_VERSION_PATCH)
#define GSPY_VERSION_DOUBLE (GSPY_VERSION_MAJOR + GSPY_VERSION_MINOR * 0.1 + GSPY_VERSION_PATCH * 0.01)

// Your function declaration
extern "C" GSPY_API void GSPy(int methodID, int* status, double* inargs, double* outargs);
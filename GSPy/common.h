#pragma once

// FIX #1: Force Python.h to use release-mode settings even in a Debug build
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

// FIX #2: Force the Linker to ignore the Python debug library
#ifdef _DEBUG
#pragma comment(linker, "/NODEFAULTLIB:python313_d.lib")
#endif

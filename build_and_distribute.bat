@echo off
setlocal

:: ===================================================================
:: USER CONFIGURATION
:: ===================================================================

:: Set the name of your solution file
set SOLUTION_FILE=GSPy.sln

:: Set the relative paths to your property sheets
set PROPS_DIR=.\
set PY311_PROPS=%PROPS_DIR%python_311.props
set PY314_PROPS=%PROPS_DIR%python_314.props

:: Set the source directory where the built DLLs are created
set SOURCE_DLL_DIR=.\x64\Release

:: Set the names of the final DLLs
set DLL_311=GSPy_Release_311.dll
set DLL_314=GSPy_Release_314.dll

:: Set all the example directories you want to copy the DLLs to
:: Add more lines as needed, following the "set EXAMPLE_DIRS=..." pattern
set EXAMPLE_DIRS=%EXAMPLE_DIRS% ".\examples\Simple Test"
set EXAMPLE_DIRS=%EXAMPLE_DIRS% ".\examples\Time Series Tests"
set EXAMPLE_DIRS=%EXAMPLE_DIRS% ".\examples\LookupTable Tests"
set EXAMPLE_DIRS=%EXAMPLE_DIRS% ".\examples\Mixed Data Tests"

:: ===================================================================

echo.
echo [Build Script] GSPy Multi-Python Build and Distribution
echo [Build Script] Version: 1.8.0
echo.

:: Check if property sheets exist
if not exist "%PY311_PROPS%" (
    echo [Build Script] ERROR: Python 3.11 property sheet not found: %PY311_PROPS%
    goto :eof
)
if not exist "%PY314_PROPS%" (
    echo [Build Script] ERROR: Python 3.14 property sheet not found: %PY314_PROPS%
    goto :eof
)

echo [Build Script] Locating MSBuild.exe for VS 2022...
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
    set MSBUILD_PATH=%%i
)

if not defined MSBUILD_PATH (
    echo [Build Script] ERROR: Could not find MSBuild.exe.
    echo Please ensure Visual Studio 2022 is installed.
    goto :eof
)

echo [Build Script] Found MSBuild at: %MSBUILD_PATH%
echo.

:: ---------------------------------
:: Build Python 3.11
:: ---------------------------------
echo [Build Script] Building for Python 3.11...
"%MSBUILD_PATH%" GSPy.vcxproj /p:Configuration=Release /p:Platform=x64 /p:PYTHON_VERSION=311

if %ERRORLEVEL% NEQ 0 (
    echo [Build Script] ERROR: Build failed for Python 3.11.
    goto :eof
)

echo [Build Script] Build 3.11 successful.

:: Check if versioned DLL was created
if not exist "%SOURCE_DLL_DIR%\%DLL_311%" (
    echo [Build Script] ERROR: Expected DLL not found at %SOURCE_DLL_DIR%\%DLL_311%
    goto :eof
)

:: ---------------------------------
:: Distribute Python 3.11
:: ---------------------------------
echo [Build Script] Distributing %DLL_311% to example folders...
for %%d in (%EXAMPLE_DIRS%) do (
    echo   -> Copying to %%d
    if not exist "%%d" (
        echo [Build Script] WARNING: Directory does not exist: %%d
    ) else (
        copy /Y "%SOURCE_DLL_DIR%\%DLL_311%" "%%d\%DLL_311%" > nul
        if %ERRORLEVEL% NEQ 0 (
            echo [Build Script] ERROR: Failed to copy DLL to %%d. Check permissions.
        )
    )
)
echo [Build Script] Distribution 3.11 complete.
echo.

:: ---------------------------------
:: Build Python 3.14
:: ---------------------------------
echo [Build Script] Building for Python 3.14...
"%MSBUILD_PATH%" GSPy.vcxproj /p:Configuration=Release /p:Platform=x64 /p:PYTHON_VERSION=314

if %ERRORLEVEL% NEQ 0 (
    echo [Build Script] ERROR: Build failed for Python 3.14.
    goto :eof
)

echo [Build Script] Build 3.14 successful.

:: Check if versioned DLL was created
if not exist "%SOURCE_DLL_DIR%\%DLL_314%" (
    echo [Build Script] ERROR: Expected DLL not found at %SOURCE_DLL_DIR%\%DLL_314%
    goto :eof
)

:: ---------------------------------
:: Distribute Python 3.14
:: ---------------------------------
echo [Build Script] Distributing %DLL_314% to example folders...
for %%d in (%EXAMPLE_DIRS%) do (
    echo   -> Copying to %%d
    if not exist "%%d" (
        echo [Build Script] WARNING: Directory does not exist: %%d
    ) else (
        copy /Y "%SOURCE_DLL_DIR%\%DLL_314%" "%%d\%DLL_314%" > nul
        if %ERRORLEVEL% NEQ 0 (
            echo [Build Script] ERROR: Failed to copy DLL to %%d. Check permissions.
        )
    )
)
echo [Build Script] Distribution 3.14 complete.
echo.

:: ---------------------------------
:: Summary
:: ---------------------------------
echo [Build Script] --- All builds and distributions successful! ---
echo [Build Script] Created:
echo   - %SOURCE_DLL_DIR%\%DLL_311%
echo   - %SOURCE_DLL_DIR%\%DLL_314%
echo [Build Script] Distributed to %EXAMPLE_DIRS% example directories.
echo.
pause
@echo off
setlocal EnableDelayedExpansion

REM ============================================
REM VS2022 to CMake Build System
REM ============================================
REM
REM Usage: build_and_run.bat [command] [configuration] [target]
REM
REM Commands:
REM   build           Incremental build (default)
REM   rebuild         Clean and build
REM   clean           Clean build directory only
REM   run             Run without building
REM   help            Show this help message
REM
REM Arguments:
REM   configuration   Build configuration: Debug or Release (default: Debug)
REM   target          Specific target to run after build (optional)
REM
REM Examples:
REM   build_and_run.bat                    # Build Debug incrementally and run
REM   build_and_run.bat build Release      # Build Release incrementally and run
REM   build_and_run.bat rebuild            # Clean, build Debug and run
REM   build_and_run.bat rebuild Release    # Clean, build Release and run
REM   build_and_run.bat clean              # Clean only
REM   build_and_run.bat run                # Run last built executable
REM   build_and_run.bat run MyApp          # Run specific target
REM
REM ============================================

echo ============================================
echo VS2022 to CMake Build System
echo ============================================
echo.

REM Initialize variables
set "COMMAND="
set "CONFIG=Debug"
set "TARGET="
set "BUILD_DIR=cmake_build"
set "ROOT=%~dp0"
set "CMAKE_GENERATOR="

REM Parse command line arguments
call :parse_arguments %*

REM Set default command if none specified
if "%COMMAND%"=="" set "COMMAND=build"

REM Execute command
if "%COMMAND%"=="help" goto :show_help
if "%COMMAND%"=="clean" goto :do_clean
if "%COMMAND%"=="build" goto :do_build
if "%COMMAND%"=="rebuild" goto :do_rebuild
if "%COMMAND%"=="run" goto :do_run

echo [ERROR] Unknown command: %COMMAND%
echo         Use 'build_and_run.bat help' for usage information.
exit /b 1

REM ============================================
REM Parse Arguments
REM ============================================
:parse_arguments
set "ARG1=%~1"
set "ARG2=%~2"
set "ARG3=%~3"

REM Check if first argument is a command
if /i "%ARG1%"=="build" (
    set "COMMAND=build"
    shift
    set "ARG1=%~2"
    set "ARG2=%~3"
) else if /i "%ARG1%"=="rebuild" (
    set "COMMAND=rebuild"
    shift
    set "ARG1=%~2"
    set "ARG2=%~3"
) else if /i "%ARG1%"=="clean" (
    set "COMMAND=clean"
    goto :eof
) else if /i "%ARG1%"=="run" (
    set "COMMAND=run"
    shift
    set "ARG1=%~2"
    set "ARG2=%~3"
) else if /i "%ARG1%"=="help" (
    set "COMMAND=help"
    goto :eof
) else if "%ARG1%"=="/?" (
    set "COMMAND=help"
    goto :eof
) else if "%ARG1%"=="-h" (
    set "COMMAND=help"
    goto :eof
) else if "%ARG1%"=="--help" (
    set "COMMAND=help"
    goto :eof
)

REM Parse configuration and target
if /i "%ARG1%"=="Debug" (
    set "CONFIG=Debug"
    if not "%ARG2%"=="" set "TARGET=%ARG2%"
) else if /i "%ARG1%"=="Release" (
    set "CONFIG=Release"
    if not "%ARG2%"=="" set "TARGET=%ARG2%"
) else if not "%ARG1%"=="" (
    REM If ARG1 is not a config, assume it's a target
    set "TARGET=%ARG1%"
)

goto :eof

REM ============================================
REM Command: Clean
REM ============================================
:do_clean
echo [INFO] Command: CLEAN
echo [INFO] Cleaning build directory: %BUILD_DIR%
echo.

pushd "%ROOT%"
if exist "%BUILD_DIR%" (
    echo [INFO] Removing %BUILD_DIR%...
    rmdir /s /q "%BUILD_DIR%"
    if errorlevel 1 (
        echo [WARN] Failed to completely clean build directory.
        echo        Some files may be in use.
        popd
        exit /b 1
    ) else (
        echo [INFO] Clean completed successfully.
    )
) else (
    echo [INFO] Build directory does not exist. Nothing to clean.
)
popd
exit /b 0

REM ============================================
REM Command: Build (Incremental)
REM ============================================
:do_build
echo [INFO] Command: BUILD (Incremental)
echo [INFO] Configuration: %CONFIG%
if not "%TARGET%"=="" echo [INFO] Target: %TARGET%
echo.

pushd "%ROOT%"
call :setup_environment
if errorlevel 1 (
    popd
    exit /b 1
)

call :configure_cmake
if errorlevel 1 (
    popd
    exit /b 1
)

call :build_project
if errorlevel 1 (
    popd
    exit /b 1
)

call :run_executable
popd
exit /b 0

REM ============================================
REM Command: Rebuild (Clean + Build)
REM ============================================
:do_rebuild
echo [INFO] Command: REBUILD (Clean + Build)
echo [INFO] Configuration: %CONFIG%
if not "%TARGET%"=="" echo [INFO] Target: %TARGET%
echo.

pushd "%ROOT%"

REM Clean first
if exist "%BUILD_DIR%" (
    echo [INFO] Cleaning previous build directory...
    rmdir /s /q "%BUILD_DIR%"
    if errorlevel 1 (
        echo [WARN] Failed to completely clean build directory.
        echo        Some files may be in use. Continuing anyway...
    ) else (
        echo [INFO] Clean completed successfully.
    )
    echo.
)

call :setup_environment
if errorlevel 1 (
    popd
    exit /b 1
)

call :configure_cmake
if errorlevel 1 (
    popd
    exit /b 1
)

call :build_project
if errorlevel 1 (
    popd
    exit /b 1
)

call :run_executable
popd
exit /b 0

REM ============================================
REM Command: Run (No Build)
REM ============================================
:do_run
echo [INFO] Command: RUN (No Build)
if not "%TARGET%"=="" echo [INFO] Target: %TARGET%
echo.

pushd "%ROOT%"
if not exist "%BUILD_DIR%" (
    echo [ERROR] Build directory does not exist.
    echo         Please build the project first using 'build' or 'rebuild'.
    popd
    exit /b 1
)

call :run_executable
popd
exit /b 0

REM ============================================
REM Setup Environment
REM ============================================
:setup_environment
REM Locate and setup MSVC environment (VS2022)
call :find_vsdevcmd
if not defined VSDEVCMD (
    echo [ERROR] Could not locate VsDevCmd.bat ^(Visual Studio Developer Command Prompt^).
    echo         Please ensure Visual Studio 2022 or Build Tools are installed.
    exit /b 1
)

echo [INFO] Setting up Visual Studio environment...
call "%VSDEVCMD%" -arch=x64 -host_arch=x64 >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Failed to setup Visual Studio environment.
    exit /b 1
)

REM Ensure cmake is available
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake not found in PATH after setting up VS environment.
    echo         Please install CMake from https://cmake.org/download/
    exit /b 1
)

REM Detect available build system (do this AFTER VS environment is set up)
call :detect_generator
echo [INFO] Using generator: %CMAKE_GENERATOR%

exit /b 0

REM ============================================
REM Configure CMake
REM ============================================
:configure_cmake
REM Generate CMakeLists.txt from .vcxproj if needed
if exist "generate_cmake_from_vcxproj.py" (
    if not exist "CMakeLists.txt" (
        echo [INFO] Generating CMakeLists.txt from vcxproj files...
        python "generate_cmake_from_vcxproj.py"
        if errorlevel 1 (
            echo [ERROR] Failed to generate CMakeLists.txt from vcxproj.
            echo         Please check Python is installed and the script has no errors.
            exit /b 1
        )
        echo.
    )
) else (
    if not exist "CMakeLists.txt" (
        echo [ERROR] No generate_cmake_from_vcxproj.py found and no CMakeLists.txt exists.
        exit /b 1
    )
)

REM Configure only if needed
set "NEED_CONFIGURE=0"

REM Check 1: CMakeCache.txt exists?
if not exist "%BUILD_DIR%\CMakeCache.txt" (
    echo [INFO] No CMake cache found. Need to configure.
    set "NEED_CONFIGURE=1"
    goto :do_configure
)

REM Check 2: Build files exist?
set "BUILD_FILES_MISSING=1"
if "%CMAKE_GENERATOR%"=="Ninja" (
    if exist "%BUILD_DIR%\build.ninja" (
        set "BUILD_FILES_MISSING=0"
    ) else (
        echo [INFO] build.ninja missing. Need to reconfigure.
    )
) else if "%CMAKE_GENERATOR%"=="NMake Makefiles" (
    if exist "%BUILD_DIR%\Makefile" (
        set "BUILD_FILES_MISSING=0"
    ) else (
        echo [INFO] Makefile missing. Need to reconfigure.
    )
)

if "%BUILD_FILES_MISSING%"=="1" (
    set "NEED_CONFIGURE=1"
    goto :do_configure
)

REM Check 3: Configuration matches?
findstr /C:"CMAKE_BUILD_TYPE:STRING=%CONFIG%" "%BUILD_DIR%\CMakeCache.txt" >nul 2>&1
if errorlevel 1 (
    echo [INFO] Configuration changed. Reconfiguring...
    set "NEED_CONFIGURE=1"
)

:do_configure
if "%NEED_CONFIGURE%"=="1" (
    echo [INFO] Configuring CMake for %CONFIG% build...
    cmake -S . -B "%BUILD_DIR%" -G "%CMAKE_GENERATOR%" -DCMAKE_BUILD_TYPE=%CONFIG%
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed.
        echo         Check the error messages above for details.
        exit /b 1
    )
    echo.
) else (
    echo [INFO] Using existing CMake configuration.
    echo.
)
exit /b 0

REM ============================================
REM Build Project
REM ============================================
:build_project
echo [INFO] Building project...
if "%CMAKE_GENERATOR%"=="Ninja" (
    echo [INFO] Using Ninja parallel build with %NUMBER_OF_PROCESSORS% cores
) else (
    echo         This may take a few minutes for JUCE projects...
)
echo.
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    echo         Check the error messages above for details.
    exit /b 1
)
echo.
echo [INFO] Build completed successfully!
echo.
exit /b 0

REM ============================================
REM Run Executable
REM ============================================
:run_executable
REM Determine target to run
if "%TARGET%"=="" (
    if exist "%BUILD_DIR%\targets.txt" (
        set /p TARGET=<"%BUILD_DIR%\targets.txt"
    )
)

if "%TARGET%"=="" (
    echo [INFO] No target specified and none discovered.
    echo        Executable should be in: %BUILD_DIR%
    
    REM Try to find any .exe in the build directory
    echo.
    echo [INFO] Looking for executables in build directory...
    set "FOUND_EXE="
    for %%F in ("%BUILD_DIR%\*.exe") do (
        echo        Found: %%~nxF
        if not defined FOUND_EXE set "FOUND_EXE=%%F"
    )
    
    if defined FOUND_EXE (
        echo.
        echo [INFO] Running first found executable...
        echo ============================================
        echo.
        "%FOUND_EXE%"
        echo.
        echo ============================================
        echo [INFO] Application exited with code: !ERRORLEVEL!
    ) else (
        echo        No executables found in build directory.
    )
    exit /b 0
)

REM Run the specified executable
set "EXE=%BUILD_DIR%\%TARGET%.exe"
if exist "%EXE%" (
    echo [INFO] Running: %TARGET%.exe
    echo ============================================
    echo.
    "%EXE%"
    echo.
    echo ============================================
    echo [INFO] Application exited with code: !ERRORLEVEL!
) else (
    echo [WARN] Executable not found: %EXE%
    echo        The build may have produced a different output name.
    echo        Check the %BUILD_DIR% directory for the actual executable.
    
    REM Try to find any .exe in the build directory
    echo.
    echo [INFO] Available executables in build directory:
    set "FOUND_ANY=0"
    for %%F in ("%BUILD_DIR%\*.exe") do (
        echo        - %%~nxF
        set "FOUND_ANY=1"
    )
    if "%FOUND_ANY%"=="0" (
        echo        No executables found in build directory.
    )
)
exit /b 0

REM ============================================
REM Function: find_vsdevcmd
REM Locates Visual Studio Developer Command Prompt
REM ============================================
:find_vsdevcmd
REM Try VSINSTALLDIR first (if running inside a VS dev prompt)
if defined VSINSTALLDIR (
    if exist "%VSINSTALLDIR%\Common7\Tools\VsDevCmd.bat" (
        set "VSDEVCMD=%VSINSTALLDIR%\Common7\Tools\VsDevCmd.bat"
        goto :eof
    )
)

REM Try vswhere (installed with VS 2017+)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2^>nul`) do (
        if exist "%%I\Common7\Tools\VsDevCmd.bat" (
            set "VSDEVCMD=%%I\Common7\Tools\VsDevCmd.bat"
            goto :eof
        )
    )
)

REM Fallback: common default install paths for VS2022
set "VS_PATHS=%ProgramFiles%\Microsoft Visual Studio\2022\Community;%ProgramFiles%\Microsoft Visual Studio\2022\Professional;%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise"
for %%P in (%VS_PATHS%) do (
    if exist "%%P\Common7\Tools\VsDevCmd.bat" (
        set "VSDEVCMD=%%P\Common7\Tools\VsDevCmd.bat"
        goto :eof
    )
)

REM Last resort: Try VS2019 if VS2022 not found
set "VS2019_PATHS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community;%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional;%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise"
for %%P in (%VS2019_PATHS%) do (
    if exist "%%P\Common7\Tools\VsDevCmd.bat" (
        echo [WARN] VS2022 not found, using VS2019 instead.
        set "VSDEVCMD=%%P\Common7\Tools\VsDevCmd.bat"
        goto :eof
    )
)

set "VSDEVCMD="
goto :eof

REM ============================================
REM Function: detect_generator
REM Detects the best available CMake generator
REM ============================================
:detect_generator
REM Check if user specified a generator preference
if defined CMAKE_GENERATOR (
    echo [INFO] Using user-specified generator: %CMAKE_GENERATOR%
    goto :eof
)

REM Try to detect Ninja first (preferred)
where ninja >nul 2>&1
if not errorlevel 1 (
    set "CMAKE_GENERATOR=Ninja"
    echo [INFO] Ninja detected - using fast parallel builds!
    goto :eof
)

REM Check for ninja.exe in common locations
set "NINJA_PATHS=%ProgramFiles%\Ninja;%LocalAppData%\Programs\Ninja;%ProgramFiles(x86)%\Ninja;C:\Tools\Ninja;C:\Ninja"
for %%P in (%NINJA_PATHS%) do (
    if exist "%%P\ninja.exe" (
        echo [INFO] Found Ninja at %%P
        echo [INFO] Adding to PATH for this session...
        set "PATH=%PATH%;%%P"
        set "CMAKE_GENERATOR=Ninja"
        echo [INFO] Ninja enabled - using fast parallel builds!
        goto :eof
    )
)

REM Fallback to NMake
set "CMAKE_GENERATOR=NMake Makefiles"
echo [INFO] Ninja not found - using NMake (single-threaded, slower)
echo [TIP]  For faster builds, install Ninja:
echo        1. Download from: https://github.com/ninja-build/ninja/releases
echo        2. Extract ninja.exe to a folder in your PATH
echo        3. Or run: winget install Ninja-build.Ninja
echo.
goto :eof

REM ============================================
REM Show Help
REM ============================================
:show_help
echo.
echo VS2022 to CMake Build System - Help
echo ============================================
echo.
echo Usage: build_and_run.bat [command] [configuration] [target]
echo.
echo Commands:
echo   build           Incremental build (default if no command specified)
echo                   - Keeps existing build files
echo                   - Only rebuilds changed files
echo                   - Faster for development
echo.
echo   rebuild         Clean and build
echo                   - Removes all build files first
echo                   - Ensures a fresh build
echo                   - Use when encountering build issues
echo.
echo   clean           Clean build directory only
echo                   - Removes all build artifacts
echo                   - Does not build
echo.
echo   run             Run without building
echo                   - Executes the last built executable
echo                   - Useful for testing without rebuild
echo.
echo   help            Show this help message
echo.
echo Arguments:
echo   configuration   Build configuration: Debug or Release
echo                   Default: Debug
echo.
echo   target          Specific executable target to run after build
echo                   Optional - will auto-detect if not specified
echo.
echo Examples:
echo   build_and_run.bat                    # Build Debug incrementally and run
echo   build_and_run.bat build              # Same as above
echo   build_and_run.bat build Release      # Build Release incrementally and run
echo   build_and_run.bat rebuild            # Clean, build Debug and run
echo   build_and_run.bat rebuild Release    # Clean, build Release and run
echo   build_and_run.bat clean              # Clean build directory only
echo   build_and_run.bat run                # Run last built executable
echo   build_and_run.bat run MyApp          # Run specific target without building
echo   build_and_run.bat build Debug MyApp  # Build Debug and run MyApp
echo.
echo Notes:
echo   - Default command is 'build' for faster incremental builds
echo   - Use 'rebuild' when you encounter linking errors or want a fresh build
echo   - The script auto-detects if reconfiguration is needed
echo   - Requires VS2022 (or VS2019), CMake, and Python installed
echo.
exit /b 0
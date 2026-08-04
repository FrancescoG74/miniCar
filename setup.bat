@echo off
REM Setup script for miniCar development environment (Windows)

setlocal enabledelayedexpansion

cls
echo ================================================
echo   miniCar Development Environment Setup
echo ================================================
echo.

REM Check for required tools
echo Checking for required tools...

set MISSING=0

where cmake >nul 2>&1
if errorlevel 1 (
    echo ❌ cmake: NOT FOUND
    set MISSING=1
) else (
    echo ✓ cmake found
)

where git >nul 2>&1
if errorlevel 1 (
    echo ❌ git: NOT FOUND
    set MISSING=1
) else (
    echo ✓ git found
)

where ninja >nul 2>&1
if errorlevel 1 (
    echo ❌ ninja: NOT FOUND
    set MISSING=1
) else (
    echo ✓ ninja found
)

echo.

if %MISSING% equ 1 (
    echo Missing dependencies detected.
    echo.
    echo To set up development environment on Windows:
    echo.
    echo Option 1: Using Chocolatey
    echo   choco install cmake ninja
    echo.
    echo Option 2: Using vcpkg (recommended for SDL3)
    echo   git clone https://github.com/microsoft/vcpkg
    echo   .\vcpkg\bootstrap-vcpkg.bat
    echo   .\vcpkg\vcpkg install sdl3 sdl3-ttf catch2
    echo.
    echo Option 3: Manual installation
    echo   - Download and install CMake from https://cmake.org
    echo   - Download and install Ninja from https://github.com/ninja-build/ninja
    echo   - Use vcpkg or manual setup for SDL3
    echo.
    pause
    exit /b 1
)

echo ✓ All required tools found
echo.

echo ================================================
echo   Optional: Android Development Setup
echo ================================================
echo.

set /p SETUP_ANDROID="Do you want to set up Android development? (y/n): "
if /i "%SETUP_ANDROID%"=="y" (
    echo.
    echo Android setup steps:
    echo 1. Download Android Studio from https://developer.android.com/studio
    echo 2. Install Android Studio
    echo 3. Open SDK Manager ^(Tools ^→ SDK Manager^)
    echo 4. Install:
    echo    - Android SDK API 33 ^(or higher^)
    echo    - NDK ^(Side by side^) - version 25.1 or compatible
    echo 5. Set ANDROID_NDK_HOME environment variable:
    echo    set ANDROID_NDK_HOME=C:\Users\^<YourUsername^>\AppData\Local\Android\Sdk\ndk\25.1.8937393
    echo 6. Add to system environment variables if needed
    echo.
)

echo ================================================
echo   Building miniCar
echo ================================================
echo.

if exist build (
    set /p CLEAN_BUILD="Build directory exists. Clean it? (y/n): "
    if /i "!CLEAN_BUILD!"=="y" (
        rmdir /s /q build
        echo ✓ Build directory cleaned
    )
)

echo.
echo Configuring CMake...

REM Check if vcpkg toolchain is available
if exist "%cd%\..\vcpkg\scripts\buildsystems\vcpkg.cmake" (
    set VCPKG_TOOLCHAIN="..\vcpkg\scripts\buildsystems\vcpkg.cmake"
    echo Using vcpkg toolchain: !VCPKG_TOOLCHAIN!
    cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=!VCPKG_TOOLCHAIN!
) else (
    cmake -S . -B build -G Ninja
)

if errorlevel 1 (
    echo ❌ CMake configuration failed
    pause
    exit /b 1
)

echo.
echo Building miniCar...
cmake --build build

if errorlevel 1 (
    echo ❌ Build failed
    pause
    exit /b 1
)

echo.
echo ✓ Build successful!
echo.
echo To run the game:
echo   build\minicar.exe
echo.

set /p RUN_TESTS="Run unit tests? (y/n): "
if /i "%RUN_TESTS%"=="y" (
    ctest --test-dir build --output-on-failure
)

echo.
echo ================================================
echo   Setup Complete!
echo ================================================
echo.
echo Next steps:
echo 1. Desktop: build\minicar.exe
echo 2. Android: Open in Android Studio and run
echo 3. VSCode: Open folder and use build tasks
echo.
echo Documentation:
echo   - BUILD_GUIDE.md     - Detailed build instructions
echo   - PLATFORM_NOTES.md  - Platform-specific development guide
echo   - README.md          - General project information
echo.
pause


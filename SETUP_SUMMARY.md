# Cross-Platform Build Setup Summary

This document summarizes all the changes made to enable miniCar to build on both desktop (Linux/macOS/Windows) and Android platforms.

## Overview

miniCar is now a true cross-platform game that can be built using:
- **Desktop**: CMake + Ninja (Linux/macOS/Windows)
- **Android**: Gradle + Android NDK + CMake
- **IDE Support**: VS Code and Android Studio

## Files Added

### Core Build Configuration

#### CMakeLists.txt (MODIFIED)
- Added Android platform detection (`if(ANDROID)`)
- Platform-specific SDL2 find_package configuration
- Conditional compilation for espeak-ng (desktop-only)
- Platform-specific entry point selection
- Android NDK library linking (android, log, android_app_glue)
- Test suite disabled for Android builds

#### build.gradle.kts (NEW)
Gradle build configuration for Android:
- Application ID and versioning
- NDK configuration (version 25.1)
- CMake integration via externalNativeBuild
- AndroidX dependencies
- C++17 compiler flags

#### settings.gradle.kts (NEW)
Gradle project configuration:
- Plugin management
- Repository configuration
- Project name

#### gradle.properties (NEW)
Gradle optimization properties:
- JVM memory settings
- AndroidX enablement
- Jetifier support

#### CMakePresets.json (NEW)
CMake preset configurations for different platforms:
- Linux, macOS, Windows presets
- Debug and Release configurations
- Build and test presets

### Entry Points

#### src/android_main.cpp (NEW)
Android JNI entry point:
- SDL2 Android initialization
- Android logging support
- Game initialization adapted for Android (1280×720)
- Exception handling with Android logging

#### src/main.cpp (EXISTING - UNCHANGED)
Desktop entry point remains unchanged:
- 1600×900 window resolution
- Full feature support

### Android Gradle & Manifest

#### src/main/AndroidManifest.xml (NEW)
Android application manifest:
- Application configuration
- MainActivity declaration
- Landscape orientation
- Intent filter for MAIN/LAUNCHER

#### src/main/kotlin/com/example/minicar/MainActivity.kt (NEW)
Android Activity implementation:
- Extends SDLActivity
- Handles JNI interface
- Minimal Java code (SDL2 handles most)

#### src/main/res/values/strings.xml (NEW)
Android string resources:
- App name and description

#### src/main/res/values/themes.xml (NEW)
Android theme configuration

#### src/main/res/values/colors.xml (NEW)
Android color resources

#### gradle/wrapper/gradle-wrapper.properties (NEW)
Gradle wrapper configuration

#### gradlew (NEW)
Gradle wrapper script for Unix-like systems

#### gradlew.bat (NEW)
Gradle wrapper script for Windows

#### proguard-rules.pro (NEW)
ProGuard rules for:
- Preserving JNI methods
- Keeping SDL2 Activity
- Keeping MainActivity

#### local.properties (NEW)
Local SDK/NDK path configuration:
- Android SDK path
- NDK path (customize for your system)

### VS Code Configuration

#### .vscode/settings.json (NEW)
VS Code C/C++ settings

#### .vscode/tasks.json (NEW)
Build tasks:
- Desktop: Build, Build & Run, Run Tests
- Android: Debug & Release builds
- Multi-platform support

#### .vscode/c_cpp_properties.json (NEW)
C/C++ IntelliSense configuration:
- Platform-specific include paths
- Compiler paths for different OSes
- Android NDK paths

#### .vscode/launch.json (NEW)
Debugging configurations:
- GDB for Linux/macOS
- MSVC for Windows

### Documentation

#### BUILD_GUIDE.md (NEW)
Comprehensive build guide:
- Quick start for both platforms
- Detailed requirements per platform
- Platform-specific installation steps
- Build instructions for CLI and IDEs
- Troubleshooting section
- Project structure explanation
- Platform-specific notes

#### PLATFORM_NOTES.md (NEW)
Developer guide for cross-platform development:
- Architecture overview
- How to add platform-specific code
- Platform differences table
- Common pitfalls and solutions
- Development workflows
- Debugging strategies
- Performance considerations
- Testing strategies

### Setup Scripts

#### setup.sh (NEW)
Unix-like systems setup:
- OS detection (Linux/macOS)
- Dependency checking
- Automatic installation
- Android setup guidance
- Build automation

#### setup.bat (NEW)
Windows setup:
- Tool detection
- Dependency installation guidance
- Android setup guidance
- Build automation

## Key Features

### Platform Detection
- Automatic Android detection via CMake
- Platform-specific code guards (`#ifdef ANDROID`)
- Conditional compilation in CMakeLists.txt

### Shared Game Logic
- `minicar_core` static library with platform-independent code
- Tested independently with Catch2 (desktop only)
- Used by both desktop and Android executables

### Entry Point Strategy
- Desktop: `src/main.cpp` → Direct game initialization
- Android: `src/android_main.cpp` → JNI interface → SDL_main
- Common game initialization code in `Game::init()`

### Build System Integration
- **Desktop**: CMake with Ninja
- **Android**: Gradle with CMake backend (via NDK)
- Dual support in CMakeLists.txt and build.gradle.kts

### IDE Support
- **VS Code**: Tasks, launch configs, IntelliSense
- **Android Studio**: Native Android IDE support
- **CLion**: CMake support for desktop

## Build Instructions Quick Reference

### Desktop (Linux/macOS/Windows)
```bash
./setup.sh  # or setup.bat on Windows
# or manually:
cmake -S . -B build -G Ninja
cmake --build build
./build/minicar
```

### Android
```bash
# Android Studio: Open project and click Run
# Command line:
./gradlew assembleDebug
./gradlew installDebug
```

## Testing

### Desktop Tests
- Catch2 framework
- Run: `ctest --test-dir build --output-on-failure`
- Located in: `tests/`

### Android
- Tests not supported (headless only)
- Manual testing on device/emulator required

## Architecture Changes

### Before
- Linux-only project
- CMake configuration for desktop
- No Android support

### After
- Linux/macOS/Windows/Android support
- Unified build system
- Platform-agnostic game logic in `minicar_core`
- Platform-specific code isolated in entry points
- Dual IDE support (VS Code and Android Studio)

## Development Workflow

### Adding New Features
1. Implement in `minicar_core` (if platform-independent)
2. Test on desktop with unit tests
3. Test on Android manually
4. Use `#ifdef ANDROID` for platform differences

### Building for Multiple Platforms
```bash
# Desktop
cmake -S . -B build -G Ninja && cmake --build build

# Android (separate terminal)
./gradlew assembleDebug
```

### Cross-Platform Testing
- Desktop: Unit tests + manual testing
- Android: Manual testing on device/emulator
- Verify both with same game logic

## Notes for Maintainers

1. **CMakeLists.txt updates**: Must maintain dual platform support
2. **Android Gradle updates**: Keep toolchain version consistent
3. **JNI interface**: Keep android_main.cpp minimal
4. **Documentation**: Update BUILD_GUIDE.md with new platform features
5. **Testing**: Always test on both desktop and Android

## Future Enhancements

Potential improvements:
- [ ] Touch controls for Android
- [ ] Android native menus (pause/settings)
- [ ] Automatic APK signing
- [ ] CI/CD pipeline (GitHub Actions)
- [ ] Cross-platform asset loading
- [ ] Mobile-optimized UI
- [ ] Network multiplayer (via sockets)

## Dependencies

### Desktop
- CMake 3.16+
- Ninja or Make
- SDL2 and SDL2_ttf
- Catch2 (for tests)
- libespeak-ng (optional, Linux only)

### Android
- Android Studio 2022.1+
- Android NDK 25.1 (or compatible)
- Android SDK API 21+ (min), 33+ (target)
- Gradle 8.0+

### Both
- C++17 compatible compiler
- Git for version control

## Support

For issues:
1. Check BUILD_GUIDE.md troubleshooting section
2. Review PLATFORM_NOTES.md for platform-specific guidance
3. Check VS Code or Android Studio build output
4. Review CMake/Gradle configuration files

## Summary

This cross-platform setup enables:
- ✅ Desktop development on Linux/macOS/Windows
- ✅ Android development with Android Studio
- ✅ VS Code support for both platforms
- ✅ Shared game logic across platforms
- ✅ Independent testing on desktop
- ✅ Easy maintenance of platform-specific code
- ✅ Comprehensive documentation and setup guides

The architecture follows best practices for cross-platform C++ development, keeping platform-independent logic separate from platform-specific code.


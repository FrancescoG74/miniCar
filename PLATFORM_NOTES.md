# Platform-Specific Development Notes

This document provides guidance for developing miniCar across different platforms.

## Architecture Overview

miniCar's architecture is designed to support multiple platforms with minimal platform-specific code:

```
┌─────────────────────────────────────────────────┐
│         Platform Layer (Entry Points)           │
├──────────────────┬──────────────────────────────┤
│  main.cpp        │  android_main.cpp            │
│  (Desktop)       │  (Android via JNI)           │
├──────────────────┴──────────────────────────────┤
│         Game Logic Layer (minicar_core)         │
├─────────────────────────────────────────────────┤
│  Track Math  │  Collision  │  Actors  │  Input  │
├─────────────────────────────────────────────────┤
│              SDL2 / SDL2_ttf                    │
├─────────────────────────────────────────────────┤
```

The `minicar_core` static library contains all platform-independent game logic and is tested independently with unit tests. Platform-specific code stays in the main executable.

## Adding Platform-Specific Code

### Desktop-Only Features

Use `#ifndef ANDROID` guards:

```cpp
#ifndef ANDROID
    // Desktop-specific code: espeak-ng support, specific window management, etc.
    #include <espeak-ng/speak_lib.h>
    void desktopSpecificFunction() { ... }
#endif
```

### Android-Only Features

Use `#ifdef ANDROID` guards:

```cpp
#ifdef ANDROID
    #include <android/log.h>
    void androidSpecificFunction() { 
        __android_log_print(ANDROID_LOG_INFO, "TAG", "message");
    }
#endif
```

### CMakeLists.txt Platform Logic

```cmake
if(IS_ANDROID)
    # Android-specific build configuration
    target_compile_definitions(minicar PRIVATE ANDROID_BUILD=1)
    target_link_libraries(minicar PRIVATE android log)
else()
    # Desktop-specific build configuration
    find_library(ESPEAK_NG_LIBRARY NAMES espeak-ng)
endif()
```

## Platform Differences

### Desktop (Linux/macOS/Windows)

| Aspect | Details |
|--------|---------|
| Entry Point | `src/main.cpp` → `Game::init()` → `Game::run()` |
| Window | SDL2 creates a native window (1600×900) |
| Audio | Full SDL audio support, optional espeak-ng |
| Input | SDL keyboard events + user input mapping |
| Tests | Full unit test suite with Catch2 |
| Build | CMake + Ninja/Make |
| Display Required | Yes (requires X11/Wayland on Linux) |

**Key Differences Between Desktop Platforms:**

- **Linux**: Uses X11/Wayland, espeak-ng optional
- **macOS**: Homebrew for dependencies, M1/Intel support
- **Windows**: vcpkg for dependencies, no native title bar theming

### Android

| Aspect | Details |
|--------|---------|
| Entry Point | `src/android_main.cpp` → JNI → SDL_main → `Game::init()` |
| Window | SDL2 uses Android's SurfaceView |
| Audio | SDL audio only, beep tones (no espeak-ng) |
| Input | Android keyboard events via SDL |
| Tests | Not supported (headless only) |
| Build | Gradle + Android NDK + CMake |
| Display Required | No (headless possible, but Android GUI required) |
| Orientation | Landscape only |

## Common Pitfalls

### 1. Platform-Independent Library Code

**WRONG:** Putting Android-specific code in `minicar_core`
```cpp
// ❌ In Actor.cpp (wrong - breaks desktop builds)
#ifdef ANDROID
    __android_log_print(...);
#endif
```

**RIGHT:** Keep `minicar_core` platform-agnostic
```cpp
// ✓ In main.cpp or android_main.cpp (correct)
#ifdef ANDROID
    __android_log_print(...);
#endif
```

### 2. Hard-coded Paths

**WRONG:** Hard-coding paths that don't exist on Android
```cpp
// ❌ Won't work on Android
FILE* f = fopen("/usr/share/fonts/truetype/...", "r");
```

**RIGHT:** Use SDL's font loading or bundle fonts
```cpp
// ✓ SDL_ttf handles font discovery cross-platform
TTF_Font* font = TTF_OpenFont("Arial.ttf", 28);
```

### 3. SDL2 Initialization

**Android-specific:** SDL2 on Android requires:
- JNI interface via SDL_main
- Native app glue library
- Proper activity configuration in AndroidManifest.xml

```cpp
// android_main.cpp
extern "C" void SDL_Android_Init(void);
extern "C" int SDL_main(int argc, char* argv[]) {
    SDL_Android_Init();  // Required on Android
    // ... game initialization
}
```

## Development Workflow

### For Desktop Development

1. **Edit code** in VS Code or your IDE
2. **Build**: `cmake --build build`
3. **Test**: `ctest --test-dir build`
4. **Run**: `./build/minicar`
5. **Debug**: Use VSCode launch configuration (GDB on Linux/macOS, MSVC on Windows)

### For Android Development

1. **Edit code** - Changes in C++ and Java are tracked separately
2. **Sync changes**: Android Studio detects CMakeLists.txt changes automatically
3. **Build**: `./gradlew assembleDebug` or use Android Studio UI
4. **Deploy**: `./gradlew installDebug` or Android Studio "Run"
5. **Debug**: Android Studio Debugger (C++ debugging requires NDK debugger setup)

### Simultaneous Development

For parallel desktop + Android development:

1. Make changes to core game logic (safe on both)
2. Build for both: 
   ```bash
   # Desktop
   cmake --build build
   
   # Android (in a separate terminal)
   ./gradlew assembleDebug
   ```
3. Test on both platforms

## Debugging

### Desktop Debugging

**Linux/macOS (GDB):**
```bash
gdb ./build/minicar
(gdb) run
(gdb) bt  # backtrace on crash
```

**Windows (MSVC):**
- Use Visual Studio debugger or VS Code with MSVC extension

### Android Debugging

**Android Studio:**
1. Connect device or start emulator
2. Click "Debug 'app'" (instead of "Run")
3. Set breakpoints in C++ code
4. IDE will sync C++ code and launch debugger

**Command Line:**
```bash
# Build with debug symbols
./gradlew assembleDebug

# Install and run with debugger
./gradlew installDebug
adb logcat  # View Android logs
```

## Environment Setup

### Linux Development

```bash
sudo apt install build-essential cmake ninja-build \
    libsdl2-dev libsdl2-ttf-dev libcatch2-dev libespeak-ng1

# Android (also install Android Studio + NDK)
```

### macOS Development

```bash
brew install cmake ninja sdl2 sdl2_ttf catch2

# Android (also install Android Studio + NDK)
```

### Windows Development

```powershell
# Using vcpkg
vcpkg install sdl2 sdl2-ttf catch2

# Android (also install Android Studio + NDK)
```

## Performance Considerations

### Desktop
- No frame rate limits (VSync available)
- Reasonable memory constraints
- File I/O can block (use threads if needed)

### Android
- Battery impact (use fixed timestep)
- Limited memory (test on lower-spec devices)
- Network/sensor access via Java/JNI if needed
- Frame rate should match device refresh rate (typically 60 Hz)

## Testing Strategy

### Unit Tests (Desktop Only)
- Use Catch2 framework
- Test game logic in `minicar_core`
- Run: `ctest --test-dir build --output-on-failure`

### Integration Tests
- Manual testing on target platform
- Create simple test scenes in Game

### Manual QA Checklist
- [ ] Desktop: Linux build and run
- [ ] Desktop: macOS build and run
- [ ] Desktop: Windows build and run
- [ ] Android: Build debug APK
- [ ] Android: Install and run on device
- [ ] Android: Test on emulator
- [ ] Android: Test on various screen sizes

## Resources

- [CMake Android Toolchain](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#android)
- [Android NDK Documentation](https://developer.android.com/ndk/guides)
- [SDL2 Android Support](https://wiki.libsdl.org/SDL2/README/android)
- [Android Gradle Plugin Guide](https://developer.android.com/studio/build)


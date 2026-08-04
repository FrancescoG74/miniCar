# Building miniCar for Desktop and Android

This project supports building for both desktop platforms (Linux, macOS, Windows) and Android using CMake and Android Studio.

## Quick Start

### Desktop Build (Linux/macOS/Windows)

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/minicar
```

### Android Build (Android Studio)

1. Open the project in Android Studio
2. Select "Build" → "Make Project" or use the build task in VSCode
3. Deploy to device or emulator using "Run" → "Run 'app'"

## Desktop Build Requirements

### Linux (Debian/Ubuntu)

```bash
sudo apt install build-essential cmake ninja-build \
                 libsdl3-dev libsdl3-ttf-dev \
                 libcatch2-dev \
                 libespeak-ng1
```

### macOS (Homebrew)

```bash
brew install cmake ninja sdl3 sdl3_ttf catch2
```

### Windows (vcpkg)

```powershell
vcpkg install sdl3 sdl3-ttf catch2
```

Then configure with your vcpkg toolchain:

```bash
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE="<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake"
cmake --build build
```

## Android Build Requirements

### Minimum Requirements

- Android Studio 2022.1 or later
- Android NDK (tested with NDK 25.1)
- Android SDK API level 21 or higher (minimum)
- Target SDK API level 33 or higher

### Setup Android SDK & NDK

1. **Install Android Studio**:
   - Download from [Android Developer site](https://developer.android.com/studio)
   - Run the installer

2. **Install NDK**:
   - Open Android Studio
   - Go to: Tools → SDK Manager → SDK Tools
   - Check "NDK (Side by side)"
   - Click "Apply" and "OK"

3. **Set NDK_HOME environment variable** (Windows):
   ```powershell
   $env:ANDROID_NDK_HOME = "C:\Users\<YourUsername>\AppData\Local\Android\Sdk\ndk\25.1.8937393"
   ```

   Or on Linux/macOS:
   ```bash
   export ANDROID_NDK_HOME=$HOME/Android/Sdk/ndk/25.1.8937393
   ```

### Android Build in Android Studio

1. Open the project in Android Studio
2. Select your target device or create an emulator
3. Click "Run" → "Run 'app'" or press Shift+F10

### Android Build from Command Line

```bash
# Debug build
./gradlew assembleDebug

# Release build
./gradlew assembleRelease

# Install on connected device
./gradlew installDebug
```

## VSCode Configuration

### For Desktop Development

1. Install extensions:
   - C/C++ (ms-vscode.cpptools)
   - CMake Tools (ms-vscode.cmake-tools)
   - CMake (twxs.cmake)

2. Open the workspace:
   ```bash
   code .
   ```

3. VSCode will detect CMakeLists.txt and configure automatically

4. Build tasks available:
   - "Build Desktop (Linux/macOS/Windows)"
   - "Build Desktop and Run"
   - "Run Tests (Desktop)"

### For Android Development

1. Install the Gradle for Java extension
2. Android build tasks in VSCode:
   - "Build Android (Debug)"
   - "Build Android (Release)"

## Project Structure

```
miniCar/
├── CMakeLists.txt                 # Desktop & Android CMake build
├── build.gradle.kts               # Android Gradle build config
├── settings.gradle.kts            # Gradle project settings
├── gradle.properties              # Gradle properties
├── gradle/                        # Gradle wrapper
├── proguard-rules.pro            # ProGuard rules for Android
├── include/                       # C++ headers
│   ├── actor/
│   ├── audio/
│   ├── game/
│   ├── input/
│   └── ...
├── src/
│   ├── main.cpp                  # Desktop entry point
│   ├── android_main.cpp          # Android entry point
│   ├── actor/
│   ├── audio/
│   ├── game/
│   ├── input/
│   └── main/                     # Android-specific resources
│       ├── AndroidManifest.xml
│       ├── kotlin/
│       └── res/
├── tests/
│   └── ...                        # Unit tests (desktop only)
└── .vscode/                       # VSCode configuration
    ├── settings.json
    ├── tasks.json
    ├── launch.json
    └── c_cpp_properties.json
```

## Build Configurations

### CMake Options

- `MINICAR_BUILD_TESTS` (ON/OFF) - Build unit tests (default: ON)
  - Automatically disabled for Android builds

### Android Gradle Configuration

Edit `build.gradle.kts` to modify:
- `compileSdk` - Target SDK version
- `minSdk` - Minimum SDK version
- `ndkVersion` - Android NDK version
- `cppFlags` - C++ compiler flags

## Troubleshooting

### Android Build Issues

**NDK Not Found:**
```
Error: Could not find NDK
```
Solution: Set `ANDROID_NDK_HOME` environment variable or configure in `local.properties`:
```properties
ndk.dir=<path-to-android-sdk>/ndk/30.0.15729638
```

**SDL3 Not Found:**
```
Error: Could not find SDL3
```
Solution: The Android NDK toolchain will fetch SDL3 automatically. Ensure you have a valid NDK installation.

**Gradle Build Fails:**
- Clean the project: `./gradlew clean`
- Rebuild: `./gradlew assembleDebug`

### Desktop Build Issues

**CMake Not Found:**
```bash
sudo apt install cmake  # Linux
brew install cmake      # macOS
choco install cmake     # Windows (with Chocolatey)
```

**SDL3 Headers Not Found:**
- Ensure SDL3 dev packages are installed
- On Linux: `sudo apt install libsdl3-dev libsdl3-ttf-dev`
- On macOS: `brew install sdl3 sdl3_ttf`

## Testing

### Desktop Tests

```bash
# Build and run tests
ctest --test-dir build --output-on-failure

# Or manually run test binary
./build/tests/minicar_tests
```

Tests are not available on Android builds (headless only).

## Platform-Specific Notes

### Android

- Entry point: `android_main.cpp` (JNI interface)
- Landscape orientation enforced
- No espeak-ng support (uses beep tones only)
- Minimum SDK 21, target SDK 33
- Touch controls not yet implemented (keyboard input only)

### Desktop

- Entry point: `main.cpp`
- Window size: 1600×900 (configurable)
- Full feature support including optional espeak-ng
- Keyboard controls: WASD (Player 1), IJKL (Player 2)
- Tests: Full unit test suite with Catch2

## Continuous Integration

For CI/CD pipeline support, see:
- `.github/workflows/` (if added for GitHub Actions)
- Build scripts can be adapted from VSCode tasks

## Contributing

When adding new platform-specific code:
1. Use `#ifdef ANDROID` guards
2. Keep core logic in `minicar_core` library
3. Platform-specific code in separate source files
4. Update both CMakeLists.txt and build.gradle.kts

## References

- [CMake Android Documentation](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#cross-compiling-for-android)
- [Android NDK Documentation](https://developer.android.com/ndk/guides)
- [SDL3 Android Setup](https://wiki.libsdl.org/SDL3/README/android)
- [Gradle Android Plugin](https://developer.android.com/studio/build)


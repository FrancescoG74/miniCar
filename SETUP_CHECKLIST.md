# miniCar Cross-Platform Setup Checklist

## ✅ Pre-Setup Checklist

Before you start, ensure you have:
- [ ] Git installed
- [ ] Administrator/sudo access (for dependency installation)
- [ ] At least 5 GB free disk space

## 🖥️ Desktop Development Setup (Pick Your OS)

### Linux (Ubuntu/Debian)
- [ ] Run: `bash setup.sh`
- [ ] Or manually:
  ```bash
  sudo apt install build-essential cmake ninja-build \
      libsdl2-dev libsdl2-ttf-dev libcatch2-dev libespeak-ng1
  ```
- [ ] Verify: `cmake --version`, `ninja --version`

### macOS
- [ ] Install Homebrew (if not already): `/bin/bash -c "$(curl -fsSL ...)"`
- [ ] Run: `bash setup.sh`
- [ ] Or manually:
  ```bash
  brew install cmake ninja sdl2 sdl2_ttf catch2
  ```
- [ ] Verify: `cmake --version`, `ninja --version`

### Windows
- [ ] Run: `setup.bat`
- [ ] Or manually:
  ```powershell
  # Using Chocolatey
  choco install cmake ninja
  
  # For SDL2, use vcpkg:
  git clone https://github.com/microsoft/vcpkg
  .\vcpkg\bootstrap-vcpkg.bat
  .\vcpkg\vcpkg install sdl2 sdl2-ttf catch2
  ```

## 📱 Android Development Setup

### Prerequisites
- [ ] Android Studio installed
- [ ] 8+ GB RAM recommended

### SDK & NDK Setup
1. [ ] Open Android Studio
2. [ ] Go to: Tools → SDK Manager
3. [ ] Install:
   - [ ] Android SDK API Level 33 or higher
   - [ ] NDK (Side by side) - Version 25.1 or compatible
4. [ ] Wait for downloads to complete

### Environment Configuration
1. [ ] Set `ANDROID_NDK_HOME` environment variable:
   
   **Windows:**
   ```powershell
   $env:ANDROID_NDK_HOME = "C:\Users\<YourUsername>\AppData\Local\Android\Sdk\ndk\25.1.8937393"
   ```
   
   **Linux/macOS:**
   ```bash
   export ANDROID_NDK_HOME=$HOME/Android/Sdk/ndk/25.1.8937393
   ```

2. [ ] (Optional) Add to `.bashrc`, `.zshrc`, or System Variables for persistence

3. [ ] Verify: 
   ```bash
   echo $ANDROID_NDK_HOME  # Unix-like
   echo %ANDROID_NDK_HOME% # Windows
   ```

### Project Configuration
- [ ] Update `local.properties` with your SDK/NDK paths
- [ ] Verify paths in file match your installation

## 🏗️ Building the Project

### Desktop Build
```bash
# Quick setup
bash setup.sh  # Unix-like
setup.bat      # Windows

# Manual build
cmake -S . -B build -G Ninja
cmake --build build

# Run
./build/minicar  # Unix-like
build\minicar.exe # Windows
```

### Android Build
**Using Android Studio:**
1. [ ] Open project in Android Studio
2. [ ] Wait for Gradle sync
3. [ ] Connect device or create emulator
4. [ ] Click "Run" or press Shift+F10
5. [ ] Select target device
6. [ ] Wait for build and deploy

**Using Command Line:**
```bash
# Debug build
./gradlew assembleDebug

# Release build  
./gradlew assembleRelease

# Install on device
./gradlew installDebug

# Run on device
adb shell am start -n com.example.minicar/.MainActivity
```

## 🔍 Verification Steps

### Desktop Verification
- [ ] Build completes without errors
- [ ] Executable created in `build/` directory
- [ ] Game launches: `./build/minicar`
- [ ] Game window opens with game running
- [ ] Keyboard controls work (WASD, IJKL)
- [ ] Game exits cleanly

### Android Verification
- [ ] APK build completes (`build/outputs/apk/debug/`)
- [ ] APK installs on device/emulator
- [ ] App launches on Android device
- [ ] Game renders correctly
- [ ] Hardware buttons work (or emulator soft keys)
- [ ] App closes without crashing

### Test Verification (Desktop Only)
```bash
ctest --test-dir build --output-on-failure
```
- [ ] All tests pass
- [ ] Output shows test summary

## 📚 Documentation Quick Links

| Document | Purpose |
|----------|---------|
| [BUILD_GUIDE.md](BUILD_GUIDE.md) | Detailed platform-specific build instructions |
| [PLATFORM_NOTES.md](PLATFORM_NOTES.md) | Development guide for cross-platform code |
| [SETUP_SUMMARY.md](SETUP_SUMMARY.md) | What was changed for cross-platform support |
| [README.md](README.md) | General project information |

## 🔧 IDE Setup

### VS Code Setup
1. [ ] Install extensions:
   - [ ] C/C++ (ms-vscode.cpptools)
   - [ ] CMake Tools (ms-vscode.cmake-tools)
   - [ ] CMake (twxs.cmake)
   - [ ] Gradle for Java (vscjava.vscode-gradle)

2. [ ] Open workspace: `code .`

3. [ ] Verify:
   - [ ] VS Code recognizes CMakeLists.txt
   - [ ] Build tasks appear in Command Palette
   - [ ] IntelliSense works for C++

### Android Studio Setup
1. [ ] Open project
2. [ ] Wait for Gradle sync
3. [ ] Verify:
   - [ ] No CMake errors in Event Log
   - [ ] Project Structure shows correct SDK/NDK
   - [ ] Build configuration selected

## 🐛 Troubleshooting

### CMake Issues
- [ ] Clear build directory: `rm -rf build` (Unix) or `rmdir /s build` (Windows)
- [ ] Reconfigure: `cmake -S . -B build -G Ninja`
- [ ] Check for correct SDL2 installation

### Gradle Issues
- [ ] Clean: `./gradlew clean`
- [ ] Check `local.properties` paths
- [ ] Verify NDK installation
- [ ] Try: `./gradlew assembleDebug --info`

### Build Issues
- [ ] Check compiler version: `g++ --version`, `clang++ --version`
- [ ] Verify C++17 support
- [ ] Check all dependencies are installed
- [ ] Review error messages carefully

## 📋 Common Commands Reference

### Desktop (CMake)
```bash
# Configure
cmake -S . -B build -G Ninja

# Build
cmake --build build

# Build specific target
cmake --build build --target minicar

# Run tests
ctest --test-dir build --output-on-failure

# Clean
rm -rf build

# Build with debug info
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

### Android (Gradle)
```bash
# Debug build
./gradlew assembleDebug

# Release build
./gradlew assembleRelease

# Clean
./gradlew clean

# Install APK
./gradlew installDebug

# View logs
adb logcat | grep miniCar

# Run on device
adb shell am start -n com.example.minicar/.MainActivity
```

## ✨ First-Time Developer Workflow

1. [ ] Clone/Open project
2. [ ] Run setup script (`setup.sh` or `setup.bat`)
3. [ ] Build desktop version
4. [ ] Run desktop version and verify it works
5. [ ] (Optional) Install Android Studio and NDK
6. [ ] Build Android version
7. [ ] Deploy Android version to device/emulator
8. [ ] Make code changes
9. [ ] Rebuild and test both platforms

## 🎯 Success Criteria

You're ready to develop when:
- ✅ Desktop version builds and runs
- ✅ Game window appears on desktop
- ✅ WASD controls work for Player 1
- ✅ (Android only) Android version builds without errors
- ✅ (Android only) APK installs on device/emulator
- ✅ (Android only) Game runs on Android

## 📞 Getting Help

1. Check the [BUILD_GUIDE.md](BUILD_GUIDE.md) troubleshooting section
2. Review [PLATFORM_NOTES.md](PLATFORM_NOTES.md) for your specific issue
3. Check CMake or Gradle build output for specific error messages
4. Verify all dependencies are installed: `setup.sh` or `setup.bat`

## 🚀 Next Steps

Once setup is complete:
1. [ ] Read [PLATFORM_NOTES.md](PLATFORM_NOTES.md) for development guidance
2. [ ] Explore the codebase structure
3. [ ] Run existing tests
4. [ ] Make a small code change and verify it builds
5. [ ] Start game development!

---

**Last Updated:** 2026-07-31
**Supported Platforms:** Linux, macOS, Windows, Android


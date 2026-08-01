# miniCar Documentation Index

Welcome to miniCar - a cross-platform C++ racing game for Desktop and Android!

## 🚀 Getting Started

### For New Developers
1. **Start here:** [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) - Step-by-step setup guide
2. **Run setup script:**
   - Linux/macOS: `bash setup.sh`
   - Windows: `setup.bat`

### For Building the Game
- **Desktop:** [BUILD_GUIDE.md](BUILD_GUIDE.md) - Desktop build instructions
- **Android:** [BUILD_GUIDE.md](BUILD_GUIDE.md) - Android build instructions
- **Both:** `CMakeLists.txt` and `build.gradle.kts` define build configuration

## 📖 Documentation Files

### Setup & Configuration
| File | Purpose | Audience |
|------|---------|----------|
| [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) | Step-by-step setup guide | Developers |
| [BUILD_GUIDE.md](BUILD_GUIDE.md) | Detailed build instructions | Developers |
| [SETUP_SUMMARY.md](SETUP_SUMMARY.md) | Summary of changes made | Project leads |
| [PLATFORM_NOTES.md](PLATFORM_NOTES.md) | Cross-platform development guide | Developers |

### Project Information
| File | Purpose | Audience |
|------|---------|----------|
| [README.md](README.md) | Project overview | Everyone |
| [CMakeLists.txt](CMakeLists.txt) | Desktop & Android build config | Developers |
| [build.gradle.kts](build.gradle.kts) | Android Gradle config | Developers |

### IDE & Editor Configuration
| File | Purpose | Platform |
|------|---------|----------|
| [.vscode/settings.json](.vscode/settings.json) | VS Code settings | VS Code |
| [.vscode/tasks.json](.vscode/tasks.json) | Build tasks | VS Code |
| [.vscode/launch.json](.vscode/launch.json) | Debug config | VS Code |
| [.vscode/c_cpp_properties.json](.vscode/c_cpp_properties.json) | IntelliSense config | VS Code |
| [CMakePresets.json](CMakePresets.json) | CMake presets | CMake Tools |

### Scripts & Configuration
| File | Purpose | Platform |
|------|---------|----------|
| [setup.sh](setup.sh) | Automated setup | Unix-like |
| [setup.bat](setup.bat) | Automated setup | Windows |
| [gradlew](gradlew) | Gradle wrapper | Unix-like |
| [gradlew.bat](gradlew.bat) | Gradle wrapper | Windows |
| [gradle.properties](gradle.properties) | Gradle settings | Android |
| [local.properties](local.properties) | Local SDK/NDK paths | Android |

### Android-Specific Files
| File | Purpose |
|------|---------|
| [src/main/AndroidManifest.xml](src/main/AndroidManifest.xml) | Android app manifest |
| [src/main/kotlin/com/example/minicar/MainActivity.kt](src/main/kotlin/com/example/minicar/MainActivity.kt) | Android activity |
| [src/main/res/values/strings.xml](src/main/res/values/strings.xml) | String resources |
| [src/main/res/values/colors.xml](src/main/res/values/colors.xml) | Color resources |
| [src/main/res/values/themes.xml](src/main/res/values/themes.xml) | Theme resources |
| [proguard-rules.pro](proguard-rules.pro) | ProGuard rules |

### Source Code Files
| File | Purpose |
|------|---------|
| [src/main.cpp](src/main.cpp) | Desktop entry point |
| [src/android_main.cpp](src/android_main.cpp) | Android entry point (JNI) |

## 🎯 Quick Reference

### Build Commands

#### Desktop
```bash
# Setup (runs automatically)
bash setup.sh           # Linux/macOS
setup.bat              # Windows

# Manual build
cmake -S . -B build -G Ninja
cmake --build build
./build/minicar        # Run

# Tests
ctest --test-dir build --output-on-failure
```

#### Android
```bash
# Using Gradle
./gradlew assembleDebug    # Build debug APK
./gradlew assembleRelease  # Build release APK
./gradlew installDebug     # Install on device

# Using Android Studio
# Open project → Select device → Run (Shift+F10)
```

### VS Code Build Tasks
| Task | Function |
|------|----------|
| Build Desktop | Configure and build for desktop |
| Build Desktop and Run | Build and run the game |
| Run Tests | Build and run unit tests |
| Build Android (Debug) | Build Android debug APK |
| Build Android (Release) | Build Android release APK |

### Common Issues & Solutions

#### Build fails on Linux
→ See [BUILD_GUIDE.md](BUILD_GUIDE.md#troubleshooting) Troubleshooting section

#### Android NDK not found
→ Check [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) NDK setup section

#### CMake configuration error
→ See [BUILD_GUIDE.md](BUILD_GUIDE.md#troubleshooting) CMake section

## 📱 Platform Support

### Desktop (Linux/macOS/Windows)
- ✅ Full desktop build support
- ✅ CMake configuration
- ✅ Unit tests with Catch2
- ✅ Optional espeak-ng support
- ✅ VS Code integration

### Android
- ✅ Full Android build support
- ✅ Gradle configuration
- ✅ Android Studio integration
- ✅ NDK C++ compilation
- ✅ Landscape orientation
- ⚠️ Tests not supported (headless only)
- ⚠️ Beep tones only (no espeak-ng)

## 🏗️ Project Structure

```
miniCar/
├── CMakeLists.txt                 # Desktop & Android build config
├── build.gradle.kts               # Android Gradle config
├── settings.gradle.kts            # Gradle settings
├── gradle.properties              # Gradle properties
├── gradle/                        # Gradle wrapper
├── local.properties               # SDK/NDK paths
│
├── src/
│   ├── main.cpp                  # Desktop entry point
│   ├── android_main.cpp          # Android entry point (NEW)
│   ├── main/                     # Android resources (NEW)
│   │   ├── AndroidManifest.xml
│   │   ├── kotlin/
│   │   └── res/
│   ├── actor/
│   ├── audio/
│   ├── game/
│   ├── input/
│   └── ...
│
├── include/                       # C++ headers
│   ├── Setup.h
│   ├── Game.h
│   └── ...
│
├── tests/                        # Unit tests (desktop only)
│
├── .vscode/                      # VS Code config (NEW)
│   ├── settings.json
│   ├── tasks.json
│   ├── launch.json
│   └── c_cpp_properties.json
│
├── Documentation/
│   ├── README.md                 # Project overview
│   ├── BUILD_GUIDE.md            # Build instructions (NEW)
│   ├── PLATFORM_NOTES.md         # Dev guide (NEW)
│   ├── SETUP_CHECKLIST.md        # Setup checklist (NEW)
│   ├── SETUP_SUMMARY.md          # Changes summary (NEW)
│   ├── INDEX.md                  # This file (NEW)
│   └── CMakePresets.json         # CMake presets (NEW)
│
└── Setup Scripts/
    ├── setup.sh                  # Unix setup (NEW)
    └── setup.bat                 # Windows setup (NEW)
```

## 📚 Reading Guide by Role

### Project Manager
1. [README.md](README.md) - Project overview
2. [SETUP_SUMMARY.md](SETUP_SUMMARY.md) - What changed
3. [BUILD_GUIDE.md](BUILD_GUIDE.md) - Build requirements

### Developer (New to Project)
1. [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md) - Get started
2. [BUILD_GUIDE.md](BUILD_GUIDE.md) - Build the project
3. [PLATFORM_NOTES.md](PLATFORM_NOTES.md) - Understand architecture
4. [README.md](README.md) - Project details

### Developer (Adding Features)
1. [PLATFORM_NOTES.md](PLATFORM_NOTES.md) - Platform architecture
2. [BUILD_GUIDE.md](BUILD_GUIDE.md) - Build procedures
3. Source code files in `src/` and `include/`

### DevOps/Build Engineer
1. [BUILD_GUIDE.md](BUILD_GUIDE.md) - Build procedures
2. [CMakeLists.txt](CMakeLists.txt) - Desktop build config
3. [build.gradle.kts](build.gradle.kts) - Android build config
4. [PLATFORM_NOTES.md](PLATFORM_NOTES.md) - Platform differences

## 🔗 External Resources

### CMake & Build
- [CMake Documentation](https://cmake.org/cmake/help/latest/)
- [CMake Android Toolchain](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html#android)
- [Ninja Build System](https://ninja-build.org/)

### SDL2
- [SDL2 Documentation](https://wiki.libsdl.org/SDL2)
- [SDL2 Android Setup](https://wiki.libsdl.org/SDL2/README/android)

### Android Development
- [Android Developer Docs](https://developer.android.com/docs)
- [Android NDK Guide](https://developer.android.com/ndk/guides)
- [Android Gradle Plugin Guide](https://developer.android.com/studio/build)

### IDE Documentation
- [VS Code C/C++ Extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [Android Studio Help](https://developer.android.com/studio/intro)

## 📞 Support & Troubleshooting

### First Steps
1. Run `setup.sh` (Unix) or `setup.bat` (Windows)
2. Check [SETUP_CHECKLIST.md](SETUP_CHECKLIST.md)
3. Review [BUILD_GUIDE.md](BUILD_GUIDE.md) troubleshooting

### For Specific Issues
- **CMake errors:** [BUILD_GUIDE.md](BUILD_GUIDE.md#troubleshooting)
- **Gradle errors:** [BUILD_GUIDE.md](BUILD_GUIDE.md#troubleshooting)
- **Platform-specific:** [PLATFORM_NOTES.md](PLATFORM_NOTES.md)
- **Architecture questions:** [PLATFORM_NOTES.md](PLATFORM_NOTES.md#architecture-overview)

### When Stuck
1. Check the error message carefully
2. Search in relevant documentation
3. Review similar sections in existing code
4. Check build output for specific details

## ✅ File Checklist

New files added for cross-platform support:
- [x] CMakeLists.txt (modified)
- [x] build.gradle.kts
- [x] settings.gradle.kts
- [x] gradle.properties
- [x] gradle/wrapper/gradle-wrapper.properties
- [x] gradlew
- [x] gradlew.bat
- [x] local.properties
- [x] proguard-rules.pro
- [x] src/android_main.cpp
- [x] src/main/AndroidManifest.xml
- [x] src/main/kotlin/com/example/minicar/MainActivity.kt
- [x] src/main/res/values/strings.xml
- [x] src/main/res/values/colors.xml
- [x] src/main/res/values/themes.xml
- [x] .vscode/settings.json
- [x] .vscode/tasks.json
- [x] .vscode/launch.json
- [x] .vscode/c_cpp_properties.json
- [x] CMakePresets.json
- [x] setup.sh
- [x] setup.bat
- [x] BUILD_GUIDE.md
- [x] PLATFORM_NOTES.md
- [x] SETUP_CHECKLIST.md
- [x] SETUP_SUMMARY.md
- [x] INDEX.md (this file)

---

**Last Updated:** 2026-07-31  
**Status:** ✅ Ready for cross-platform development  
**Supported Platforms:** Linux, macOS, Windows, Android


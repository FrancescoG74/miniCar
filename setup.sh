#!/bin/bash
# Setup script for miniCar development environment (Linux/macOS)

set -e

echo "================================================"
echo "  miniCar Development Environment Setup"
echo "================================================"
echo ""

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    echo "❌ Unsupported OS: $OSTYPE"
    exit 1
fi

echo "✓ Detected OS: $OS"
echo ""

# Check for required tools
check_command() {
    if command -v $1 &> /dev/null; then
        VERSION=$($1 --version 2>&1 | head -n1)
        echo "✓ $1: $VERSION"
        return 0
    else
        echo "❌ $1: NOT FOUND"
        return 1
    fi
}

echo "Checking for required tools..."
MISSING=0

check_command cmake || MISSING=1
check_command git || MISSING=1
check_command ninja || MISSING=1

echo ""

if [ $MISSING -eq 1 ]; then
    echo "Installing missing dependencies..."
    echo ""

    if [ "$OS" == "linux" ]; then
        echo "Installing for Debian/Ubuntu..."
        sudo apt update
        sudo apt install -y build-essential cmake ninja-build \
                          libsdl2-dev libsdl2-ttf-dev \
                          libcatch2-dev \
                          libespeak-ng1
        echo "✓ Dependencies installed"
    elif [ "$OS" == "macos" ]; then
        echo "Installing for macOS (using Homebrew)..."
        if ! command -v brew &> /dev/null; then
            echo "Installing Homebrew..."
            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        fi
        brew install cmake ninja sdl2 sdl2_ttf catch2
        echo "✓ Dependencies installed"
    fi
else
    echo "✓ All required tools found"
fi

echo ""
echo "================================================"
echo "  Optional: Android Development Setup"
echo "================================================"
echo ""

read -p "Do you want to set up Android development? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Android setup steps:"
    echo "1. Download Android Studio from https://developer.android.com/studio"
    echo "2. Install Android Studio"
    echo "3. Open SDK Manager (Tools → SDK Manager)"
    echo "4. Install:"
    echo "   - Android SDK API 33 (or higher)"
    echo "   - NDK (Side by side) - version 25.1 or compatible"
    echo "5. Set ANDROID_NDK_HOME environment variable:"

    if [ "$OS" == "linux" ]; then
        echo "   export ANDROID_NDK_HOME=\$HOME/Android/Sdk/ndk/25.1.8937393"
    elif [ "$OS" == "macos" ]; then
        echo "   export ANDROID_NDK_HOME=\$HOME/Library/Android/sdk/ndk/25.1.8937393"
    fi

    echo ""
    echo "6. Add to ~/.bashrc or ~/.zshrc if needed"
fi

echo ""
echo "================================================"
echo "  Building miniCar"
echo "================================================"
echo ""

# Build desktop version
if [ -d "build" ]; then
    read -p "Build directory exists. Clean it? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf build
    fi
fi

echo "Configuring CMake..."
cmake -S . -B build -G Ninja

echo ""
echo "Building miniCar..."
cmake --build build

echo ""
echo "✓ Build successful!"
echo ""
echo "To run the game:"
echo "  ./build/minicar"
echo ""

# Test if Catch2 is available for tests
if cmake --help 2>&1 | grep -q "Catch"; then
    read -p "Run unit tests? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        ctest --test-dir build --output-on-failure
    fi
fi

echo ""
echo "================================================"
echo "  Setup Complete!"
echo "================================================"
echo ""
echo "Next steps:"
echo "1. Desktop: ./build/minicar"
echo "2. Android: Open in Android Studio and run"
echo "3. VSCode: Open folder and use build tasks"
echo ""
echo "Documentation:"
echo "  - BUILD_GUIDE.md     - Detailed build instructions"
echo "  - PLATFORM_NOTES.md  - Platform-specific development guide"
echo "  - README.md          - General project information"
echo ""


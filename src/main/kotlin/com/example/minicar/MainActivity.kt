package com.example.minicar

import android.app.Activity
import android.os.Bundle
import org.libsdl.app.SDLActivity

/**
 * MainActivity for miniCar Android application.
 * 
 * This class extends SDLActivity from the SDL3 Android library, which handles:
 * - Loading the native library
 * - Setting up the SDL environment
 * - Managing the game lifecycle
 * 
 * The actual game logic is implemented in C++ using the minicar core library.
 */
class MainActivity : SDLActivity() {
    companion object {
        // Library name - must match the CMakeLists.txt project name
        init {
            // SDL3 handles loading the native library automatically through JNI
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // The native C++ code (SDL_main/android_main) will handle the game loop
    }
}


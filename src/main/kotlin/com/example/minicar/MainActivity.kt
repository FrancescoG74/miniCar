package com.example.minicar

import android.app.Activity
import android.os.Bundle
import android.view.View
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
        hideSystemUi()
        // The native C++ code (SDL_main/android_main) will handle the game loop
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            hideSystemUi()
        }
    }

    /**
     * Hides the status bar and navigation bar so the game renders truly
     * full-screen. Without this, the on-screen navigation bar overlaps the
     * bottom of the screen and hides/blocks the virtual touch controller.
     */
    private fun hideSystemUi() {
        @Suppress("DEPRECATION")
        window.decorView.systemUiVisibility = (
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            )
    }
}


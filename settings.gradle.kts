pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
    // ADD THIS BLOCK:
    plugins {
        id("com.android.application") version "8.13.2" // Use a version compatible with your Android Studio
        id("com.android.library") version "8.13.2"
        id("org.jetbrains.kotlin.android") version "1.8.10"
    }
}
plugins {
    id("org.gradle.toolchains.foojay-resolver-convention") version "0.10.0"
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "miniCar"

// For simpler structure, include the root project as the app
// If you want a separate app/ subdirectory, change this to: include(":app")



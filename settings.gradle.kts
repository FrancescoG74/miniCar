pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
    // ADD THIS BLOCK:
    plugins {
        id("com.android.application") version "9.4.1" // Use a version compatible with your Android Studio
        id("com.android.library") version "9.4.1"
        id("org.jetbrains.kotlin.android") version "2.0.21"
    }
}
plugins {
    id("org.gradle.toolchains.foojay-resolver-convention") version "1.0.0"
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



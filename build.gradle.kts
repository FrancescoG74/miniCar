plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.example.minicar"
    compileSdk = 33
    defaultConfig {
        applicationId = "com.example.minicar"
        minSdk = 29
        targetSdk = 33
        versionCode = 1
        versionName = "1.0"

        ndk {
            // Specify the Android NDK version to use
            // Adjust this to match your NDK version
            ndkVersion = "30.0.15729638"
        }

        // Configure CMake
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DANDROID_PLATFORM=android-29"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    kotlinOptions {
        jvmTarget = "11"
    }
    buildFeatures {
        prefab = true
    }
}

dependencies {
    // AndroidX dependencies
    implementation("androidx.appcompat:appcompat:1.6.1")
    implementation("androidx.constraintlayout:constraintlayout:2.1.4")
    
    // Required for native development with CMake
    implementation(files("libs/SDL3-3.4.12.aar"))
    implementation(files("libs/SDL3_ttf-3.2.2.aar"))   // Add this
    implementation(files("libs/SDL3_image-3.4.4.aar")) // Add this

}


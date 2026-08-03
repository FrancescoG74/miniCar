cmake_minimum_required(VERSION 3.16)
project(minicar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MINICAR_BUILD_TESTS "Build the miniCar unit test suite" ON)
option(MINICAR_ENABLE_WARNINGS "Enable compiler warnings" ON)

# --- HELPER FUNCTION ---
function(minicar_configure_quality target)
    if(MINICAR_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /W4)
        else()
            target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
endfunction()

# --- SDL3 FIND LOGIC (Cross-Platform) ---

# 1. Main SDL3
# Try Config mode first (works for Android Prefab/AAR and Windows installations)
find_package(SDL3 QUIET CONFIG)
if(NOT SDL3_FOUND)
    # Fallback for Linux system packages
    find_package(SDL3 REQUIRED)
endif()

# 2. SDL3_ttf
find_package(SDL3_ttf QUIET CONFIG)
if(NOT SDL3_ttf_FOUND)
    find_package(SDL3_ttf QUIET)
endif()
if(NOT SDL3_ttf_FOUND AND NOT MSVC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3_TTF REQUIRED sdl3-ttf)
endif()

# 3. SDL3_image
find_package(SDL3_image QUIET CONFIG)
if(NOT SDL3_image_FOUND)
    find_package(SDL3_image QUIET)
endif()
if(NOT SDL3_image_FOUND AND NOT MSVC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3_IMAGE REQUIRED sdl3-image)
endif()

# --- CORE LIBRARY ---
add_library(minicar_core STATIC
        src/Track.cpp
        src/CollisionSystem.cpp
        src/Sprite.cpp
        src/actor/Actor.cpp
        src/actor/Car.cpp
        src/actor/Rock.cpp
        src/actor/Gate.cpp
        src/actor/StartLine.cpp
        src/input/KeyboardInput.cpp
        src/input/AiInput.cpp
        src/game/DriverAssignmentService.cpp
        src/game/RaceBuilder.cpp
        src/game/RaceSession.cpp
)

target_include_directories(minicar_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
minicar_configure_quality(minicar_core)

# Link SDL3 Core
if(TARGET SDL3::SDL3)
    target_link_libraries(minicar_core PUBLIC SDL3::SDL3)
elseif(TARGET SDL3::SDL3-shared)
    target_link_libraries(minicar_core PUBLIC SDL3::SDL3-shared)
else()
    target_link_libraries(minicar_core PUBLIC ${SDL3_LIBRARIES})
endif()

# --- MAIN EXECUTABLE ---
add_executable(minicar
        src/main.cpp
        src/Audio.cpp
        src/Voice.cpp
        src/Setup.cpp
        src/audio/Announcer.cpp
        src/game/Game.cpp
        src/game/GamePhases.cpp
        src/game/WorldRenderer.cpp
        src/game/HudRenderer.cpp
)

target_include_directories(minicar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Include directories for TTF/Image (mostly for Linux fallback)
if(SDL3_TTF_INCLUDE_DIRS)
    target_include_directories(minicar PRIVATE ${SDL3_TTF_INCLUDE_DIRS})
endif()
if(SDL3_IMAGE_INCLUDE_DIRS)
    target_include_directories(minicar PRIVATE ${SDL3_IMAGE_INCLUDE_DIRS})
endif()

target_link_libraries(minicar PRIVATE minicar_core)
minicar_configure_quality(minicar)

# --- LINKING ADDITIONAL LIBS ---

# TTF
if(TARGET SDL3_ttf::SDL3_ttf)
    target_link_libraries(minicar PRIVATE SDL3_ttf::SDL3_ttf)
elseif(SDL3_TTF_LIBRARIES)
    target_link_libraries(minicar PRIVATE ${SDL3_TTF_LIBRARIES})
endif()

# Image
if(TARGET SDL3_image::SDL3_image)
    target_link_libraries(minicar PRIVATE SDL3_image::SDL3_image)
elseif(SDL3_IMAGE_LIBRARIES)
    target_link_libraries(minicar PRIVATE ${SDL3_IMAGE_LIBRARIES})
endif()

# --- ESPEAK (Linux Only) ---
if(NOT WIN32 AND NOT ANDROID)
    find_library(ESPEAK_NG_LIBRARY NAMES espeak-ng)
    if(ESPEAK_NG_LIBRARY)
        target_compile_definitions(minicar PRIVATE MINICAR_HAVE_ESPEAK=1)
        target_link_libraries(minicar PRIVATE ${ESPEAK_NG_LIBRARY})
    endif()
endif()

target_compile_definitions(minicar PRIVATE MINICAR_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

if(MINICAR_BUILD_TESTS)
    include(CTest)
    enable_testing()
    # add_subdirectory(tests) # Ensure this exists
endif()
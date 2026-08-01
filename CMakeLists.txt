cmake_minimum_required(VERSION 3.16)
project(minicar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MINICAR_BUILD_TESTS "Build the miniCar unit test suite" ON)
option(MINICAR_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(MINICAR_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MINICAR_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# --- SDL3 CONFIGURATION (PREFAB) ---
# This looks for the AAR you added to your libs folder
find_package(SDL3 REQUIRED CONFIG)

function(minicar_configure_quality target)
    if(MINICAR_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(${target} PRIVATE /W4)
            if(MINICAR_WARNINGS_AS_ERRORS)
                target_compile_options(${target} PRIVATE /WX)
            endif()
        else()
            target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
            if(MINICAR_WARNINGS_AS_ERRORS)
                target_compile_options(${target} PRIVATE -Werror)
            endif()
        endif()
    endif()

    if(MINICAR_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endfunction()

find_package(SDL3 REQUIRED)

# Try find_package first, fall back to pkg-config for SDL3_ttf
find_package(SDL3_ttf QUIET)
if(NOT SDL3_ttf_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3_TTF REQUIRED sdl3-ttf)
    set(SDL3_ttf_FOUND TRUE)
    set(SDL3_TTF_INCLUDE_DIRS ${SDL3_TTF_INCLUDE_DIRS})
    set(SDL3_TTF_LIBRARIES ${SDL3_TTF_LIBRARIES})
endif()

# Try find_package first, fall back to pkg-config for SDL3_image
find_package(SDL3_image QUIET)
if(NOT SDL3_image_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3_IMAGE REQUIRED sdl3-image)
    set(SDL3_image_FOUND TRUE)
    set(SDL3_IMAGE_INCLUDE_DIRS ${SDL3_IMAGE_INCLUDE_DIRS})
    set(SDL3_IMAGE_LIBRARIES ${SDL3_IMAGE_LIBRARIES})
endif()

# Pure gameplay logic: track math, physics, collisions, driver strategies and
# the race domain model. Deliberately excludes anything that touches audio
# devices, fonts or window/renderer creation, so it can be linked into the
# unit test binary with no display/audio device required.
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

target_include_directories(minicar_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
minicar_configure_quality(minicar_core)

if(TARGET SDL3::SDL3)
    target_link_libraries(minicar_core PUBLIC SDL3::SDL3)
else()
    target_link_libraries(minicar_core PUBLIC ${SDL3_LIBRARIES})
endif()

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

target_include_directories(minicar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL3_TTF_INCLUDE_DIRS}
    ${SDL3_IMAGE_INCLUDE_DIRS}
    /usr/include/SDL3_ttf
    /usr/include/SDL3_image
)

target_link_libraries(minicar PRIVATE minicar_core)
minicar_configure_quality(minicar)

# Optional: spoken-word countdown ("3", "2", "1", "Go") via espeak-ng, if its
# runtime library can be found. Only the -dev package ships an unversioned
# libespeak-ng.so + headers, so this also looks for the bare versioned runtime
# lib (e.g. Debian/Ubuntu without espeak-ng-dev installed). When not found, the
# game still builds and falls back to simple beep tones (see UiSound).
find_library(ESPEAK_NG_LIBRARY NAMES espeak-ng)
if(NOT ESPEAK_NG_LIBRARY)
    file(GLOB _espeak_ng_versioned_candidates
        /usr/lib/*/libespeak-ng.so.1
        /usr/lib/libespeak-ng.so.1
        /usr/lib64/libespeak-ng.so.1
        /usr/local/lib/libespeak-ng.so.1
    )
    if(_espeak_ng_versioned_candidates)
        list(GET _espeak_ng_versioned_candidates 0 ESPEAK_NG_LIBRARY)
    endif()
endif()

if(ESPEAK_NG_LIBRARY)
    message(STATUS "espeak-ng found (${ESPEAK_NG_LIBRARY}): spoken countdown enabled")
    target_compile_definitions(minicar PRIVATE MINICAR_HAVE_ESPEAK=1)
    target_link_libraries(minicar PRIVATE ${ESPEAK_NG_LIBRARY})
else()
    message(STATUS "espeak-ng not found: countdown will use beep tones only")
endif()

if(TARGET SDL3_ttf::SDL3_ttf)
    target_link_libraries(minicar PRIVATE SDL3_ttf::SDL3_ttf)
else()
    target_link_libraries(minicar PRIVATE ${SDL3_TTF_LIBRARIES})
endif()

if(TARGET SDL3_image::SDL3_image)
    target_link_libraries(minicar PRIVATE SDL3_image::SDL3_image)
else()
    target_link_libraries(minicar PRIVATE ${SDL3_IMAGE_LIBRARIES})
endif()

# Absolute path to assets/ baked in at build time so the game finds its sprite
# files regardless of the working directory the executable is launched from.
target_compile_definitions(minicar PRIVATE MINICAR_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")

if(MINICAR_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

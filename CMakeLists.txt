cmake_minimum_required(VERSION 3.16)
project(minicar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MINICAR_BUILD_TESTS "Build the miniCar unit test suite" ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Pure gameplay logic: track math, physics, collisions, driver strategies and
# the race domain model. Deliberately excludes anything that touches audio
# devices, fonts or window/renderer creation, so it can be linked into the
# unit test binary with no display/audio device required.
add_library(minicar_core STATIC
    src/Track.cpp
    src/CollisionSystem.cpp
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
    ${SDL2_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2)
    target_link_libraries(minicar_core PUBLIC SDL2::SDL2)
else()
    target_link_libraries(minicar_core PUBLIC ${SDL2_LIBRARIES})
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
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(minicar PRIVATE minicar_core)

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

# minicar_core already links SDL2 PUBLICly, but SDL2main (when present) must be
# linked directly on the final executable to correctly override main.
if(TARGET SDL2::SDL2main)
    target_link_libraries(minicar PRIVATE SDL2::SDL2main)
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(minicar PRIVATE SDL2_ttf::SDL2_ttf)
else()
    target_link_libraries(minicar PRIVATE ${SDL2_TTF_LIBRARIES})
endif()

if(MINICAR_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

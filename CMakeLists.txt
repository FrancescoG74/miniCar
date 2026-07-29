cmake_minimum_required(VERSION 3.16)
project(minicar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(minicar
    src/main.cpp
    src/Track.cpp
    src/Audio.cpp
    src/Setup.cpp
    src/actor/Car.cpp
    src/actor/Actor.cpp
    src/actor/Rock.cpp
    src/actor/Gate.cpp
    src/actor/StartLine.cpp
)

target_include_directories(minicar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2)
    target_link_libraries(minicar PRIVATE SDL2::SDL2)
else()
    target_link_libraries(minicar PRIVATE ${SDL2_LIBRARIES})
endif()

# Some SDL2 CMake configs (e.g. on Windows/vcpkg) also provide SDL2::SDL2main,
# which must be linked before SDL2::SDL2 to correctly override main.
if(TARGET SDL2::SDL2main)
    target_link_libraries(minicar PRIVATE SDL2::SDL2main)
endif()

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(minicar PRIVATE SDL2_ttf::SDL2_ttf)
else()
    target_link_libraries(minicar PRIVATE ${SDL2_TTF_LIBRARIES})
endif()

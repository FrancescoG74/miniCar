find_package(Catch2 3 REQUIRED)

add_executable(minicar_tests
    test_track_math.cpp
    test_track.cpp
    test_actor.cpp
    test_car.cpp
    test_gate.cpp
    test_rock.cpp
    test_start_line.cpp
    test_collision_system.cpp
    test_driver_assignment_service.cpp
    test_race_builder.cpp
    test_race_session.cpp
)

target_link_libraries(minicar_tests PRIVATE minicar_core Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(minicar_tests)

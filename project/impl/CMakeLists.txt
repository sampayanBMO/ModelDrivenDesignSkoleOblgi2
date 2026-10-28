cmake_minimum_required(VERSION 3.20)
project(lab_booking CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

add_library(lab_booking STATIC
    src/admin.cpp
    src/booking.cpp
    src/equipment.cpp
    src/student.cpp
    src/user.cpp
)
target_include_directories(lab_booking PUBLIC include)

if(MSVC)
    target_compile_options(lab_booking PRIVATE /W4)
else()
    target_compile_options(lab_booking PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(demo src/demo.cpp)
target_link_libraries(demo PRIVATE lab_booking)

add_executable(scenario_diagram scenarios/diagram.cpp)
target_link_libraries(scenario_diagram PRIVATE lab_booking)

add_executable(booking_tests tests/booking_tests.cpp)
target_link_libraries(booking_tests PRIVATE lab_booking)
add_test(NAME booking_tests COMMAND booking_tests)

add_executable(equipment_tests tests/equipment_tests.cpp)
target_link_libraries(equipment_tests PRIVATE lab_booking)
add_test(NAME equipment_tests COMMAND equipment_tests)

add_executable(workflow_tests tests/workflow_tests.cpp)
target_link_libraries(workflow_tests PRIVATE lab_booking)
add_test(NAME workflow_tests COMMAND workflow_tests)

add_executable(verification_probe ../verification/probe.cpp)
target_link_libraries(verification_probe PRIVATE lab_booking)

add_executable(benchmark_runner ../verification/benchmark.cpp)
target_link_libraries(benchmark_runner PRIVATE lab_booking)

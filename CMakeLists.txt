cmake_minimum_required(VERSION 3.16)
project(MAIN VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_TESTS ON)
add_subdirectory(legrad_metal)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} legrad_metal_shared)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(TEST_NAME ${LIB_NAME}_test)

# Include Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(${TEST_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/view_pack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/array_view.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
    ${SHARED_LIB_NAME}
    Catch2::Catch2WithMain
)

# Enable testing
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${TEST_NAME})
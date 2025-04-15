set(LIB_NAME legrad_metal)
set(SHARED_LIB_NAME ${LIB_NAME}_shared)

file(GLOB_RECURSE SRC_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/internal/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/metal_mgr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/metal_function.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/metal_dispatcher.cpp"
)

file(GLOB_RECURSE INCLUDE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/macros/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/internal/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/metal_mgr.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/metal_function.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/backend/metal_dispatcher.h"
)

set(INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/"
)

# Add OpenCV
find_package(OpenCV REQUIRED)
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found")
endif()
message(STATUS "Use OpenCV")
set(EXTRA_LIBS ${EXTRA_LIBS} ${OpenCV_LIBS})

# Add fmt
include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)
message(STATUS "Use fmt")
set(EXTRA_LIBS ${EXTRA_LIBS} fmt::fmt)

# Use threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Use Boost
find_package(Boost CONFIG COMPONENTS system log log_setup REQUIRED)

# Add Sanitizer
add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined)
link_libraries(-fsanitize=address -fsanitize=undefined)

# Copy all shader file to output
# file(GLOB SHADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/kernels/*")
# foreach(SHADER_FILE ${SHADER_FILES})
#     get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
#     configure_file(${SHADER_FILE} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHADER_NAME}" COPYONLY)
# endforeach()

# Add Metal
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/backend/metal")
set(EXTRA_LIBS ${EXTRA_LIBS} METAL_CPP)

# Optimization for Apple M
add_compile_options(-mcpu=native)

# Compiler Warnings
set(cxx_flags # Common and useful compiler warning flags.
    -Wall # Enable all common warnings.
    -Wextra # Enable extra warnings.
    -Wpedantic # Enable pedantic warnings (stricter standard conformance).
    -Wcast-qual # Warn about casting away qualifiers (const, volatile).
    -Wno-unused-function # Suppress warnings about unused functions (can be noisy in header-only libraries or during development).
) 
add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${cxx_flags}>")

# Add Profiller
add_compile_options(-pg)

# Build library
message(STATUS "Build shared library")
add_library(${SHARED_LIB_NAME} SHARED ${SRC_FILES})
target_include_directories(${SHARED_LIB_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<INSTALL_INTERFACE:${INCLUDE_DIR}>
    ${EXTRA_INCLUDES}
)
target_link_libraries(${SHARED_LIB_NAME} PUBLIC 
                    Boost::system
                    Boost::log
                    Boost::log_setup
                    Threads::Threads
                    ${EXTRA_LIBS})
target_compile_definitions(${SHARED_LIB_NAME} PUBLIC KERNEL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/backend/kernels/")
target_compile_features(${SHARED_LIB_NAME} PRIVATE cxx_std_17)

if (BUILD_TESTS)
    message(STATUS "Build tests")
    add_subdirectory(tests)
endif()
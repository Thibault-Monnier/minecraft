cmake_minimum_required(VERSION 3.30)
project(minecraft)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
            "This project requires Clang (found: ${CMAKE_CXX_COMPILER_ID})\n"
            "Try adding:\n -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"
    )
endif ()


set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

# Default to Debug build if not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    message(STATUS "No build type specified, defaulting to Debug")
endif ()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wshadow
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        CMAKE_ROOT_DIR="${CMAKE_SOURCE_DIR}"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -fsanitize=address,undefined
            -g
            -fno-omit-frame-pointer
            -O0
    )
    target_link_options(${PROJECT_NAME} PRIVATE
            -fsanitize=address,undefined
    )

else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)

endif ()

# === raylib setup ===
FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG master
)
FetchContent_MakeAvailable(raylib)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${raylib_SOURCE_DIR}/src
        resources/shaders
)

# === abseil setup ===
FetchContent_Declare(
        abseil-cpp
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG 20250512.1
)
FetchContent_MakeAvailable(abseil-cpp)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${abseil-cpp_SOURCE_DIR}/absl
)

# === stb_perlin setup ===
add_library(stb_perlin STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/stb_perlin_wrapper.cpp
)
target_compile_definitions(stb_perlin PRIVATE STB_PERLIN_IMPLEMENTATION)
target_include_directories(stb_perlin PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE raylib
        PRIVATE absl::base
        PRIVATE absl::flat_hash_map
        PRIVATE absl::hash
        PRIVATE stb_perlin
        PRIVATE m pthread dl
)
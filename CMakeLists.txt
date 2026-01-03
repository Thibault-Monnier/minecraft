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
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif ()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

add_executable(minecraft ${SRC_FILES})

target_include_directories(minecraft PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(minecraft SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders
)

target_compile_definitions(minecraft PRIVATE CMAKE_ROOT_DIR="${CMAKE_SOURCE_DIR}")

target_compile_options(minecraft PRIVATE -Wall -Wextra -Wshadow)
target_link_options(minecraft PRIVATE -fuse-ld=lld)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SAN_FLAGS "-fsanitize=address,undefined,leak,signed-integer-overflow -fno-omit-frame-pointer -O0 -g")

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${SAN_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined,leak")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address,undefined,leak")

else ()
    target_compile_options(minecraft PRIVATE -O3 -flto -ffast-math)
    target_link_options(minecraft PRIVATE -flto)

endif ()

# === raylib setup ===
add_subdirectory(third_party/raylib)
get_target_property(RAYLIB_INCLUDES raylib INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(raylib SYSTEM INTERFACE ${RAYLIB_INCLUDES})
target_compile_options(raylib PRIVATE
        $<$<C_COMPILER_ID:Clang>:-w>
        $<$<C_COMPILER_ID:GNU>:-w>
        $<$<C_COMPILER_ID:MSVC>:/w>
)

# === abseil setup ===
add_subdirectory(third_party/abseil-cpp)

target_link_libraries(minecraft
        PRIVATE raylib
        PRIVATE absl::base
        PRIVATE absl::flat_hash_map
        PRIVATE absl::hash
        PRIVATE m pthread dl
)